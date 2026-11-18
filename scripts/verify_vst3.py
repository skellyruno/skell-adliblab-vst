#!/usr/bin/env python3
"""Fail-closed Windows x64 VST3 artifact verification for Skell AdlibLab."""
import argparse
import hashlib
import json
import pathlib
import stat
import struct
import sys
import zipfile

EXPECTED = "Skell AdlibLab.vst3"
MODULE = "Contents/x86_64-win/Skell AdlibLab.vst3"


def verify_pe(blob: bytes) -> None:
    if len(blob) < 0x100 or blob[:2] != b"MZ":
        raise ValueError("missing MZ header")
    offset = struct.unpack_from("<I", blob, 0x3C)[0]
    if offset < 0x40 or offset + 26 > len(blob) or blob[offset:offset + 4] != b"PE\0\0":
        raise ValueError("invalid PE offset or missing PE signature")
    machine = struct.unpack_from("<H", blob, offset + 4)[0]
    magic = struct.unpack_from("<H", blob, offset + 24)[0]
    if machine != 0x8664 or magic != 0x20B:
        raise ValueError("expected AMD64 PE32+ module")


def _validate_zip_entry(info: zipfile.ZipInfo) -> None:
    name = info.filename
    path = pathlib.PurePosixPath(name)
    if (not name or path.is_absolute() or name.startswith("/")
            or "\\" in name or ":" in name or ".." in path.parts):
        raise ValueError("unsafe ZIP entry: " + name)
    mode = (info.external_attr >> 16) & 0xFFFF
    if stat.S_ISLNK(mode):
        raise ValueError("symlink ZIP entry is not allowed: " + name)
    if info.flag_bits & 0x1:
        raise ValueError("encrypted ZIP entry is not allowed: " + name)


def inspect(path: pathlib.Path | str) -> dict:
    path = pathlib.Path(path)
    if path.suffix.lower() == ".zip":
        with zipfile.ZipFile(path) as archive:
            entries = archive.infolist()
            names = [entry.filename for entry in entries]
            if len(names) != len(set(names)):
                raise ValueError("duplicate ZIP entries")
            if len(names) != len({name.casefold() for name in names}):
                raise ValueError("case-colliding ZIP entries")
            for entry in entries:
                _validate_zip_entry(entry)

            target = EXPECTED + "/" + MODULE
            modules = [
                name for name in names
                if name.lower().endswith(".vst3") and "/x86_64-win/" in name.lower()
            ]
            if modules != [target]:
                raise ValueError("expected exactly one module at " + target)
            if archive.testzip() is not None:
                raise ValueError("ZIP CRC failure")
            data = archive.read(target)
    else:
        if path.is_symlink() or path.name != EXPECTED or not path.is_dir():
            raise ValueError("expected a non-symlink bundle directory named " + EXPECTED)
        for entry in path.rglob("*"):
            if entry.is_symlink():
                raise ValueError("symlink in VST3 bundle: " + str(entry.relative_to(path)))
        module_path = path / MODULE
        modules = [
            entry for entry in path.rglob("*")
            if entry.is_file() and entry.suffix.lower() == ".vst3"
        ]
        if len(modules) != 1 or modules[0].relative_to(path).as_posix() != MODULE:
            raise ValueError("expected exactly one module at " + MODULE)
        data = module_path.read_bytes()

    verify_pe(data)
    return {
        "module_sha256": hashlib.sha256(data).hexdigest(),
        "module_bytes": len(data),
        "artifact": str(path),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("artifact", help="VST3 bundle directory or ZIP containing the bundle")
    args = parser.parse_args()
    try:
        print(json.dumps(inspect(args.artifact), indent=2))
    except (OSError, ValueError, RuntimeError, zipfile.BadZipFile) as exc:
        print("RELEASE GATE FAILED: " + str(exc), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
