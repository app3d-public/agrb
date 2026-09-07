# AGRB S2U (SPIR-V to UMBF)

`s2u` packs compiled `.spv` files into one UMBF file (`.umlib`).

It scans an input directory, reads all `.spv` files, and stores each shader directly as an `agrb::shader_block` payload.

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

- Output file type: `umbf::sign_block::format::raw`
- Each table entry has signature `AGRB_SIGN_ID_SHADER`
- Each entry payload contains one serialized `agrb::shader_block`
- The table and shader blob are compressed together when compression is enabled

## Typical CMake flow

1. `shader_builder.py` generates `*.spv.cmd`.
2. CMake runs all `*.spv.cmd`.
3. `agrb_s2u` packs namespace directory to `<namespace>.umlib`.
