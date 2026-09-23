#include <print>

#include <AudioFile.hpp>
#include <demix/demix.hpp>

int wmain(const int argc, const wchar_t* argv[]) {
    std::vector<fs::path> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);

    try {
        if (argc == 6 && (args[0].string() == "--infer" || args[0].string() == "-i")) {
            std::vector<float> tsr_i;
            std::vector<float> tsr_o;
            demix::Model model(args[1], args[2].string());
            if (!model) throw std::invalid_argument("invalid model");
            for (const auto &entry: fs::directory_iterator(args[3])) {
                if (!demix::is_ext(entry, ".wav")) continue;
                demix::Audio audio(entry, tsr_i);
                if (!audio) continue;
                model.infer(tsr_i, audio.get_n(), tsr_o);
                audio.write(tsr_o, args[4] / entry.path().filename());
            }
        }
        else if (argc == 5 && (args[0].string() == "--cache" || args[0].string() == "-c")) {
            demix::Model::cache(args[1], args[3], args[2].string());
        }
        else demix::help();
    } catch (const std::exception& e) {
        std::println(std::cerr, "{}\n", e.what());
        return 1;
    }

    return 0;
}