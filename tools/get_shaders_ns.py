#!/usr/bin/env python3
"""Print top-level shader namespaces from manifest YAML.

Output format:
  one namespace per line
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Get top-level shader namespaces from YAML manifest")
    p.add_argument("-i", "--manifest", required=True, help="Path to shaders.yaml")
    return p.parse_args()


def main() -> int:
    args = parse_args()
    manifest_path = Path(args.manifest).resolve()
    if not manifest_path.exists():
        print(f"Manifest not found: {manifest_path}", file=sys.stderr)
        return 1

    try:
        import yaml
    except ImportError:
        print("PyYAML is required. Install with: pip install pyyaml", file=sys.stderr)
        return 1

    with manifest_path.open("r", encoding="utf-8") as f:
        data = yaml.safe_load(f) or {}

    if not isinstance(data, dict):
        print("Manifest root must be a YAML mapping", file=sys.stderr)
        return 1

    for key in data.keys():
        if isinstance(key, str) and key:
            print(key)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

