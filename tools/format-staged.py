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

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ns_comments  # noqa: E402

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
        encoding="utf-8",
        errors="replace",
    )
    for name in out.splitlines():
        if os.path.splitext(name)[1].lower() in EXTENSIONS:
            yield name


def changed_line_ranges(path):
    """ステージ済み差分から、新しい側の変更行範囲 (start, end) を返す。"""
    out = subprocess.check_output(
        ["git", "diff", "--cached", "-U0", "--", path],
        encoding="utf-8",
        errors="replace",
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
        changed = False

        # ① 変更した行だけを整形する
        ranges = changed_line_ranges(path)
        if ranges:
            args = [clang_format, "-i", "--style=file"]
            for start, end in ranges:
                args.append(f"--lines={start}:{end}")
            args.append(path)
            subprocess.check_call(args)
            changed = True

        # ② 名前空間の閉じ } にコメントが無ければ追加する
        if add_namespace_comments(path):
            changed = True

        if changed:
            subprocess.check_call(["git", "add", "--", path])

    return 0


def add_namespace_comments(path):
    """名前空間コメントを追記した場合 True を返す。"""
    with open(path, "rb") as f:
        src = f.read().decode("utf-8", errors="replace")
    out = ns_comments.add_namespace_comments(src)
    if out == src:
        return False
    with open(path, "wb") as f:
        f.write(out.encode("utf-8"))
    return True


if __name__ == "__main__":
    sys.exit(main())
