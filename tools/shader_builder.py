#!/usr/bin/env python3
"""AGRB shader command generator.

Parses env + manifest YAML files and generates (per namespace):
- <build>/<namespace>/*.spv.cmd command files
- <build>/<namespace>/shaders.h
- <build>/<namespace>/agrb_shaders.d
"""

from __future__ import annotations

import argparse
import re
import shlex
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any

INCLUDE_RE = re.compile(r"^\s*#\s*include\s*[<\"]([^\">]+)[\">]", re.MULTILINE)
STAGE_CODE = {"vs": 0x1, "fs": 0x2, "cs": 0x3, "gs": 0x4, "tcs": 0x5, "tes": 0x6}
DEFAULT_DEPFILE = "agrb_shaders.d"
DEFAULT_AGGREGATE_DEPFILE = "agrb_shaders_all.d"
SHADER_PREFIX = "AS_"


class ShaderBuildError(RuntimeError):
    pass


@dataclass
class EnvVariant:
    name: str
    bit: int
    flags: list[str]


@dataclass
class StageVariant:
    label: str
    variant_names: list[str]


@dataclass
class StageDesc:
    stage: str
    src: str
    variants: list[StageVariant]
    compiler_flags: list[str]


@dataclass
class ShaderDesc:
    name: str
    shader_id: int
    stages: list[StageDesc]
    base_dir: Path


@dataclass
class CompilerConfig:
    path: str
    flags: list[str]
    profiles: dict[str, list[str]]


@dataclass
class Job:
    output_name: str
    output_path: Path
    cmd_path: Path
    source_path: Path
    include_deps: list[Path]
    includes: list[Path]
    compiler_flags: list[str]


def sanitize_token(value: str) -> str:
    out = re.sub(r"[^0-9A-Za-z]+", "_", value.upper()).strip("_")
    return out or "UNNAMED"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Generate AGRB shader command files and metadata")
    p.add_argument("--env", required=True, help="Path to env YAML config")
    p.add_argument("-i", "--manifest", nargs="+", required=True, help="Path(s) to shader manifest YAML")
    p.add_argument("-b", "--build-dir", required=True, help="Build root directory")
    p.add_argument(
        "--namespace",
        default="",
        help="Namespace to generate (e.g. scene,postprocess). Empty means all namespaces from manifest",
    )
    p.add_argument("--profile", default="", help="Compiler profile from env.compiler.profiles")
    p.add_argument("--compiler", default="", help="Compiler executable override (has priority over env.compiler.path)")
    p.add_argument(
        "--compiler-flags",
        default="",
        help="Extra compiler flags added for all commands, e.g. \"-O --target-env=vulkan1.2\"",
    )
    p.add_argument(
        "--aggregate-target",
        default="",
        help="Optional target path for global depfile (<build-dir>/agrb_shaders_all.d)",
    )
    p.add_argument("--verbose", action="store_true", help="Print generated jobs")
    return p.parse_args()


def load_yaml(path: Path) -> Any:
    try:
        import yaml
    except ImportError as exc:
        raise SystemExit("PyYAML is required. Install it with: pip install pyyaml") from exc

    with path.open("r", encoding="utf-8") as f:
        data = yaml.safe_load(f)
    return {} if data is None else data


def ensure_str_list(value: Any) -> list[str]:
    if value is None:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, list):
        out: list[str] = []
        for item in value:
            if not isinstance(item, str):
                raise ShaderBuildError(f"Expected string list item, got: {item!r}")
            out.append(item)
        return out
    raise ShaderBuildError(f"Expected list/string, got: {value!r}")


def normalize_compiler_flags(value: Any) -> list[str]:
    if value is None:
        return []
    if isinstance(value, str):
        return shlex.split(value, posix=False)
    if isinstance(value, list):
        out: list[str] = []
        for item in value:
            if not isinstance(item, str):
                raise ShaderBuildError(f"Expected string compiler flag, got: {item!r}")
            out.extend(shlex.split(item, posix=False))
        return out
    raise ShaderBuildError(f"Expected list/string for compiler flags, got: {value!r}")


def dedupe_preserve(seq: list[str]) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for item in seq:
        if item in seen:
            continue
        seen.add(item)
        out.append(item)
    return out


def expand_manifest_inputs(inputs: list[str]) -> list[Path]:
    paths: list[Path] = []
    for raw in inputs:
        p = Path(raw).resolve()
        if p.is_dir():
            yamls = sorted(p.glob("*.yaml")) + sorted(p.glob("*.yml"))
            paths.extend(x.resolve() for x in yamls if x.is_file())
        else:
            paths.append(p)

    uniq: list[Path] = []
    seen: set[Path] = set()
    for p in paths:
        if p in seen:
            continue
        seen.add(p)
        uniq.append(p)
    if not uniq:
        raise ShaderBuildError("No manifest files found")
    return uniq


