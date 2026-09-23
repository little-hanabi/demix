# Demix

> [!IMPORTANT]
> - 非专业工具，使用风险自负。

基于 OpenVINO 的 Windows 命令行音频分离工具。

## 特点

- 批量处理 `.wav` 文件。
- 使用 OpenVINO 推理。
- 支持模型缓存。
- 简单命令行接口。

## 使用

```
demix --infer <cache_dir> <device> <input_dir> <output_dir>
demix --cache <model_dir> <device> <cache_dir>
```

- `--infer`、`-i`：处理 `<input_dir>` 下所有 `.wav` 文件，结果写入 `<output_dir>`。
- `--cache`、`-c`：使用 `<model_dir>` 构建模型缓存到 `<cache_dir>`。
- `<device>`：OpenVINO 设备，如 `CPU`、`GPU`。

## 构建

```bash
cmake -B build -S . -G Ninja -D CMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 许可

见 `LICENSE`