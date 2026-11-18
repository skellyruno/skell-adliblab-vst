#!/usr/bin/env python3
"""Adversarial regression tests for the Windows VST3 release verifier."""
from __future__ import annotations

import hashlib
import pathlib
import stat
import struct
import sys
import tempfile
import unittest
import warnings
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from verify_vst3 import EXPECTED, MODULE, inspect, verify_pe  # noqa: E402

TARGET = EXPECTED + "/" + MODULE


def fake_pe(machine: int = 0x8664, magic: int = 0x20B, pe_offset: int = 0x80) -> bytes:
    data = bytearray(0x240)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, pe_offset)
    if 0 <= pe_offset <= len(data) - 26:
        data[pe_offset:pe_offset + 4] = b"PE\0\0"
        struct.pack_into("<H", data, pe_offset + 4, machine)
        struct.pack_into("<H", data, pe_offset + 24, magic)
    return bytes(data)


class VST3VerifierTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self.temp.name)
        self.module = fake_pe()
        self.digest = hashlib.sha256(self.module).hexdigest()

    def tearDown(self) -> None:
        self.temp.cleanup()

    def make_bundle(self, root: pathlib.Path) -> pathlib.Path:
        bundle = root / EXPECTED
        module = bundle / MODULE
        module.parent.mkdir(parents=True)
        module.write_bytes(self.module)
        return bundle

    def make_zip(self, path: pathlib.Path, extra_entries=()) -> pathlib.Path:
        with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_STORED) as archive:
            archive.writestr(TARGET, self.module)
            for entry in extra_entries:
                name, data, attrs = entry
                if attrs is None:
                    archive.writestr(name, data)
                else:
                    info = zipfile.ZipInfo(name)
                    info.create_system = 3
                    info.external_attr = attrs
                    archive.writestr(info, data)
        return path

    def test_accepts_exact_bundle_directory_and_reports_module_hash(self) -> None:
        bundle = self.make_bundle(self.root)
        result = inspect(bundle)
        self.assertEqual(result["module_sha256"], self.digest)
        self.assertEqual(result["module_bytes"], len(self.module))

    def test_accepts_flat_zip_with_exact_module_path(self) -> None:
        archive = self.make_zip(self.root / "preview.zip")
        result = inspect(archive)
        self.assertEqual(result["module_sha256"], self.digest)

    def test_rejects_wrong_architecture_or_pe32(self) -> None:
        with self.assertRaisesRegex(ValueError, "AMD64 PE32\\+"):
            verify_pe(fake_pe(machine=0x014C, magic=0x10B))

    def test_rejects_invalid_pe_offset(self) -> None:
        with self.assertRaisesRegex(ValueError, "invalid PE offset"):
            verify_pe(fake_pe(pe_offset=0x20))

    def test_rejects_additional_vst3_module_in_directory(self) -> None:
        bundle = self.make_bundle(self.root)
        extra = bundle / "Contents" / "x86_64-win" / "Unexpected.vst3"
        extra.write_bytes(self.module)
        with self.assertRaisesRegex(ValueError, "exactly one module"):
            inspect(bundle)

    def test_rejects_unsafe_zip_path(self) -> None:
        archive = self.make_zip(
            self.root / "unsafe.zip",
            [("../outside.txt", b"escape", None)],
        )
        with self.assertRaisesRegex(ValueError, "unsafe ZIP entry"):
            inspect(archive)

    def test_rejects_duplicate_zip_entries(self) -> None:
        archive = self.root / "duplicate.zip"
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_STORED) as z:
                z.writestr(TARGET, self.module)
                z.writestr(TARGET, self.module)
        with self.assertRaisesRegex(ValueError, "duplicate ZIP entries"):
            inspect(archive)

    def test_rejects_case_colliding_zip_entries(self) -> None:
        archive = self.make_zip(
            self.root / "case.zip",
            [(TARGET.swapcase(), self.module, None)],
        )
        with self.assertRaisesRegex(ValueError, "case-colliding"):
            inspect(archive)

    def test_rejects_symlink_zip_entry(self) -> None:
        attrs = (stat.S_IFLNK | 0o777) << 16
        archive = self.make_zip(
            self.root / "symlink.zip",
            [(TARGET + ".link", b"../outside", attrs)],
        )
        with self.assertRaisesRegex(ValueError, "symlink ZIP entry"):
            inspect(archive)

    def test_rejects_corrupt_zip_crc(self) -> None:
        archive = self.make_zip(self.root / "corrupt.zip")
        with zipfile.ZipFile(archive) as z:
            info = z.getinfo(TARGET)
        raw = bytearray(archive.read_bytes())
        name_length, extra_length = struct.unpack_from("<HH", raw, info.header_offset + 26)
        data_offset = info.header_offset + 30 + name_length + extra_length
        raw[data_offset + 4] ^= 0x01
        archive.write_bytes(raw)
        with self.assertRaisesRegex(ValueError, "ZIP CRC failure"):
            inspect(archive)


if __name__ == "__main__":
    unittest.main(verbosity=2)
