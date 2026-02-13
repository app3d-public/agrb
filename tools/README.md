# AGRB Shader Command Generator

`shader_builder.py` parses shader YAML files and generates build artifacts for CMake.

## Requirements
- Python 3.10+
- `PyYAML` (`pip install pyyaml`)

## CLI

```bash
python modules/agrb/tools/shader_builder.py \
  --env src/shaders/env.yaml \
  -i src/shaders/shaders.yaml \
  -b build/src/shaders \
  --profile debug
```

Arguments:
- `--env`: global environment YAML
- `-i/--manifest`: one or multiple manifest files/directories
- `-b/--build-dir`: output build root
- `--namespace`: optional namespace filter (`scene`, `postprocess`, `common`, `compute`); supports comma-separated list
- `--profile`: compiler profile from `env.compiler.profiles`
- `--compiler`: compiler path override (higher priority than `env.compiler.path`)
- `--compiler-flags`: extra global compiler flags
- `--aggregate-target`: optional target used to emit global depfile `agrb_shaders_all.d`

If `--namespace` is omitted, all namespaces from manifest are generated.

## Generated files

For each namespace:
- `<build>/<namespace>/*.spv.cmd`: compiler command files
- `<build>/<namespace>/shaders.h`: C/C++ shader IDs
- `<build>/<namespace>/agrb_shaders.d`: depfile for namespace outputs

Optional global depfile:
- `<build>/agrb_shaders_all.d` when `--aggregate-target` is set

## Output ID format

SPIR-V names and header constants use packed u64:
- `(shader_id << 32) | (stage << 24) | variant_mask`
- stage codes: `vs=1`, `fs=2`, `cs=3`, `gs=4`, `tcs=5`, `tes=6`

Example:
- `E95440BD01000001.spv`

## YAML format

Manifest layout:
- top-level keys are namespaces
- namespace contains shaders
- each shader has `id` and `stages`

Supported stage forms:

```yaml
stages:
  - vs: "scene/grid.vert"
  - fs: "scene/grid.frag"
```

or full form:

```yaml
stages:
  - vs:
      src: "scene/solidcolor.vert"
      variants:
        - idx: [indexed]
        - dir: [direct]
      compiler_flags: ["-DLOCAL_OPTION"]
```

Environment layout:
- `includes`
- `source_dir`
- `variants`: label -> compiler flags
- `compiler.path`
- `compiler.flags`
- `compiler.profiles.<name>.flags`
