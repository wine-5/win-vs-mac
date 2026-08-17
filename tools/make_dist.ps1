<#
.SYNOPSIS
    配布用フォルダとZIPを1コマンドで組み立てる。

.DESCRIPTION
    ゲームは assets / web を「カレントディレクトリからの相対パス」で開くため
    （ResourceManager.cpp や WebView2Host.cpp を参照）、実行ファイルと同じ階層に
    assets / web / SampleFiles が揃っていないと起動できない。
    毎回手でコピーすると入れ忘れが起きるので、ここに手順を固定する。

    生成物: dist\WinVsMac\      … そのまま実行できるフォルダ
            dist\WinVsMac_日付.zip … 配布用ZIP

.PARAMETER NoBuild
    ビルドを省略し、既存の実行ファイルを使って詰め直すだけにする。

.PARAMETER NoZip
    ZIPを作らず、フォルダの組み立てだけで止める。

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\make_dist.ps1
#>
param(
    [switch]$NoBuild,
    [switch]$NoZip,
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',
    [string]$Platform = 'x64'
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$distRoot = Join-Path $repoRoot 'dist'
$stageDir = Join-Path $distRoot 'WinVsMac'
$outDir   = Join-Path $repoRoot "$Platform\$Configuration"

# 実行ファイル名は構成ごとに違う（vcxproj で Release だけ TargetName を差し替えているため）
if ($Configuration -eq 'Release') { $exeName = 'WinVsMac.exe' } else { $exeName = 'DxLib-3D.exe' }

# 実行に必要なフォルダ。ここに足し忘れると配布物だけ動かなくなる
$contentDirs = @('assets', 'web', 'ファイルはこちらからでも選べます')

function Write-Step($text) {
    Write-Host ''
    Write-Host "==> $text" -ForegroundColor Cyan
}

function Invoke-Robocopy($source, $destination) {
    # /MIR は宛先を source と同じ状態にする（消したファイルが配布物に残らない）
    # /XF は Explorer が勝手に置いていくファイル。配布物に入れる意味がない
    robocopy $source $destination /MIR /XF desktop.ini Thumbs.db .DS_Store /NFL /NDL /NJH /NJS /NP | Out-Null
    # robocopy は「コピーした」だけで 1 を返す。8 以上が本当の失敗
    if ($LASTEXITCODE -ge 8)
        { throw "robocopy に失敗しました（$source → $destination / 終了コード $LASTEXITCODE）" }
    $global:LASTEXITCODE = 0
}

# ------------------------------------------------------------
# 1. ビルド
# ------------------------------------------------------------
if (-not $NoBuild) {
    Write-Step "ビルド（$Configuration|$Platform）"

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere))
        { throw 'vswhere.exe が見つかりません。Visual Studio がインストールされているか確認してください' }

    $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if ([string]::IsNullOrWhiteSpace($vsPath))
        { throw 'MSBuild を持つ Visual Studio が見つかりません' }

    $msbuild = Join-Path $vsPath 'MSBuild\Current\Bin\MSBuild.exe'
    if (-not (Test-Path $msbuild))
        { throw "MSBuild.exe が見つかりません: $msbuild" }

    # WebView2 は NuGet 参照。packages\ が無いと途中で止まるので先に気づけるようにする
    $webview2Targets = Join-Path $repoRoot 'packages\Microsoft.Web.WebView2.1.0.3967.48\build\native\Microsoft.Web.WebView2.targets'
    if (-not (Test-Path $webview2Targets))
        { throw 'WebView2 の NuGet パッケージが復元されていません。Visual Studio でソリューションを開き直すか nuget restore を実行してください' }

    & $msbuild (Join-Path $repoRoot 'DxLib-3D.sln') `
        /p:Configuration=$Configuration /p:Platform=$Platform /m /v:minimal /nologo
    if ($LASTEXITCODE -ne 0)
        { throw "ビルドに失敗しました（終了コード $LASTEXITCODE）" }
}

$exePath = Join-Path $outDir $exeName
if (-not (Test-Path $exePath))
    { throw "実行ファイルが見つかりません: $exePath" }

# ------------------------------------------------------------
# 2. 配布フォルダを組み立てる
# ------------------------------------------------------------
Write-Step '配布フォルダを組み立て'

if (Test-Path $stageDir) { Remove-Item $stageDir -Recurse -Force }
New-Item -ItemType Directory -Path $stageDir | Out-Null

Copy-Item $exePath (Join-Path $stageDir $exeName)
Write-Host "  $exeName"

# WebView2Loader.dll は NuGet がビルド時に出力先へ置く。無いとセレクト画面が開かない
$loaderPath = Join-Path $outDir 'WebView2Loader.dll'
if (-not (Test-Path $loaderPath))
    { throw "WebView2Loader.dll が見つかりません: $loaderPath" }
Copy-Item $loaderPath (Join-Path $stageDir 'WebView2Loader.dll')
Write-Host '  WebView2Loader.dll'

foreach ($dir in $contentDirs) {
    $source = Join-Path $repoRoot $dir
    if (-not (Test-Path $source))
        { throw "$dir フォルダが見つかりません: $source" }
    Invoke-Robocopy $source (Join-Path $stageDir $dir)
    $count = (Get-ChildItem (Join-Path $stageDir $dir) -Recurse -File).Count
    Write-Host ("  {0}\ ({1} ファイル)" -f $dir, $count)
}

# ------------------------------------------------------------
# 3. 配布物に混ざってはいけないものを弾く
# ------------------------------------------------------------
Write-Step '不要ファイルの確認'

# .pdb はデバッグ情報、*.exe.WebView2 は実行時に作られるユーザーデータ、
# ログ類は手元で遊んだ痕跡。どれも配布物に入れる意味がない
$junk = Get-ChildItem $stageDir -Recurse -Force |
    Where-Object { $_.Name -like '*.pdb' -or $_.Name -like '*.WebView2' -or $_.Name -like '*_log.txt' -or $_.Name -eq 'settings.json' }

if ($junk) {
    foreach ($item in $junk) {
        Remove-Item $item.FullName -Recurse -Force
        Write-Host ("  除外: {0}" -f $item.Name)
    }
} else {
    Write-Host '  なし'
}

$total = (Get-ChildItem $stageDir -Recurse -File | Measure-Object -Property Length -Sum)
Write-Host ('  合計 {0:N1} MB / {1} ファイル' -f ($total.Sum / 1MB), $total.Count)

# ------------------------------------------------------------
# 4. ZIP
# ------------------------------------------------------------
if (-not $NoZip) {
    Write-Step 'ZIPを作成'

    # ビルドの取り違えを防ぐため日付を入れる。同日に作り直したら上書きする
    $stamp   = Get-Date -Format 'yyyyMMdd'
    $zipPath = Join-Path $distRoot "WinVsMac_$stamp.zip"
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }

    # Compress-Archive は同梱物に .zip があると「使用中」で失敗することがある。
    # .NET の実装なら起きないうえに速い
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory(
        $stageDir, $zipPath, [System.IO.Compression.CompressionLevel]::Optimal, $true)
    Write-Host ('  {0} ({1:N1} MB)' -f $zipPath, ((Get-Item $zipPath).Length / 1MB))
}

Write-Host ''
Write-Host "完成: $stageDir" -ForegroundColor Green
exit 0
