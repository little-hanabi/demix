#include <fstream>
#include <ranges>

#include <demix/model.hpp>
#include <demix/param.hpp>

namespace demix {
    Model::Model(const fs::path &dir, const std::string &device) {
        if (!fs::is_directory(dir)) return;
        core.set_property(device, ov::hint::execution_mode(ov::hint::ExecutionMode::PERFORMANCE));
        for (int i = 0; i < 4; ++i) {
            auto f = std::ifstream(dir / std::format("{}.blob", i), std::ios::binary);
            auto m = core.import_model(f, device);
            sess[i] = m.create_infer_request();
        }
        good = true;
    }

    void Model::cache(const fs::path& dir_i, const fs::path& dir_o, const std::string& device) {
        if (!fs::is_directory(dir_i) || !fs::is_directory(dir_o)) return;
        ov::Core core;
        core.set_property(device, ov::hint::execution_mode(ov::hint::ExecutionMode::PERFORMANCE));
        for (int i = 0; i < 4; ++i) {
            auto f = std::ofstream(dir_o / std::format("{}.blob", i), std::ios::binary);
            const auto prop = i ? ov::AnyMap{} : ov::AnyMap{ov::hint::inference_precision(ov::element::f32)};
            core.compile_model(dir_i / std::format("{}.xml", i), device, prop).export_model(f);
        }
    }

    void Model::infer(const std::span<const float> src, const uint64_t num, std::vector<float>& dst) {
        if (!good || src.empty() || num == 0) return;

        dst.resize(num * T_CHK);
        for (size_t n = 0, off = 0; n < num; ++n, off += T_CHK - T_MIX) {
            ov::Tensor buf;
            if (n == 0 || n == num - 1) {
                buf = ov::Tensor(ov::element::f32, ov::Shape{1, T, N_BIN * C * 2});
                std::fill_n(buf.data<float>(), T_CHK, 0.0f);
                std::ranges::copy(src | std::views::drop(off) | std::views::take(T_CHK), buf.data<float>());
            } else buf = ov::Tensor(ov::element::f32, ov::Shape{1, T, N_BIN * C * 2}, const_cast<float*>(src.data() + off));

            sess[0].set_input_tensor(buf);
            sess[0].infer();

            sess[1].set_input_tensor(sess[0].get_output_tensor());
            sess[1].infer();

            sess[2].set_input_tensor(sess[1].get_output_tensor());
            sess[2].infer();

            sess[3].set_input_tensor(0, sess[1].get_output_tensor());
            sess[3].set_input_tensor(1, sess[2].get_output_tensor());
            sess[3].set_output_tensor(ov::Tensor(ov::element::f32, ov::Shape{1, T, N_BIN * C * 2}, dst.data() + n * T_CHK));
            sess[3].infer();
        }
    }
}