def parse_variants(raw_variants: Any) -> dict[str, list[str]]:
    if raw_variants is None:
        return {}

    result: dict[str, list[str]] = {}

    def add_variant(name: str, value: Any) -> None:
        if not isinstance(name, str) or not name:
            raise ShaderBuildError(f"Variant key must be non-empty string, got: {name!r}")
        result[name] = normalize_compiler_flags(value)

    if isinstance(raw_variants, dict):
        for key, value in raw_variants.items():
            add_variant(str(key), value)
        return result

    if not isinstance(raw_variants, list):
        raise ShaderBuildError("env.variants must be map or list")

    for item in raw_variants:
        if not isinstance(item, dict) or len(item) != 1:
            raise ShaderBuildError(f"Invalid env.variants item: {item!r}")
        key, value = next(iter(item.items()))
        add_variant(str(key), value)

    return result


def parse_compiler(raw_compiler: Any) -> CompilerConfig:
    if raw_compiler is None:
        return CompilerConfig(path="glslc", flags=[], profiles={})
    if not isinstance(raw_compiler, dict):
        raise ShaderBuildError("env.compiler must be map")

    path = raw_compiler.get("path", "glslc")
    if not isinstance(path, str) or not path.strip():
        raise ShaderBuildError("env.compiler.path must be non-empty string")

    flags = normalize_compiler_flags(raw_compiler.get("flags"))
    profiles_raw = raw_compiler.get("profiles", {})
    if not isinstance(profiles_raw, dict):
        raise ShaderBuildError("env.compiler.profiles must be map")

    profiles: dict[str, list[str]] = {}
    for name, cfg in profiles_raw.items():
        if not isinstance(name, str) or not name:
            raise ShaderBuildError(f"Invalid profile name: {name!r}")
        if cfg is None:
            profiles[name.lower()] = []
            continue
        if isinstance(cfg, dict):
            profiles[name.lower()] = normalize_compiler_flags(cfg.get("flags"))
            continue
        profiles[name.lower()] = normalize_compiler_flags(cfg)

    return CompilerConfig(path=path, flags=flags, profiles=profiles)


def parse_env_config(env_path: Path) -> tuple[list[Path], Path, CompilerConfig, list[EnvVariant], set[Path]]:
    data = load_yaml(env_path)
    deps = {env_path.resolve()}

    if not isinstance(data, dict):
        raise ShaderBuildError("Env config must be top-level map")
    if "env" in data and isinstance(data["env"], dict):
        data = data["env"]

    env_dir = env_path.parent.resolve()
    cwd = Path.cwd().resolve()

    def resolve_config_path(raw: str) -> Path:
        p = Path(raw)
        if p.is_absolute():
            return p.resolve()
        cwd_candidate = (cwd / p).resolve()
        if cwd_candidate.exists():
            return cwd_candidate
        env_candidate = (env_dir / p).resolve()
        if env_candidate.exists():
            return env_candidate
        return cwd_candidate

    includes = [resolve_config_path(x) for x in ensure_str_list(data.get("includes", []))]
    source_dir_raw = data.get("source_dir", ".")
    if not isinstance(source_dir_raw, str) or not source_dir_raw:
        raise ShaderBuildError("env.source_dir must be non-empty string")
    source_dir = resolve_config_path(source_dir_raw)

    compiler = parse_compiler(data.get("compiler"))
    env_variants_raw = parse_variants(data.get("variants"))
    env_variants: list[EnvVariant] = []
    for idx, (name, flags) in enumerate(env_variants_raw.items()):
        env_variants.append(EnvVariant(name=name, bit=1 << idx, flags=flags))

    return includes, source_dir, compiler, env_variants, deps


def normalize_stage_variants(raw: Any) -> list[StageVariant]:
    if raw is None:
        return []
    if isinstance(raw, dict):
        out: list[StageVariant] = []
        for label, variant_names in raw.items():
            out.append(StageVariant(label=str(label), variant_names=ensure_str_list(variant_names)))
        return out
    if isinstance(raw, list):
        out: list[StageVariant] = []
        for item in raw:
            if not isinstance(item, dict) or len(item) != 1:
                raise ShaderBuildError(f"Invalid stage variants item: {item!r}")
            label, variant_names = next(iter(item.items()))
            out.append(StageVariant(label=str(label), variant_names=ensure_str_list(variant_names)))
        return out
    raise ShaderBuildError("stage.variants must be map or list")


