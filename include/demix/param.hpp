#pragma once

namespace demix {
    constexpr int S = 44100;
    constexpr int C = 2;
    constexpr int T = 948;
    constexpr int M = 87;
    constexpr int N_FFT = 2048;
    constexpr int N_HOP = 512;

    static_assert(T > M);
    static_assert(N_FFT % N_HOP == 0);

    constexpr int N_BIN = (N_FFT >> 1) + 1;
    constexpr int T_CHK = T * N_BIN * C * 2;
    constexpr int T_MIX = M * N_BIN * C * 2;
    constexpr int STEP = T - M;
    constexpr int HALF = N_FFT / 2;
    constexpr int TAIL = N_FFT / N_HOP - 1;
}
