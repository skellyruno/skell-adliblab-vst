param(
    [Parameter(Mandatory = $true)][string]$PackagePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ($PSVersionTable.PSVersion.Major -lt 7) { throw 'Installer integration tests require PowerShell 7 or newer.' }
$PackagePath = [System.IO.Path]::GetFullPath($PackagePath)
$Installer = Join-Path $PackagePath 'Install-SkellAdlibLabPreview.ps1'
if (-not (Test-Path -LiteralPath $Installer -PathType Leaf)) { throw 'Installer script is missing from the staged package.' }

function Get-TreeDigest {
    param([string]$Bundle)
    $root = [System.IO.Path]::GetFullPath($Bundle)
    $records = foreach ($file in @(Get-ChildItem -LiteralPath $root -File -Recurse -Force | Sort-Object FullName)) {
        $relative = [System.IO.Path]::GetRelativePath($root, $file.FullName).Replace('\', '/')
        $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        "$relative|$hash|$($file.Length)"
    }
    $lines = @($records | Sort-Object)
    $payload = (@($lines) -join [System.Environment]::NewLine) + [System.Environment]::NewLine
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try { return [System.Convert]::ToHexString($sha.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($payload))).ToLowerInvariant() }
    finally { $sha.Dispose() }
}

function Copy-Package {
    param([string]$Source, [string]$Destination)
    New-Item -ItemType Directory -Path $Destination | Out-Null
    foreach ($item in @(Get-ChildItem -LiteralPath $Source -Force)) {
        Copy-Item -LiteralPath $item.FullName -Destination $Destination -Recurse -Force
    }
}

function Invoke-ExpectedFailure {
    param([scriptblock]$Action, [string]$Label)
    $failed = $false
    try { & $Action | Out-Null } catch { $failed = $true }
    if (-not $failed) { throw "Expected rejection did not occur: $Label" }
    Write-Host "PASS: $Label rejected before changing the installed version"
}

$testRoot = Join-Path $env:TEMP "SkellAdlibLabInstallerTest-$([Guid]::NewGuid().ToString('N'))"
$ownerToken = [Guid]::NewGuid().ToString('N')
New-Item -ItemType Directory -Path $testRoot | Out-Null
$marker = Join-Path $testRoot '.skell-adliblab-installer-test-owner'
[System.IO.File]::WriteAllText($marker, $ownerToken, [System.Text.UTF8Encoding]::new($false))

try {
    $installRoot = Join-Path $testRoot 'user-vst3'
    $backupRoot = Join-Path $testRoot 'backups'
    $target = Join-Path $installRoot 'Skell AdlibLab.vst3'

    $dryRun = & $Installer -PackagePath $PackagePath -InstallRoot $installRoot -BackupRoot $backupRoot -WhatIf -Confirm:$false
    if ($dryRun.Status -cne 'WhatIf' -or (Test-Path -LiteralPath $installRoot) -or (Test-Path -LiteralPath $backupRoot)) {
        throw 'WhatIf created or changed an installation or backup path.'
    }
    Write-Host 'PASS: WhatIf made no filesystem changes'

    $first = & $Installer -PackagePath $PackagePath -InstallRoot $installRoot -BackupRoot $backupRoot -Confirm:$false
    if ($first.Status -cne 'Installed' -or -not (Test-Path -LiteralPath $target -PathType Container)) {
        throw 'Fresh per-user install failed.'
    }
    $originalTree = Get-TreeDigest $target
    Write-Host 'PASS: fresh per-user install'

    $second = & $Installer -PackagePath $PackagePath -InstallRoot $installRoot -BackupRoot $backupRoot -Confirm:$false
    if ($second.Status -cne 'AlreadyCurrent') { throw 'Repeated install was not idempotent.' }
    Write-Host 'PASS: idempotent install'

    $driveRoot = [System.IO.Path]::GetPathRoot($env:TEMP)
    $rootTarget = Join-Path $driveRoot 'Skell AdlibLab.vst3'
    $rootExisted = Test-Path -LiteralPath $rootTarget -PathType Container
    $rootDigest = if ($rootExisted) { Get-TreeDigest $rootTarget } else { $null }
    Invoke-ExpectedFailure -Label 'filesystem-root install destination' -Action {
        & $Installer -PackagePath $PackagePath -InstallRoot $driveRoot -BackupRoot $backupRoot -Confirm:$false
    }
    if ($rootExisted -and (Get-TreeDigest $rootTarget) -cne $rootDigest) {
        throw 'Filesystem-root rejection changed a pre-existing plugin directory.'
    }
    if (-not $rootExisted -and (Test-Path -LiteralPath $rootTarget)) {
        throw 'Filesystem-root rejection unexpectedly created a plugin directory.'
    }

    $packageChildInstall = Join-Path $PackagePath 'unsafe-install-root'
    Invoke-ExpectedFailure -Label 'install root inside package directory' -Action {
        & $Installer -PackagePath $PackagePath -InstallRoot $packageChildInstall -BackupRoot $backupRoot -Confirm:$false
    }
    if (Test-Path -LiteralPath $packageChildInstall) { throw 'Package-overlap rejection created an install directory.' }

    $unsafePackage = Join-Path $testRoot 'unsafe-path-package'
    Copy-Package -Source $PackagePath -Destination $unsafePackage
    $unsafeSumPath = Join-Path $unsafePackage 'SHA256SUMS.txt'
    $unsafeLines = @(Get-Content -LiteralPath $unsafeSumPath -Encoding ascii)
    $unsafeLines = @($unsafeLines | ForEach-Object {
        if ($_ -match '^([0-9a-f]{64})  BUILD-INFO\.json$') { "$($Matches[1])  ../BUILD-INFO.json" } else { $_ }
    })
    [System.IO.File]::WriteAllLines($unsafeSumPath, $unsafeLines, [System.Text.Encoding]::ASCII)
    Invoke-ExpectedFailure -Label 'checksum path traversal' -Action {
        & (Join-Path $unsafePackage 'Install-SkellAdlibLabPreview.ps1') -PackagePath $unsafePackage -InstallRoot $installRoot -BackupRoot $backupRoot -Confirm:$false
    }
    if ((Get-TreeDigest $target) -cne $originalTree) { throw 'Unsafe checksum path changed the installed bundle.' }

    $tamperedPackage = Join-Path $testRoot 'tampered-package'
    Copy-Package -Source $PackagePath -Destination $tamperedPackage
    $tamperedModule = Join-Path $tamperedPackage 'Skell AdlibLab.vst3/Contents/x86_64-win/Skell AdlibLab.vst3'
    $bytes = [System.IO.File]::ReadAllBytes($tamperedModule)
    $bytes[$bytes.Length - 1] = $bytes[$bytes.Length - 1] -bxor 1
    [System.IO.File]::WriteAllBytes($tamperedModule, $bytes)
    Invoke-ExpectedFailure -Label 'tampered package checksum' -Action {
        & (Join-Path $tamperedPackage 'Install-SkellAdlibLabPreview.ps1') -PackagePath $tamperedPackage -InstallRoot $installRoot -BackupRoot $backupRoot -Confirm:$false
    }
    if ((Get-TreeDigest $target) -cne $originalTree) { throw 'Tampered package changed the installed bundle.' }

    $updatePackage = Join-Path $testRoot 'update-package'
    Copy-Package -Source $PackagePath -Destination $updatePackage
    $moduleInfoPath = Join-Path $updatePackage 'Skell AdlibLab.vst3/Contents/Resources/moduleinfo.json'
    $moduleInfo = Get-Content -LiteralPath $moduleInfoPath -Raw | ConvertFrom-Json -AsHashtable -Depth 100
    $moduleInfo['Version'] = '0.10.3'
    foreach ($class in @($moduleInfo['Classes'])) { $class['Version'] = '0.10.3' }
    $moduleInfoJson = ($moduleInfo | ConvertTo-Json -Depth 100) + [System.Environment]::NewLine
    [System.IO.File]::WriteAllText($moduleInfoPath, $moduleInfoJson, [System.Text.UTF8Encoding]::new($false))
    $moduleInfoHash = (Get-FileHash -LiteralPath $moduleInfoPath -Algorithm SHA256).Hash.ToLowerInvariant()

    $buildInfoPath = Join-Path $updatePackage 'BUILD-INFO.json'
    $buildInfo = Get-Content -LiteralPath $buildInfoPath -Raw | ConvertFrom-Json -AsHashtable -Depth 100
    $buildInfo['version'] = '0.10.3'
    $buildInfoJson = ($buildInfo | ConvertTo-Json -Depth 100) + [System.Environment]::NewLine
    [System.IO.File]::WriteAllText($buildInfoPath, $buildInfoJson, [System.Text.UTF8Encoding]::new($false))
    $buildInfoHash = (Get-FileHash -LiteralPath $buildInfoPath -Algorithm SHA256).Hash.ToLowerInvariant()

    $sumPath = Join-Path $updatePackage 'SHA256SUMS.txt'
    $moduleInfoRelative = 'Skell AdlibLab.vst3/Contents/Resources/moduleinfo.json'
    $sumLines = foreach ($line in @(Get-Content -LiteralPath $sumPath -Encoding ascii)) {
        $separator = $line.IndexOf('  ')
        if ($separator -ne 64) { throw "Malformed test checksum line: $line" }
        $relative = $line.Substring(66)
        if ($relative -ceq $moduleInfoRelative) { "$moduleInfoHash  $relative" }
        elseif ($relative -ceq 'BUILD-INFO.json') { "$buildInfoHash  $relative" }
        else { $line }
    }
    [System.IO.File]::WriteAllLines($sumPath, @($sumLines), [System.Text.Encoding]::ASCII)

    $update = & $Installer -PackagePath $updatePackage -InstallRoot $installRoot -BackupRoot $backupRoot -Confirm:$false
    if ($update.Status -cne 'Updated' -or [string]::IsNullOrWhiteSpace($update.BackupPath) -or
        -not (Test-Path -LiteralPath $update.BackupPath -PathType Container)) {
        throw 'Update did not create a verified rollback backup.'
    }
    $updatedTree = Get-TreeDigest $target
    if ($updatedTree -ceq $originalTree) { throw 'Update did not change the installed package tree.' }
    $installedInfo = Get-Content -LiteralPath (Join-Path $target 'Contents/Resources/moduleinfo.json') -Raw | ConvertFrom-Json
    if ($installedInfo.Version -cne '0.10.3') { throw 'Updated plugin version was not installed.' }
    Write-Host 'PASS: update staged, swapped and backed up the previous version'

    $tamperedBackup = Join-Path $backupRoot 'tampered-backup-test'
    Copy-Package -Source $update.BackupPath -Destination $tamperedBackup
    $backupModule = Join-Path $tamperedBackup 'Skell AdlibLab.vst3/Contents/x86_64-win/Skell AdlibLab.vst3'
    $backupBytes = [System.IO.File]::ReadAllBytes($backupModule)
    $backupBytes[$backupBytes.Length - 1] = $backupBytes[$backupBytes.Length - 1] -bxor 1
    [System.IO.File]::WriteAllBytes($backupModule, $backupBytes)
    Invoke-ExpectedFailure -Label 'tampered rollback backup' -Action {
        & $Installer -RollbackBackup $tamperedBackup -InstallRoot $installRoot -BackupRoot $backupRoot -Confirm:$false
    }
    if ((Get-TreeDigest $target) -cne $updatedTree) { throw 'Tampered rollback backup changed the installed bundle.' }

    $rollback = & $Installer -RollbackBackup $update.BackupPath -InstallRoot $installRoot -BackupRoot $backupRoot -Confirm:$false
    if ($rollback.Status -cne 'RolledBack' -or (Get-TreeDigest $target) -cne $originalTree) {
        throw 'Rollback did not restore the original bundle tree.'
    }
    $restoredInfo = Get-Content -LiteralPath (Join-Path $target 'Contents/Resources/moduleinfo.json') -Raw | ConvertFrom-Json
    if ($restoredInfo.Version -cne '0.10.2') { throw 'Rollback did not restore the original plugin version.' }
    if ([string]::IsNullOrWhiteSpace($rollback.PreservedCurrentVersion) -or
        -not (Test-Path -LiteralPath $rollback.PreservedCurrentVersion -PathType Container)) {
        throw 'Rollback did not preserve the version it replaced.'
    }
    Write-Host 'PASS: rollback restored the original bundle and preserved the replaced version'
    Write-Host 'PASS: all private-preview installer integration checks completed'
}
finally {
    $safeToClean = $false
    if (Test-Path -LiteralPath $marker -PathType Leaf) {
        $safeToClean = ([System.IO.File]::ReadAllText($marker) -ceq $ownerToken)
    }
    if ($safeToClean) {
        $reparse = @(Get-ChildItem -LiteralPath $testRoot -Force -Recurse | Where-Object {
            ($_.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0
        })
        if ($reparse.Count -eq 0) {
            Remove-Item -LiteralPath $testRoot -Recurse -Force
        }
        else {
            Write-Warning "Test directory contains a reparse point and was preserved: $testRoot"
        }
    }
    else {
        Write-Warning "Test ownership marker changed; preserving test directory: $testRoot"
    }
}
