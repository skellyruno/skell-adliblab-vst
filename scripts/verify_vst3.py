#!/usr/bin/env python3
"""Fail-closed Windows x64 VST3 artifact verification for Skell AdlibLab."""
import argparse
import hashlib
import json
import pathlib
import struct
import sys
import zipfile

EXPECTED = 'Skell AdlibLab.vst3'
MODULE = 'Contents/x86_64-win/Skell AdlibLab.vst3'

def verify_pe(blob):
    if len(blob) < 0x100 or blob[:2] != b'MZ':
        raise ValueError('missing MZ header')
    offset = struct.unpack_from('<I', blob, 0x3c)[0]
    if offset + 26 > len(blob) or blob[offset:offset+4] != b'PE\0\0':
        raise ValueError('missing PE signature')
    machine = struct.unpack_from('<H', blob, offset+4)[0]
    magic = struct.unpack_from('<H', blob, offset+24)[0]
    if machine != 0x8664 or magic != 0x20b:
        raise ValueError('expected AMD64 PE32+ module')

def inspect(path):
    path = pathlib.Path(path)
    if path.suffix.lower() == '.zip':
        with zipfile.ZipFile(path) as archive:
            names = [i.filename for i in archive.infolist()]
            if len(names) != len(set(names)) or len(names) != len({n.casefold() for n in names}):
                raise ValueError('duplicate or case-colliding ZIP entries')
            for name in names:
                p = pathlib.PurePosixPath(name)
                if name.startswith('/') or '\\' in name or '..' in p.parts or ':' in name:
                    raise ValueError('unsafe ZIP entry: ' + name)
            target = EXPECTED + '/' + MODULE
            modules = [n for n in names if n.endswith('.vst3') and '/x86_64-win/' in n]
            if modules != [target]:
                raise ValueError('expected exactly one module at ' + target)
            if archive.testzip() is not None:
                raise ValueError('ZIP CRC failure')
            data = archive.read(target)
    else:
        if path.name != EXPECTED or not path.is_dir():
            raise ValueError('expected bundle directory ' + EXPECTED)
        data = (path / MODULE).read_bytes()
        modules = [m for m in path.rglob('*.vst3') if m.is_file()]
        if len(modules) != 1:
            raise ValueError('expected exactly one VST3 module')
    verify_pe(data)
    return {'module_sha256': hashlib.sha256(data).hexdigest(), 'module_bytes': len(data), 'artifact': str(path)}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('artifact', help='VST3 bundle directory or ZIP containing the bundle')
    args = parser.parse_args()
    try:
        print(json.dumps(inspect(args.artifact), indent=2))
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        print('RELEASE GATE FAILED: ' + str(exc), file=sys.stderr)
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())
