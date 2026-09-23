#include <limits>
#include <numbers>

#include <demix/audio.hpp>
#include <demix/param.hpp>

#include <AudioFile.hpp>

namespace demix {
    namespace {
        const auto WIN = [] {
            constexpr float a = 2.0f * std::numbers::pi_v<float>;
            std::array<float, N_FFT> w{};
            for (int i = 0; i < N_FFT; ++i) w[i] = 0.5f * (1.0f - std::cos(static_cast<float>(i) * a / N_FFT)); // periodic window
            return w;
        }();

        constexpr float EPS = std::numeric_limits<float>::epsilon();
    }

    void Audio::stft(const std::vector<std::vector<float>>& wav, std::vector<float>& tsr) const {
        float* real = fftwf_alloc_real(C * T_ * N_FFT);

        for (int c = 0; c < C; ++c) {
            for (uint64_t t = 0; t < T_; ++t) {
                for (int i = 0; i < N_FFT; ++i) { // reflective center padding
                    const auto pos = t * N_HOP + i;
                    const auto src = pos < N_FFT / 2 ? (N_FFT / 2 - pos) : (pos - N_FFT / 2);
                    const auto dst = (c * T_ + t) * N_FFT + i;
                    if (src >= L_) real[dst] = src > 2 * (L_ - 1) ? 0.0f : wav[c][2 * (L_ - 1) - src] * WIN[i];
                    else           real[dst] = wav[c][src] * WIN[i];
                }
            }
        }

        const auto plan = fftwf_plan_many_dft_r2c(
            1, &N_FFT, static_cast<int>(C * T_),
            real, nullptr, 1, N_FFT,
            spec.get(), nullptr, 1, N_BIN, FFTW_ESTIMATE
        );

        fftwf_execute(plan);
        fftwf_destroy_plan(plan);
        fftwf_free(real);

        tsr.resize(T_ * N_BIN * C * 2);
        for (uint64_t t = 0; t < T_; ++t) {
            for (int f = 0; f < N_BIN; ++f) {
                for (int c = 0; c < C; ++c) {
                    const auto dst = ((t * N_BIN + f) * C + c) * 2;
                    const auto src = (c * T_ + t) * N_BIN + f;
                    tsr[dst]     = spec[src][0];
                    tsr[dst + 1] = spec[src][1];
                }
            }
        }
    }

    void Audio::mask(const std::span<const float> tsr, std::vector<std::vector<float>>& wav) const {
        fftwf_complex* buf_i = fftwf_alloc_complex(N_BIN);
        float*         buf_o = fftwf_alloc_real(N_FFT);
        const auto plan = fftwf_plan_dft_c2r_1d(N_FFT, buf_i, buf_o, FFTW_ESTIMATE);

        std::array<float, N_FFT> acc{}; // overlap-added windowed sample
        std::array<float, N_FFT> env{}; // sum of squared window energy

        for (int c = 0; c < C; ++c) {
            acc.fill(0.0f);
            env.fill(0.0f);

            for (uint64_t t = 0; t < T_ + TAIL; ++t) {
                if (t < T_) {
                    const auto n = std::min(N_ - 1, t / STEP);         // chunk num in total [0, N_- 1]
                    const auto p = t - n * STEP;                       // frame pos in chunk [0, T - 1]
                    const bool m = n > 0 && p < M;                     // crossfade condition
                    const auto a = static_cast<float>(p) / (M - 1.0f); // crossfade coefficient

                    for (int f = 0; f < N_BIN; ++f) {
                        const auto ori = (c * T_ + t) * N_BIN + f;                  // index of spec
                        const auto mod = n * T_CHK + ((p * N_BIN + f) * C + c) * 2; // index of mask
                        const auto r = m ? (a * tsr[mod    ] + (1.0f - a) * tsr[mod - T_MIX    ]) : tsr[mod    ];
                        const auto i = m ? (a * tsr[mod + 1] + (1.0f - a) * tsr[mod - T_MIX + 1]) : tsr[mod + 1];
                        buf_i[f][0] = spec[ori][0] * r - spec[ori][1] * i;
                        buf_i[f][1] = spec[ori][0] * i + spec[ori][1] * r;
                    }

                    fftwf_execute(plan);

                    for (int i = 0; i < N_FFT; ++i) {
                        acc[i] += WIN[i] * buf_o[i];
                        env[i] += WIN[i] * WIN[i];
                    }
                }

                for (uint64_t k = 0, i = t * N_HOP; k < N_HOP; ++k, ++i) { // output the oldest N_HOP sample
                    if (i < HALF || i >= L_ + HALF) continue;
                    wav[c][i - HALF] = env[k] > EPS ? acc[k] / (env[k] * N_FFT) : 0.0f;
                }

                std::ranges::fill(std::ranges::shift_left(acc, N_HOP).end(), acc.end(), 0.0f);
                std::ranges::fill(std::ranges::shift_left(env, N_HOP).end(), env.end(), 0.0f);
            }
        }

        fftwf_destroy_plan(plan);
        fftwf_free(buf_i);
        fftwf_free(buf_o);
    }

    void Audio::write(const std::span<const float> tsr, const fs::path& pth) const {
        if (!good) return;
        AudioFile<float> file;
        file.setNumChannels(C);
        file.setBitDepth(32);
        file.setSampleRate(S);
        file.setNumSamplesPerChannel(L_);
        mask(tsr, file.samples);
        file.save(pth);
    }

    Audio::Audio(const fs::path& pth, std::vector<float>& tsr) {
        AudioFile<float> file;
        if (!file.load(pth)) return;
        if (file.getSampleRate() != S) return;
        if (file.getNumChannels() != C) return;
        L_ = file.getNumSamplesPerChannel();
        if (L_ == 0) return;
        T_ = L_ / N_HOP + 1;
        N_ = T_ <= T ? 1 : 1 + (T_ - M - 1) / (T - M);
        spec.reset(fftwf_alloc_complex(C * T_ * N_BIN));
        stft(file.samples, tsr);
        good = true;
    }
}
