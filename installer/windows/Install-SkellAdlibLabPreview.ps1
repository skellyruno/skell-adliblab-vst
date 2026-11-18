[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = 'Medium')]
param(
    [string]$PackagePath = $PSScriptRoot,
    [string]$InstallRoot = '',
    [string]$BackupRoot = '',
    [string]$RollbackBackup = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Product = 'Skell AdlibLab'
$Vendor = 'Skell Audio'
$BundleName = 'Skell AdlibLab.vst3'
$ModuleRelative = 'Contents/x86_64-win/Skell AdlibLab.vst3'
$InfoRelative = 'Contents/Resources/moduleinfo.json'
$VersionPattern = '^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$'
$CidPattern = '^[0-9A-Fa-f]{32}$'
$ExpectedCids = @(
    'abcdef019182faeb536b417541644c31',
    'abcdef011234abcd536b417541644c31'
)

function Get-FullPath {
    param([Parameter(Mandatory = $true)][string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) { throw 'A required path is empty.' }
    return [System.IO.Path]::GetFullPath($Path)
}

function Get-NormalizedPath {
    param([Parameter(Mandatory = $true)][string]$Path)
    $full = Get-FullPath $Path
    $root = [System.IO.Path]::GetPathRoot($full)
    if ($full.Length -gt $root.Length) {
        $full = $full.TrimEnd([char[]]@('\', '/'))
    }
    return $full
}

function Test-PathWithin {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Root
    )
    $candidate = Get-NormalizedPath $Path
    $base = Get-NormalizedPath $Root
    if ($candidate.Equals($base, [System.StringComparison]::OrdinalIgnoreCase)) { return $true }
    $volumeRoot = [System.IO.Path]::GetPathRoot($base)
    if ($base.Equals($volumeRoot, [System.StringComparison]::OrdinalIgnoreCase)) { return $true }
    $prefix = $base + [System.IO.Path]::DirectorySeparatorChar
    return $candidate.StartsWith($prefix, [System.StringComparison]::OrdinalIgnoreCase)
}

function Assert-NotFilesystemRoot {
    param([Parameter(Mandatory = $true)][string]$Path, [Parameter(Mandatory = $true)][string]$Label)
    $full = Get-NormalizedPath $Path
    $root = Get-NormalizedPath ([System.IO.Path]::GetPathRoot($full))
    if ($full.Equals($root, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "$Label must not be a filesystem root."
    }
}

function Assert-NoReparseAncestors {
    param([Parameter(Mandatory = $true)][string]$Path)
    $cursor = Get-FullPath $Path
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $item = Get-Item -LiteralPath $cursor -Force
            if (($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
                throw "Refusing a reparse-point path component: $cursor"
            }
        }
        $parent = [System.IO.Directory]::GetParent($cursor)
        if ($null -eq $parent) { break }
        $cursor = $parent.FullName
    }
}

function Assert-NoReparsePoints {
    param([Parameter(Mandatory = $true)][string]$Root)
    if (-not (Test-Path -LiteralPath $Root -PathType Container)) {
        throw "Expected directory does not exist: $Root"
    }
    Assert-NoReparseAncestors $Root
    foreach ($item in @(Get-ChildItem -LiteralPath $Root -Force -Recurse)) {
        if (($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "Reparse point is not allowed: $($item.FullName)"
        }
    }
}

function Assert-UniqueJsonProperties {
    param(
        [Parameter(Mandatory = $true)][System.Text.Json.JsonElement]$Element,
        [string]$Location = '$'
    )
    if ($Element.ValueKind -eq [System.Text.Json.JsonValueKind]::Object) {
        $seen = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
        foreach ($property in $Element.EnumerateObject()) {
            if (-not $seen.Add($property.Name)) {
                throw "Duplicate JSON property '$($property.Name)' at $Location"
            }
            Assert-UniqueJsonProperties -Element $property.Value -Location "$Location.$($property.Name)"
        }
    }
    elseif ($Element.ValueKind -eq [System.Text.Json.JsonValueKind]::Array) {
        $index = 0
        foreach ($child in $Element.EnumerateArray()) {
            Assert-UniqueJsonProperties -Element $child -Location "$Location[$index]"
            $index++
        }
    }
}

function Read-StrictJson {
    param([Parameter(Mandatory = $true)][string]$Path)
    $encoding = [System.Text.UTF8Encoding]::new($false, $true)
    $text = $encoding.GetString([System.IO.File]::ReadAllBytes($Path))
    $document = $null
    try {
        $document = [System.Text.Json.JsonDocument]::Parse($text)
        Assert-UniqueJsonProperties -Element $document.RootElement
        $value = ConvertFrom-Json -InputObject $text -AsHashtable -Depth 100 -ErrorAction Stop
        if ($value -isnot [System.Collections.IDictionary]) { throw 'JSON root must be an object.' }
        return $value
    }
    catch {
        throw "Strict JSON validation failed for '$Path': $($_.Exception.Message)"
    }
    finally {
        if ($null -ne $document) { $document.Dispose() }
    }
}

function Get-FileRecords {
    param([Parameter(Mandatory = $true)][string]$Root)
    $rootFull = Get-FullPath $Root
    $files = @(Get-ChildItem -LiteralPath $rootFull -File -Recurse -Force | Sort-Object FullName)
    $records = foreach ($file in $files) {
        $relative = [System.IO.Path]::GetRelativePath($rootFull, $file.FullName).Replace('\', '/')
        [pscustomobject]@{
            Path = $relative
            Sha256 = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
            SizeBytes = [long]$file.Length
        }
    }
    return @($records | Sort-Object Path -CaseSensitive)
}

function Get-TreeDigest {
    param([Parameter(Mandatory = $true)][string]$BundlePath)
    $records = @(Get-FileRecords $BundlePath)
    if ($records.Count -eq 0) { throw "Bundle contains no files: $BundlePath" }
    $lines = @($records | ForEach-Object { "$($_.Path)|$($_.Sha256)|$($_.SizeBytes)" })
    $payload = ($lines -join [System.Environment]::NewLine) + [System.Environment]::NewLine
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        return [System.Convert]::ToHexString($sha.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($payload))).ToLowerInvariant()
    }
    finally {
        $sha.Dispose()
    }
}

function Assert-ValidBundle {
    param([Parameter(Mandatory = $true)][string]$BundlePath)
    if (-not (Test-Path -LiteralPath $BundlePath -PathType Container)) {
        throw "Missing VST3 bundle: $BundlePath"
    }
    Assert-NoReparsePoints $BundlePath
    $module = Join-Path $BundlePath ($ModuleRelative.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
    $moduleInfo = Join-Path $BundlePath ($InfoRelative.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
    if (-not (Test-Path -LiteralPath $module -PathType Leaf)) { throw "Missing Windows x64 VST3 module: $ModuleRelative" }
    if (-not (Test-Path -LiteralPath $moduleInfo -PathType Leaf)) { throw "Missing VST3 moduleinfo.json: $InfoRelative" }

    $files = @(Get-ChildItem -LiteralPath $BundlePath -File -Recurse -Force)
    $modules = @($files | Where-Object { $_.Extension -ieq '.vst3' })
    if ($modules.Count -ne 1 -or $modules[0].FullName -ne (Get-FullPath $module)) {
        throw "Bundle must contain exactly one module at $ModuleRelative."
    }
    $forbidden = @($files | Where-Object { $_.Extension -in @('.exe', '.zip', '.component', '.vst2', '.app') })
    if ($forbidden.Count -gt 0) { throw "Unexpected non-VST3 payload: $($forbidden[0].FullName)" }

    $info = Read-StrictJson $moduleInfo
    if ($info['Name'] -cne $Product -or $info['Factory Info']['Vendor'] -cne $Vendor) {
        throw 'VST3 product/vendor identity does not match Skell AdlibLab / Skell Audio.'
    }
    if ([string]$info['Version'] -notmatch $VersionPattern) { throw "Invalid VST3 version: $($info['Version'])" }
    $classes = @($info['Classes'])
    if ($classes.Count -ne 2) { throw 'VST3 metadata must contain exactly two classes.' }
    $categories = @($classes | ForEach-Object { [string]$_['Category'] } | Sort-Object -CaseSensitive)
    if (($categories -join '|') -cne 'Audio Module Class|Component Controller Class') {
        throw 'VST3 component/controller categories are missing or duplicated.'
    }

    $cids = @()
    foreach ($class in $classes) {
        if ($class['Name'] -cne $Product -or $class['Vendor'] -cne $Vendor -or
            [string]$class['Version'] -cne [string]$info['Version']) {
            throw 'VST3 class identity does not match the bundle identity.'
        }
        $cid = [string]$class['CID']
        if ($cid -notmatch $CidPattern) { throw 'VST3 class contains an invalid 128-bit CID.' }
        $cids += $cid.ToLowerInvariant()
    }
    $actualCids = @($cids | Sort-Object -CaseSensitive)
    $expectedCids = @($ExpectedCids | Sort-Object -CaseSensitive)
    if (($actualCids -join '|') -cne ($expectedCids -join '|')) {
        throw 'VST3 component/controller CIDs do not match the protected AdlibLab identity.'
    }

    $binary = [System.IO.File]::ReadAllBytes($module)
    if ($binary.Length -lt 0x100 -or [System.Text.Encoding]::ASCII.GetString($binary, 0, 2) -cne 'MZ') {
        throw 'VST3 module is not a valid Windows PE image.'
    }
    $peOffset = [System.BitConverter]::ToInt32($binary, 0x3C)
    if ($peOffset -lt 0x40 -or $peOffset + 26 -gt $binary.Length) { throw 'PE header offset is invalid.' }
    if ($binary[$peOffset] -ne 80 -or $binary[$peOffset + 1] -ne 69 -or
        $binary[$peOffset + 2] -ne 0 -or $binary[$peOffset + 3] -ne 0) {
        throw 'PE signature is missing.'
    }
    $machine = [System.BitConverter]::ToUInt16($binary, $peOffset + 4)
    $sectionCount = [System.BitConverter]::ToUInt16($binary, $peOffset + 6)
    $optionalHeaderSize = [System.BitConverter]::ToUInt16($binary, $peOffset + 20)
    $characteristics = [System.BitConverter]::ToUInt16($binary, $peOffset + 22)
    $optionalMagic = [System.BitConverter]::ToUInt16($binary, $peOffset + 24)
    if ($machine -ne 0x8664 -or $optionalMagic -ne 0x20B -or
        ($characteristics -band 0x2000) -eq 0 -or $sectionCount -eq 0 -or
        $optionalHeaderSize -lt 0x70 -or $peOffset + 24 + $optionalHeaderSize -gt $binary.Length) {
        throw 'VST3 module must be a structurally valid AMD64 PE32+ DLL.'
    }

    return [pscustomobject]@{
        Version = [string]$info['Version']
        ModuleInfo = $info
        ModulePath = $module
        ModuleInfoPath = $moduleInfo
        BinarySha256 = (Get-FileHash -LiteralPath $module -Algorithm SHA256).Hash.ToLowerInvariant()
        ModuleInfoSha256 = (Get-FileHash -LiteralPath $moduleInfo -Algorithm SHA256).Hash.ToLowerInvariant()
        ClassIds = $actualCids
        BinarySize = [long](Get-Item -LiteralPath $module).Length
    }
}

function Assert-PreviewPackage {
    param([Parameter(Mandatory = $true)][string]$PackagePath)
    $root = Get-FullPath $PackagePath
    Assert-NoReparsePoints $root
    $expectedTop = @(
        'BUILD-INFO.json',
        'INSTALL-PREVIEW.md',
        'Install-SkellAdlibLabPreview.ps1',
        'PREVIEW-ONLY.txt',
        'README.md',
        'RELEASE-NOTES.md',
        'SHA256SUMS.txt',
        'Skell AdlibLab.vst3',
        'THIRD-PARTY-NOTICES',
        'VST3-VERIFICATION.json'
    ) | Sort-Object -CaseSensitive
    $actualTop = @(Get-ChildItem -LiteralPath $root -Force | ForEach-Object { $_.Name } | Sort-Object -CaseSensitive)
    if (($actualTop -join '|') -cne ($expectedTop -join '|')) {
        throw "Preview package top-level contents are unexpected. Expected: $($expectedTop -join ', ')"
    }

    $bundle = Join-Path $root $BundleName
    $bundleCheck = Assert-ValidBundle $bundle
    $manifest = Read-StrictJson (Join-Path $root 'BUILD-INFO.json')
    if ($manifest['product'] -cne $Product -or $manifest['version'] -cne $bundleCheck.Version -or
        $manifest['format'] -cne 'VST3' -or $manifest['platform'] -cne 'Windows x86_64' -or
        -not ([string]$manifest['release_status']).StartsWith('preview-only', [System.StringComparison]::OrdinalIgnoreCase)) {
        throw 'Build manifest product, version, platform or preview status is invalid.'
    }
    foreach ($field in @('source_head_commit', 'ci_checkout_commit', 'juce_resolved_commit', 'skellcore_commit', 'skellcore_resolved_commit')) {
        if ([string]$manifest[$field] -notmatch '^[0-9a-f]{40}$') { throw "Build provenance field is invalid: $field" }
    }
    if ([string]$manifest['workflow_run_id'] -notmatch '^\d+$' -or
        $manifest['pluginval_version'] -cne 'v2.0.0-beta3' -or
        [int]$manifest['pluginval_strictness'] -ne 5 -or
        $manifest['skellcore_commit'] -cne $manifest['skellcore_resolved_commit']) {
        throw 'Build provenance or pluginval gate is invalid.'
    }

    $verification = Read-StrictJson (Join-Path $root 'VST3-VERIFICATION.json')
    if ($verification['module_sha256'] -cne $bundleCheck.BinarySha256 -or
        [long]$verification['module_bytes'] -ne $bundleCheck.BinarySize) {
        throw 'VST3 verification report does not match the staged binary.'
    }

    $noticeRoot = Join-Path $root 'THIRD-PARTY-NOTICES'
    foreach ($notice in @('JUCE-LICENSE.md', 'README.txt', 'SkellCore-SOURCE-PROVENANCE.md', 'SkellyTune-MIT.txt')) {
        if (-not (Test-Path -LiteralPath (Join-Path $noticeRoot $notice) -PathType Leaf)) {
            throw "Required dependency notice is missing: $notice"
        }
    }
    $previewText = [System.IO.File]::ReadAllText((Join-Path $root 'PREVIEW-ONLY.txt'))
    if ($previewText -notmatch 'preview-only; not a commercial release' -or
        $previewText -notmatch 'Unsigned private per-user installer') {
        throw 'Preview-only notice is missing or does not describe the installer correctly.'
    }

    $sumPath = Join-Path $root 'SHA256SUMS.txt'
    $sumMap = [System.Collections.Generic.Dictionary[string,string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($line in @(Get-Content -LiteralPath $sumPath -Encoding ascii)) {
        if ($line -notmatch '^([0-9a-f]{64})  (.+)$') { throw "Malformed SHA256SUMS line: $line" }
        $relative = $Matches[2]
        if ([System.IO.Path]::IsPathRooted($relative) -or $relative.Contains('\') -or
            $relative.Contains(':') -or @($relative.Split('/') | Where-Object { $_ -in @('', '.', '..') }).Count -gt 0) {
            throw "Unsafe checksum path: $relative"
        }
        if ($sumMap.ContainsKey($relative)) { throw "Duplicate or case-colliding checksum path: $relative" }
        $sumMap.Add($relative, $Matches[1])
    }

    $sumFull = Get-FullPath $sumPath
    $expectedPaths = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($file in @(Get-ChildItem -LiteralPath $root -File -Recurse -Force)) {
        if ((Get-FullPath $file.FullName) -cne $sumFull) {
            $relative = [System.IO.Path]::GetRelativePath($root, $file.FullName).Replace('\', '/')
            [void]$expectedPaths.Add($relative)
        }
    }
    if ($sumMap.Count -ne $expectedPaths.Count) { throw 'SHA256SUMS does not cover the exact package file inventory.' }
    foreach ($relative in $expectedPaths) {
        if (-not $sumMap.ContainsKey($relative)) { throw "SHA256SUMS is missing: $relative" }
        $native = $relative.Replace('/', [System.IO.Path]::DirectorySeparatorChar)
        $file = Join-Path $root $native
        if (-not (Test-Path -LiteralPath $file -PathType Leaf)) { throw "Checksum target is missing: $relative" }
        $actualHash = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -cne $sumMap[$relative]) { throw "SHA-256 mismatch: $relative" }
    }

    return [pscustomobject]@{
        Root = $root
        BundlePath = $bundle
        Version = $bundleCheck.Version
        BundleTreeSha256 = Get-TreeDigest $bundle
        Manifest = $manifest
    }
}

function Assert-Backup {
    param([Parameter(Mandatory = $true)][string]$BackupPath)
    $root = Get-FullPath $BackupPath
    Assert-NoReparsePoints $root
    $expected = @('backup-info.json', $BundleName) | Sort-Object -CaseSensitive
    $actual = @(Get-ChildItem -LiteralPath $root -Force | ForEach-Object { $_.Name } | Sort-Object -CaseSensitive)
    if (($actual -join '|') -cne ($expected -join '|')) { throw 'Backup contents are unexpected.' }
    $info = Read-StrictJson (Join-Path $root 'backup-info.json')
    $bundle = Join-Path $root $BundleName
    $check = Assert-ValidBundle $bundle
    $digest = Get-TreeDigest $bundle
    if ($info['schema_version'] -ne 1 -or $info['product'] -cne $Product -or
        $info['vendor'] -cne $Vendor -or $info['version'] -cne $check.Version -or
        $info['bundle_tree_sha256'] -cne $digest) {
        throw 'Backup identity or file-tree checksum is invalid.'
    }
    return [pscustomobject]@{ BundlePath = $bundle; Version = $check.Version; BundleTreeSha256 = $digest }
}

function New-VerifiedBackup {
    param(
        [Parameter(Mandatory = $true)][string]$BundlePath,
        [Parameter(Mandatory = $true)][string]$BackupRoot
    )
    if (-not (Test-Path -LiteralPath $BackupRoot -PathType Container)) {
        New-Item -ItemType Directory -Path $BackupRoot -Force | Out-Null
    }
    Assert-NoReparseAncestors $BackupRoot
    $sourceCheck = Assert-ValidBundle $BundlePath
    $name = "Skell-AdlibLab-$($sourceCheck.Version)-$([Guid]::NewGuid().ToString('N'))"
    $temporary = Join-Path $BackupRoot ".$name.tmp"
    $final = Join-Path $BackupRoot $name
    try {
        New-Item -ItemType Directory -Path $temporary | Out-Null
        Copy-Item -LiteralPath $BundlePath -Destination (Join-Path $temporary $BundleName) -Recurse
        $digest = Get-TreeDigest (Join-Path $temporary $BundleName)
        $info = [ordered]@{
            schema_version = 1
            product = $Product
            vendor = $Vendor
            version = $sourceCheck.Version
            bundle_tree_sha256 = $digest
            created_utc = [DateTime]::UtcNow.ToString('o')
        }
        $json = ($info | ConvertTo-Json -Depth 5) + [System.Environment]::NewLine
        [System.IO.File]::WriteAllText((Join-Path $temporary 'backup-info.json'), $json, [System.Text.UTF8Encoding]::new($false))
        [void](Assert-Backup $temporary)
        Move-Item -LiteralPath $temporary -Destination $final
        return $final
    }
    catch {
        if (Test-Path -LiteralPath $temporary -PathType Container) {
            Remove-Item -LiteralPath $temporary -Recurse -Force
        }
        throw
    }
}

function Invoke-InstallSwap {
    param(
        [Parameter(Mandatory = $true)][string]$SourceBundle,
        [Parameter(Mandatory = $true)][string]$InstallRoot,
        [Parameter(Mandatory = $true)][string]$TargetPath,
        [Parameter(Mandatory = $true)][string]$ExpectedDigest
    )
    $token = [Guid]::NewGuid().ToString('N')
    $stage = Join-Path $InstallRoot ".Skell-AdlibLab-stage-$token"
    $old = Join-Path $InstallRoot ".Skell-AdlibLab-old-$token"
    $failed = Join-Path $InstallRoot ".Skell-AdlibLab-failed-$token"
    $hadTarget = Test-Path -LiteralPath $TargetPath -PathType Container
    try {
        Copy-Item -LiteralPath $SourceBundle -Destination $stage -Recurse
        if ((Get-TreeDigest $stage) -cne $ExpectedDigest) { throw 'Staged VST3 tree checksum mismatch.' }
        if ($hadTarget) { Move-Item -LiteralPath $TargetPath -Destination $old }
        try {
            Move-Item -LiteralPath $stage -Destination $TargetPath
            if ((Get-TreeDigest $TargetPath) -cne $ExpectedDigest) { throw 'Installed VST3 tree checksum mismatch.' }
        }
        catch {
            if (Test-Path -LiteralPath $TargetPath -PathType Container) {
                Move-Item -LiteralPath $TargetPath -Destination $failed
            }
            if ($hadTarget -and (Test-Path -LiteralPath $old -PathType Container)) {
                Move-Item -LiteralPath $old -Destination $TargetPath
                $hadTarget = $false
            }
            throw
        }
        if (Test-Path -LiteralPath $old -PathType Container) {
            Remove-Item -LiteralPath $old -Recurse -Force
        }
    }
    finally {
        if (Test-Path -LiteralPath $stage -PathType Container) {
            Remove-Item -LiteralPath $stage -Recurse -Force
        }
        if ($hadTarget -and (Test-Path -LiteralPath $old -PathType Container) -and
            -not (Test-Path -LiteralPath $TargetPath)) {
            Move-Item -LiteralPath $old -Destination $TargetPath
        }
    }
}

if ([string]::IsNullOrWhiteSpace($env:LOCALAPPDATA) -and
    ([string]::IsNullOrWhiteSpace($InstallRoot) -or [string]::IsNullOrWhiteSpace($BackupRoot))) {
    throw 'LOCALAPPDATA is unavailable; pass both -InstallRoot and -BackupRoot explicitly.'
}
if ([string]::IsNullOrWhiteSpace($InstallRoot)) {
    $InstallRoot = Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3'
}
if ([string]::IsNullOrWhiteSpace($BackupRoot)) {
    $BackupRoot = Join-Path $env:LOCALAPPDATA 'Skelly\Skell-AdlibLab\backups'
}

$InstallRoot = Get-FullPath $InstallRoot
$BackupRoot = Get-FullPath $BackupRoot
Assert-NotFilesystemRoot -Path $InstallRoot -Label 'InstallRoot'
Assert-NotFilesystemRoot -Path $BackupRoot -Label 'BackupRoot'
if ((Test-PathWithin -Path $InstallRoot -Root $BackupRoot) -or
    (Test-PathWithin -Path $BackupRoot -Root $InstallRoot)) {
    throw 'InstallRoot and BackupRoot must be separate, non-overlapping directories.'
}
Assert-NoReparseAncestors $InstallRoot
Assert-NoReparseAncestors $BackupRoot
if (Test-Path -LiteralPath $InstallRoot -PathType Leaf) { throw 'InstallRoot exists as a file.' }
if (Test-Path -LiteralPath $BackupRoot -PathType Leaf) { throw 'BackupRoot exists as a file.' }

$TargetPath = Join-Path $InstallRoot $BundleName

if (-not [string]::IsNullOrWhiteSpace($RollbackBackup)) {
    $RollbackBackup = Get-FullPath $RollbackBackup
    if (-not (Test-PathWithin -Path $RollbackBackup -Root $BackupRoot) -or
        $RollbackBackup.Equals((Get-NormalizedPath $BackupRoot), [System.StringComparison]::OrdinalIgnoreCase)) {
        throw 'RollbackBackup must be a backup directory inside BackupRoot.'
    }
    $backup = Assert-Backup $RollbackBackup
    $currentVersion = $null
    $currentDigest = $null
    if (Test-Path -LiteralPath $TargetPath -PathType Container) {
        $currentCheck = Assert-ValidBundle $TargetPath
        $currentVersion = $currentCheck.Version
        $currentDigest = Get-TreeDigest $TargetPath
    }
    if ($currentDigest -and $currentDigest -ceq $backup.BundleTreeSha256) {
        return [pscustomobject]@{ Status = 'AlreadyCurrent'; InstallPath = $TargetPath; BackupPath = $RollbackBackup; Version = $backup.Version }
    }
    if (-not $PSCmdlet.ShouldProcess($TargetPath, "Roll back Skell AdlibLab to $($backup.Version)")) {
        return [pscustomobject]@{ Status = 'WhatIf'; InstallPath = $TargetPath; BackupPath = $RollbackBackup; Version = $backup.Version }
    }
    if (-not (Test-Path -LiteralPath $InstallRoot -PathType Container)) {
        New-Item -ItemType Directory -Path $InstallRoot -Force | Out-Null
    }
    $currentBackup = $null
    if ($currentVersion) {
        $currentBackup = New-VerifiedBackup -BundlePath $TargetPath -BackupRoot $BackupRoot
    }
    Invoke-InstallSwap -SourceBundle $backup.BundlePath -InstallRoot $InstallRoot -TargetPath $TargetPath -ExpectedDigest $backup.BundleTreeSha256
    return [pscustomobject]@{
        Status = 'RolledBack'
        InstallPath = $TargetPath
        BackupPath = $RollbackBackup
        PreservedCurrentVersion = $currentBackup
        Version = $backup.Version
    }
}

$package = Assert-PreviewPackage $PackagePath
if ((Test-PathWithin -Path $InstallRoot -Root $package.Root) -or
    (Test-PathWithin -Path $package.Root -Root $InstallRoot) -or
    (Test-PathWithin -Path $BackupRoot -Root $package.Root) -or
    (Test-PathWithin -Path $package.Root -Root $BackupRoot)) {
    throw 'PackagePath, InstallRoot and BackupRoot must be separate, non-overlapping directories.'
}

$currentVersion = $null
$currentDigest = $null
if (Test-Path -LiteralPath $TargetPath) {
    if (-not (Test-Path -LiteralPath $TargetPath -PathType Container)) { throw 'Existing plugin target is not a directory.' }
    $currentCheck = Assert-ValidBundle $TargetPath
    $currentVersion = $currentCheck.Version
    $currentDigest = Get-TreeDigest $TargetPath
}
if ($currentDigest -and $currentDigest -ceq $package.BundleTreeSha256) {
    return [pscustomobject]@{ Status = 'AlreadyCurrent'; InstallPath = $TargetPath; Version = $package.Version }
}

$action = if ($currentVersion) { 'Update' } else { 'Install' }
if (-not $PSCmdlet.ShouldProcess($TargetPath, "$action private-preview Skell AdlibLab $($package.Version)")) {
    return [pscustomobject]@{ Status = 'WhatIf'; InstallPath = $TargetPath; Version = $package.Version }
}
if (-not (Test-Path -LiteralPath $InstallRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $InstallRoot -Force | Out-Null
}
$backupPath = $null
if ($currentVersion) {
    $backupPath = New-VerifiedBackup -BundlePath $TargetPath -BackupRoot $BackupRoot
}
Invoke-InstallSwap -SourceBundle $package.BundlePath -InstallRoot $InstallRoot -TargetPath $TargetPath -ExpectedDigest $package.BundleTreeSha256
$status = if ($currentVersion) { 'Updated' } else { 'Installed' }
return [pscustomobject]@{
    Status = $status
    InstallPath = $TargetPath
    BackupPath = $backupPath
    PreviousVersion = $currentVersion
    Version = $package.Version
    BundleTreeSha256 = $package.BundleTreeSha256
}
