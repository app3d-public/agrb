# AGRB S2U (SPIR-V to UMBF)

`s2u` packs compiled `.spv` files into one UMBF library (`.umlib`).

It scans an input directory, reads all `.spv` files, and stores each shader as an `agrb::shader_block` inside `umbf::Library`.

## CLI

```bash
agrb_s2u --input <dir_with_spv> --output <file.umlib> [--compression <0..22>]
```

Short flags:
- `-i`: input directory
- `-o`: output file
- `-c`: compression level (default: `5`)

## Input requirements

- Input directory must contain `.spv` files.
- File stem must be a final packed hex shader ID, for example:
  - `E95440BD01000001.spv`
  - `3B976C1D02000000.spv`

## Output format

- Output file type: `umbf::sign_block::format::library`
- Root node: folder
- Each shader is a file node with one `agrb::shader_block`

Shader node headers:
- `vendor_sign = AGRB_VENDOR_ID`
- `vendor_version = AGRB_VERSION`
- `spec_version = UMBF_VERSION`
- `type_sign = AGRB_TYPE_ID_SHADER`

## Typical CMake flow

1. `shader_builder.py` generates `*.spv.cmd`.
2. CMake runs all `*.spv.cmd`.
3. `agrb_s2u` packs namespace directory to `<namespace>.umlib`.
