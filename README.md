# SimpleScaler

A lightweight OpenCV-based image scaling tool supporting **relative** and **absolute**
scaling modes, with a clean library/CLI separation.

## Features

- Relative scaling (`--relative`)
- Absolute scaling (`--absolute` or `WxH`)
- `--dry-run` mode
- Self-test image generation
- Library-first design (DLL-ready)

## Usage

```bash
SimpleScaler --relative input.png output.png 2.0
SimpleScaler --absolute input.png output.png 300x200
SimpleScaler --relative input.png output.png 2 --dry-run
SimpleScaler --self-test
```

## Build

Requires:
* C++ 17
* OpenCV

## Third-Party Dependencies

This project uses OpenCV, which is licensed under the Apache License 2.0.
