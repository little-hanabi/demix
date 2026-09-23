# Demix

> [!IMPORTANT]
> - Not a professional tool. Use at your own risk.

A Windows command-line audio demixing tool using OpenVINO.

## Features

- Batch process `.wav` files.
- OpenVINO inference.
- Model cache support.
- Simple CLI.

## Usage

```
demix --infer <cache_dir> <device> <input_dir> <output_dir>
demix --cache <model_dir> <device> <cache_dir>
```

- `--infer`, `-i`: process all `.wav` files in `<input_dir>`, write results to `<output_dir>`.
- `--cache`, `-c`: build model cache from `<model_dir>` into `<cache_dir>`.
- `<device>`: OpenVINO device, e.g. `CPU`, `GPU`.

## Build

```bash
cmake -B build -S . -G Ninja -D CMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## License

See `LICENSE`