def normalize_stages(raw_stages: Any) -> list[StageDesc]:
    if not isinstance(raw_stages, list):
        raise ShaderBuildError("Shader 'stages' must be list")
    stages: list[StageDesc] = []
    for item in raw_stages:
        if not isinstance(item, dict) or len(item) != 1:
            raise ShaderBuildError(f"Invalid stage entry: {item!r}")
        stage_name, cfg = next(iter(item.items()))
        stage_name = str(stage_name)
        if stage_name not in STAGE_CODE:
            raise ShaderBuildError(f"Unsupported stage '{stage_name}'")
        if isinstance(cfg, str):
            # Short form: - vs: "path/to/file.vert"
            cfg = {"src": cfg}
        elif cfg is None:
            cfg = {}
        elif not isinstance(cfg, dict):
            raise ShaderBuildError(f"Stage '{stage_name}' config must be map or string")

        src = cfg.get("src")
        if not isinstance(src, str) or not src:
            raise ShaderBuildError(f"Stage '{stage_name}' missing non-empty 'src'")

        stages.append(
            StageDesc(
                stage=stage_name,
                src=src,
                variants=normalize_stage_variants(cfg.get("variants")),
                compiler_flags=normalize_compiler_flags(cfg.get("compiler_flags")),
            )
        )
    return stages


def parse_shader_manifests(paths: list[Path], namespace: str) -> tuple[list[ShaderDesc], set[Path]]:
    shaders: list[ShaderDesc] = []
    deps: set[Path] = set()
    found_namespace = False

    for manifest_path in paths:
        data = load_yaml(manifest_path)
        deps.add(manifest_path.resolve())
        if not isinstance(data, dict):
            raise ShaderBuildError(f"Manifest {manifest_path} must be top-level map")

        ns_block = data.get(namespace)
        if ns_block is None:
            continue
        found_namespace = True
        if not isinstance(ns_block, dict):
            raise ShaderBuildError(f"Namespace '{namespace}' in {manifest_path} must be map")

        for shader_name, cfg in ns_block.items():
            if not isinstance(cfg, dict):
                raise ShaderBuildError(f"Shader '{shader_name}' config must be map")
            if "id" not in cfg:
                raise ShaderBuildError(f"Shader '{shader_name}' missing 'id'")
            shader_id = int(cfg["id"], 0) if isinstance(cfg["id"], str) else int(cfg["id"])
            if shader_id < 0 or shader_id > 0xFFFFFFFF:
                raise ShaderBuildError(f"Shader '{shader_name}' id must be u32")

            stages = normalize_stages(cfg.get("stages", []))
            if not stages:
                raise ShaderBuildError(f"Shader '{shader_name}' has no stages")

            shaders.append(
                ShaderDesc(
                    name=str(shader_name),
                    shader_id=shader_id,
                    stages=stages,
                    base_dir=manifest_path.parent.resolve(),
                )
            )

    if not found_namespace:
        raise ShaderBuildError(f"Namespace '{namespace}' not found in provided manifest(s)")
    return shaders, deps


def collect_manifest_namespaces(paths: list[Path]) -> list[str]:
    found: list[str] = []
    seen: set[str] = set()
    for manifest_path in paths:
        data = load_yaml(manifest_path)
        if not isinstance(data, dict):
            raise ShaderBuildError(f"Manifest {manifest_path} must be top-level map")
        for key, value in data.items():
            if not isinstance(key, str):
                continue
            if not isinstance(value, dict):
                continue
            if key in seen:
                continue
            seen.add(key)
            found.append(key)
    if not found:
        raise ShaderBuildError("No namespaces found in provided manifest(s)")
    return found


def resolve_include(include_name: str, parent_file: Path, include_dirs: list[Path]) -> Path:
    candidates = [parent_file.parent / include_name]
    candidates.extend(d / include_name for d in include_dirs)
    for p in candidates:
        rp = p.resolve()
        if rp.exists() and rp.is_file():
            return rp
    raise ShaderBuildError(f"Include '{include_name}' not found for source '{parent_file}'")


def collect_include_deps(entry: Path, include_dirs: list[Path]) -> list[Path]:
    collected: list[Path] = []
    seen: set[Path] = set()

    def visit(path: Path, stack: set[Path]) -> None:
        rp = path.resolve()
        if rp in stack:
            raise ShaderBuildError(f"Include cycle detected at: {rp}")
        if rp in seen:
            return
        if not rp.exists() or not rp.is_file():
            raise ShaderBuildError(f"Source file not found: {rp}")

        stack.add(rp)
        seen.add(rp)
        text = rp.read_text(encoding="utf-8")
        for inc in INCLUDE_RE.findall(text):
            visit(resolve_include(inc, rp, include_dirs), stack)
        stack.remove(rp)
        collected.append(rp)

    visit(entry, set())
    return collected


