#!/usr/bin/env python3
"""名前空間の閉じ } に `// namespace X` コメントを付与するユーティリティ。

C++ を簡易トークナイズして名前空間の開き/閉じ括弧を対応付け、
閉じ括弧が単独で行にある複数行の名前空間に対してのみコメントを追記する。
コメント文言・付与条件は clang-format の FixNamespaceComments に合わせている。

・コメント/文字列/文字リテラル/生文字列内の括弧は無視する
・`using namespace` や `namespace x = y;`（エイリアス）は対象外
・本体が短い名前空間（1行以下）は対象外（clang-format の ShortNamespaceLines=1 相当）
・既にコメントが付いている場合は追記しない（冪等）
"""
import re

_IDENT = re.compile(r"[A-Za-z_]\w*")

# 本体がこの行数以下の名前空間にはコメントを付けない（clang-format の既定値と一致）
_SHORT_NAMESPACE_LINES = 1


def _namespace_close_map(text):
    """close行インデックス -> 名前空間名 の辞書を返す（付与対象のみ）。"""
    n = len(text)
    i = 0
    line = 0
    stack = []          # 各要素: 名前空間なら ("ns", name, open_line) / それ以外は None
    result = {}
    prev_word = ""      # 直前の識別子（using namespace 判定用）
    collecting = None   # 名前空間名収集中: {"parts": [...], "open_line": int}

    while i < n:
        c = text[i]

        # 改行
        if c == "\n":
            line += 1
            i += 1
            continue

        # 行コメント
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            i = n if j == -1 else j
            continue

        # ブロックコメント
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            if j == -1:
                break
            line += text.count("\n", i, j + 2)
            i = j + 2
            continue

        # 生文字列 R"delim( ... )delim"
        if c == "R" and i + 1 < n and text[i + 1] == '"':
            k = i + 2
            delim = ""
            while k < n and text[k] != "(":
                delim += text[k]
                k += 1
            end = ')' + delim + '"'
            j = text.find(end, k)
            if j == -1:
                break
            line += text.count("\n", i, j + len(end))
            i = j + len(end)
            prev_word = ""
            continue

        # 文字列 / 文字リテラル
        if c == '"' or c == "'":
            quote = c
            i += 1
            while i < n:
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == "\n":
                    line += 1
                if text[i] == quote:
                    i += 1
                    break
                i += 1
            prev_word = ""
            continue

        # 識別子
        m = _IDENT.match(text, i)
        if m:
            word = m.group(0)
            i = m.end()
            if collecting is not None:
                collecting["parts"].append(word)
            elif word == "namespace" and prev_word != "using":
                collecting = {"parts": [], "open_line": line}
            prev_word = word
            continue

        # 名前空間名の一部としての ::
        if collecting is not None and c == ":":
            collecting["parts"].append(":")
            i += 1
            continue

        # 開き括弧（open_line は開き { がある行）
        if c == "{":
            if collecting is not None:
                stack.append(("ns", "".join(collecting["parts"]), line))
                collecting = None
            else:
                stack.append(None)
            prev_word = ""
            i += 1
            continue

        # 閉じ括弧
        if c == "}":
            if collecting is not None:
                collecting = None
            top = stack.pop() if stack else None
            if top is not None and top[0] == "ns":
                _, name, open_line = top
                body_lines = line - open_line - 1
                if body_lines > _SHORT_NAMESPACE_LINES:
                    result[line] = name
            prev_word = ""
            i += 1
            continue

        # エイリアス等: namespace 名収集中に = や ; が来たら中止
        if collecting is not None and c in "=;":
            collecting = None

        if not c.isspace():
            prev_word = ""
        i += 1

    return result


def add_namespace_comments(text):
    """テキストに名前空間コメントを追記した新テキストを返す。変更が無ければ元と同一。"""
    close_map = _namespace_close_map(text)
    if not close_map:
        return text

    newline = "\r\n" if "\r\n" in text else "\n"
    trailing = text.endswith("\n")
    lines = text.splitlines()

    for idx, name in close_map.items():
        if idx >= len(lines):
            continue
        raw = lines[idx]
        if raw.strip() != "}":
            continue  # 閉じ括弧が単独行でない場合は触らない
        if "// namespace" in raw:
            continue  # 既にコメント済み
        comment = "// namespace" if not name else f"// namespace {name}"
        lines[idx] = raw.rstrip() + " " + comment

    body = newline.join(lines)
    if trailing:
        body += newline
    return body


if __name__ == "__main__":
    import sys

    for path in sys.argv[1:]:
        with open(path, "rb") as f:
            src = f.read().decode("utf-8")
        out = add_namespace_comments(src)
        if out != src:
            with open(path, "wb") as f:
                f.write(out.encode("utf-8"))
            print(f"updated: {path}")
