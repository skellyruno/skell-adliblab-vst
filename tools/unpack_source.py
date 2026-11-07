#!/usr/bin/env python3
"""Unpack the one-time GitHub upload bundle when the normal source tree is absent.

The packaged source is committed as normal files by the CI publish job. The
bundle exists only because the initial GitHub connector has text-only writes.
"""
from __future__ import annotations
from pathlib import Path
import shutil
import tarfile

root = Path(__file__).resolve().parents[1]
if (root / "CMakeLists.txt").exists() and "project(SkellAdlibLab " in (root / "CMakeLists.txt").read_text():
    print("Standard source tree already present; no extraction needed")
    raise SystemExit(0)
bundle = root / "_bootstrap" / "adliblab-src.tar.xz"
if not bundle.exists():
    raise SystemExit("AdlibLab source bundle is missing")
with tarfile.open(bundle, mode="r:xz") as archive:
    for member in archive.getmembers():
        member_path = Path(member.name)
        if member_path.is_absolute() or ".." in member_path.parts or member.issym() or member.islnk():
            raise SystemExit(f"Unsafe source bundle entry: {member.name}")
        dest = root / member_path
        if member.isdir():
            dest.mkdir(parents=True, exist_ok=True)
        elif member.isfile():
            dest.parent.mkdir(parents=True, exist_ok=True)
            with archive.extractfile(member) as stream, dest.open("wb") as output:
                shutil.copyfileobj(stream, output)
print("Expanded initial AdlibLab source tree")