def output_name(shader_id: int, stage: str, mask: int) -> str:
    stage_code = STAGE_CODE[stage]
    packed_id = (shader_id << 32) | (stage_code << 24) | mask
    return f"{packed_id:016X}.spv"


def compiler_flags_from_variants(
    names: list[str], variant_by_name: dict[str, EnvVariant], variant_bits: dict[str, int]
) -> tuple[int, list[str]]:
    mask = 0
    flags: list[str] = []
    for name in names:
        if name not in variant_by_name:
            raise ShaderBuildError(f"Unknown variant '{name}' in manifest")
        mask |= variant_bits[name]
        flags.extend(variant_by_name[name].flags)
    return mask, dedupe_preserve(flags)


def build_jobs(
    shaders: list[ShaderDesc],
    source_dir: Path | None,
    include_dirs: list[Path],
    variant_by_name: dict[str, EnvVariant],
    variant_bits: dict[str, int],
    ns_build_dir: Path,
    compiler_flags_global: list[str],
) -> list[Job]:
    jobs: list[Job] = []
    seen_outputs: dict[str, tuple[Path, tuple[str, ...]]] = {}

    for shader in shaders:
        for stage in shader.stages:
            stage_variants = stage.variants if stage.variants else [StageVariant(label="default", variant_names=[])]
            for variant in stage_variants:
                mask, variant_flags = compiler_flags_from_variants(
                    variant.variant_names, variant_by_name, variant_bits
                )
                flags = dedupe_preserve(compiler_flags_global + stage.compiler_flags + variant_flags)

                src = Path(stage.src)
                if src.is_absolute():
                    src = src.resolve()
                else:
                    base = source_dir if source_dir is not None else shader.base_dir
                    src = (base / src).resolve()

                deps = collect_include_deps(src, include_dirs)
                out_name = output_name(shader.shader_id, stage.stage, mask)
                out_path = (ns_build_dir / out_name).resolve()
                cmd_path = (ns_build_dir / f"{out_name}.cmd").resolve()

                signature = (src, tuple(flags))
                prev = seen_outputs.get(out_name)
                if prev is not None and prev != signature:
                    raise ShaderBuildError(
                        f"Output collision for '{out_name}': variants map to same output with different flags/source"
                    )
                seen_outputs[out_name] = signature

                jobs.append(
                    Job(
                        output_name=out_name,
                        output_path=out_path,
                        cmd_path=cmd_path,
                        source_path=src,
                        include_deps=deps,
                        includes=include_dirs,
                        compiler_flags=flags,
                    )
                )

    return jobs


def shell_quote(arg: str) -> str:
    return '"' + arg.replace('"', '\\"') + '"'


def build_cmd_line(compiler: str, source_path: Path, output_path: Path, includes: list[Path], flags: list[str]) -> str:
    cmd = [shell_quote(compiler), shell_quote(str(source_path)), "-o", shell_quote(str(output_path))]
    for flag in flags:
        cmd.append(shell_quote(flag))
    for inc in includes:
        cmd.append(shell_quote(f"-I{inc}"))
    return " ".join(cmd)


def write_job_files(jobs: list[Job], compiler: str, ns_build_dir: Path) -> None:
    ns_build_dir.mkdir(parents=True, exist_ok=True)
    for job in jobs:
        line = build_cmd_line(compiler, job.source_path, job.output_path, job.includes, job.compiler_flags)
        job.cmd_path.write_text(line + "\n", encoding="utf-8")


def dep_escape(path: Path) -> str:
    return str(path).replace("\\", "/").replace(" ", "\\ ")


