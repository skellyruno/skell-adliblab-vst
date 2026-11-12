#!/usr/bin/env python3
"""Normalize JUCE-generated VST3 moduleinfo.json without touching plugin DSP.

Some JUCE/VST3 generators leave trailing commas before closing braces/brackets.
A strict JSON parser must accept the final staged resource.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path


def remove_trailing_commas(source: str) -> str:
    output: list[str] = []
    in_string = False
    escaped = False
    for position, char in enumerate(source):
        if in_string:
            output.append(char)
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            continue
        if char == '"':
            in_string = True
            output.append(char)
        elif char == ",":
            following = position + 1
            while following < len(source) and source[following].isspace():
                following += 1
            if following < len(source) and source[following] in "]}":
                continue
            output.append(char)
        else:
            output.append(char)
    return "".join(output)


def normalize(path: Path) -> None:
    source = path.read_text(encoding="utf-8-sig")
    data = json.loads(remove_trailing_commas(source))
    if not isinstance(data, dict) or not isinstance(data.get("Version"), str):
        raise ValueError("VST3 moduleinfo must contain a Version string")
    target = json.dumps(data, ensure_ascii=False, indent=2) + "\n"
    json.loads(target)  # Validate strict JSON before writing anything.
    path.write_text(target, encoding="utf-8")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: normalize_moduleinfo.py <moduleinfo.json>")
    normalize(Path(sys.argv[1]))
    print("VST3 moduleinfo strict JSON PASS:", sys.argv[1])
