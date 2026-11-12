#!/usr/bin/env python3
"""Strict VST3 metadata JSON validation regression without JUCE."""
import json
import tempfile
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "packaging"))
from normalize_moduleinfo import remove_trailing_commas, normalize

bad = '{"Version":"0.10.2", "Test": [1,2,], "Nested": {"label":"comma, ] inside a string",},}'
strict = json.loads(remove_trailing_commas(bad))
assert strict["Test"] == [1, 2]
assert strict["Nested"]["label"] == "comma, ] inside a string"
with tempfile.TemporaryDirectory() as folder:
    path = Path(folder) / "moduleinfo.json"
    path.write_text(bad, encoding="utf-8")
    normalize(path)
    repaired = json.loads(path.read_text(encoding="utf-8"))
    assert repaired == strict
    original = path.read_bytes()
    normalize(path)
    assert path.read_bytes() == original, "normalization should be idempotent"
    path.write_text('{"Version":"0.10.2","Broken":[1,}', encoding="utf-8")
    try:
        normalize(path)
    except json.JSONDecodeError:
        pass
    else:
        raise AssertionError("Malformed JSON must not be silently accepted")

print("VST3 moduleinfo strict JSON PASS: string safety, arrays, objects, idempotence")