def generate_header(path: Path, namespace: str, variant_bits: dict[str, int], shaders: list[ShaderDesc]) -> None:
    ns_tok = sanitize_token(namespace)
    macro_prefix = f"{SHADER_PREFIX}{ns_tok}_"
    lines: list[str] = [
        "#pragma once",
        "",
        "// Generated by agrb/tools/shader_builder.py",
        "",
    ]

    bit_by_name = variant_bits
    for shader in shaders:
        shader_tok = sanitize_token(shader.name)
        for stage in shader.stages:
            stage_tok = sanitize_token(stage.stage)
            stage_value = STAGE_CODE[stage.stage] << 24
            id_value = shader.shader_id << 32

            if stage.variants:
                for variant in stage.variants:
                    macro = f"{macro_prefix}{shader_tok}_{stage_tok}_{sanitize_token(variant.label)}"
                    mask = 0
                    for name in variant.variant_names:
                        if name not in bit_by_name:
                            raise ShaderBuildError(f"Unknown variant '{name}' while generating header")
                        mask |= bit_by_name[name]
                    value = id_value | stage_value | mask
                    lines.append(f"#define {macro} 0x{value:016X}ULL")
            else:
                macro = f"{macro_prefix}{shader_tok}_{stage_tok}"
                value = id_value | stage_value
                lines.append(f"#define {macro} 0x{value:016X}ULL")

    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def write_depfile(depfile_path: Path, jobs: list[Job], yaml_deps: set[Path], header_path: Path) -> None:
    depfile_path.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []
    all_source_deps = set(yaml_deps)

    for job in jobs:
        srcs = set(job.include_deps)
        all_source_deps.update(srcs)
        deps_joined = " ".join(dep_escape(p) for p in sorted(srcs | yaml_deps))
        lines.append(f"{dep_escape(job.output_path)} {dep_escape(job.cmd_path)}: {deps_joined}")

    aggregate_targets = [depfile_path, header_path]
    aggregate_deps = " ".join(dep_escape(p) for p in sorted(all_source_deps))
    lines.append(f"{' '.join(dep_escape(t) for t in aggregate_targets)}: {aggregate_deps}")
    depfile_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_aggregate_depfile(depfile_path: Path, target_path: Path, deps: set[Path]) -> None:
    depfile_path.parent.mkdir(parents=True, exist_ok=True)
    depfile_path.write_text(
        f"{dep_escape(target_path)}: {' '.join(dep_escape(p) for p in sorted(deps))}\n",
        encoding="utf-8",
    )


def main() -> int:
    args = parse_args()

    env_path = Path(args.env).resolve()
    includes, source_dir, compiler_cfg, env_variants, env_deps = parse_env_config(env_path)

    manifest_paths = expand_manifest_inputs(args.manifest)
    if args.namespace.strip():
        namespaces = [x.strip() for x in args.namespace.split(",") if x.strip()]
    else:
        namespaces = collect_manifest_namespaces(manifest_paths)

    if not namespaces:
        raise ShaderBuildError("No namespaces selected for generation")

    variant_by_name = {v.name: v for v in env_variants}
    variant_bits = {v.name: v.bit for v in env_variants}

    profile_flags: list[str] = []
    if args.profile:
        profile_name = args.profile.lower()
        if profile_name not in compiler_cfg.profiles:
            raise ShaderBuildError(f"Unknown compiler profile '{args.profile}'")
        profile_flags = compiler_cfg.profiles[profile_name]

    compiler = args.compiler.strip() if args.compiler.strip() else compiler_cfg.path
    cli_flags = normalize_compiler_flags(args.compiler_flags)
    compiler_flags_global = dedupe_preserve(compiler_cfg.flags + profile_flags + cli_flags)

    build_root = Path(args.build_dir).resolve()

    aggregate_deps: set[Path] = set(env_deps)

    for namespace in namespaces:
        shaders, manifest_deps = parse_shader_manifests(manifest_paths, namespace)
        aggregate_deps.update(manifest_deps)
        ns_build_dir = (build_root / namespace).resolve()
        header_path = (ns_build_dir / "shaders.h").resolve()
        depfile_path = (ns_build_dir / DEFAULT_DEPFILE).resolve()

        jobs = build_jobs(
            shaders=shaders,
            source_dir=source_dir,
            include_dirs=includes,
            variant_by_name=variant_by_name,
            variant_bits=variant_bits,
            ns_build_dir=ns_build_dir,
            compiler_flags_global=compiler_flags_global,
        )
        write_job_files(jobs, compiler=compiler, ns_build_dir=ns_build_dir)
        generate_header(header_path, namespace=namespace, variant_bits=variant_bits, shaders=shaders)
        write_depfile(depfile_path, jobs, env_deps | manifest_deps, header_path)
        for job in jobs:
            aggregate_deps.update(job.include_deps)

        if args.verbose:
            print(f"namespace: {namespace}")
            print(f"generated jobs: {len(jobs)}")
            print(f"header: {header_path}")
            print(f"depfile: {depfile_path}")
            print(f"compiler: {compiler}")

    if args.aggregate_target.strip():
        write_aggregate_depfile(
            depfile_path=(build_root / DEFAULT_AGGREGATE_DEPFILE).resolve(),
            target_path=Path(args.aggregate_target).resolve(),
            deps=aggregate_deps,
        )

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ShaderBuildError as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(2)
