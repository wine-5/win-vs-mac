#!/usr/bin/env python3
"""ステージした C/C++ ファイルの「変更した行だけ」を clang-format で整形する。

コミット時（pre-commit フック）から呼ばれる想定。
ファイル全体ではなく変更行のみを対象にするため、触っていない箇所
（例: 手動で列を揃えた switch 文）は一切変わらない。
"""
import os
import re
import shutil
import subprocess
import sys

# 対象拡張子
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}

# Visual Studio 同梱の clang-format のフォールバックパス
FALLBACK_CLANG_FORMAT = [
    r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\bin\clang-format.exe",
    r"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\Llvm\bin\clang-format.exe",
    r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\Llvm\bin\clang-format.exe",
]

HUNK_RE = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")


def find_clang_format():
    exe = shutil.which("clang-format")
    if exe:
        return exe
    for path in FALLBACK_CLANG_FORMAT:
        if os.path.isfile(path):
            return path
    return None


def staged_files():
    out = subprocess.check_output(
        ["git", "diff", "--cached", "--name-only", "--diff-filter=ACM"],
        text=True,
    )
    for name in out.splitlines():
        if os.path.splitext(name)[1].lower() in EXTENSIONS:
            yield name


def changed_line_ranges(path):
    """ステージ済み差分から、新しい側の変更行範囲 (start, end) を返す。"""
    out = subprocess.check_output(
        ["git", "diff", "--cached", "-U0", "--", path], text=True
    )
    ranges = []
    for line in out.splitlines():
        m = HUNK_RE.match(line)
        if not m:
            continue
        start = int(m.group(1))
        count = int(m.group(2)) if m.group(2) is not None else 1
        if count == 0:  # 削除のみの箇所は整形対象なし
            continue
        ranges.append((start, start + count - 1))
    return ranges


def main():
    clang_format = find_clang_format()
    if not clang_format:
        print("[format-staged] clang-format が見つかりません。整形をスキップします。",
              file=sys.stderr)
        return 0

    for path in staged_files():
        if not os.path.isfile(path):
            continue
        ranges = changed_line_ranges(path)
        if not ranges:
            continue
        args = [clang_format, "-i", "--style=file"]
        for start, end in ranges:
            args.append(f"--lines={start}:{end}")
        args.append(path)
        subprocess.check_call(args)
        subprocess.check_call(["git", "add", "--", path])

    return 0


if __name__ == "__main__":
    sys.exit(main())
