# Skell AdlibLab private Windows VST3 preview installer

This helper is for owner testing of an unsigned engineering preview. It is not a commercial installer, license manager, or automatic update service.

## Requirements

- Windows x64.
- PowerShell 7 or newer.
- The complete extracted preview package, including BUILD-INFO.json, SHA256SUMS.txt, VST3-VERIFICATION.json, dependency notices, the VST3 bundle, this guide and the installer script.

## Preview installation

Extract the ZIP to a normal user-writable folder. Open PowerShell 7 in that folder and first inspect the proposed install:

    .\Install-SkellAdlibLabPreview.ps1 -WhatIf

After reviewing the destination, install explicitly:

    .\Install-SkellAdlibLabPreview.ps1 -Confirm

Default destination:

    %LOCALAPPDATA%\Programs\Common\VST3\Skell AdlibLab.vst3

The installer verifies the complete SHA256SUMS inventory, strict JSON, product/vendor/version, the two protected VST3 class IDs, the AMD64 PE32+ module, pluginval provenance and the preview-only notice before changing an existing installation.

## Updating

Run the same installer from a newer validated preview package. Before replacing an existing bundle, it creates a file-tree-hash-verified backup under:

    %LOCALAPPDATA%\Skelly\Skell-AdlibLab\backups

The installer stages and re-verifies the new bundle before swapping it into place. If the swap fails, it attempts to restore the prior directory. It does not elevate, write to Program Files, modify the registry, alter host projects, or touch presets.

## Rolling back

Use the exact backup directory printed by the update operation:

    .\Install-SkellAdlibLabPreview.ps1 -RollbackBackup "C:\Users\<you>\AppData\Local\Skelly\Skell-AdlibLab\backups\<backup-folder>" -Confirm

Rollback verifies the backup identity and full bundle-tree digest. It preserves the currently installed version as another verified backup before restoring the selected version.

## Important limitations

- The helper is unsigned and manually invoked; it is not an automatic updater.
- The ZIP is private-preview-only and not approved for commercial redistribution.
- FL Studio scan/load, host state/preset/automation recall, real musical A/B and target-hardware CPU/latency still require owner testing.
- Licensing, attribution, support/contact, code signing and commercial terms remain open.
