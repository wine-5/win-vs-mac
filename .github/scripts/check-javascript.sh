#!/bin/bash

# JavaScript・Web UI 命名規則チェックスクリプト
# ドキュメント: docs/conventions/javascript_naming_convention.md

set -e

WEB_PATH="web"
VIOLATIONS=0
TEMP_VIOLATIONS="/tmp/js_violations.txt"

> "$TEMP_VIOLATIONS"

echo "🔍 JavaScript命名規則をチェック中..."

# ========================================
# 1. ファイル名がケバブケースか（JS・HTML・CSS）
# ========================================
echo "  ファイル名をチェック..." >&2

# JSファイル: ケバブケース [a-z0-9-]+.js
js_file_violations=$(find "$WEB_PATH" -name "*.js" -type f | grep -E '[A-Z_]' || true)

if [ -n "$js_file_violations" ]; then
  echo "❌ JSファイル名（ケバブケース必須）:" >> "$TEMP_VIOLATIONS"
  echo "$js_file_violations" | while read file; do
    echo "   $file:1" >> "$TEMP_VIOLATIONS"
  done
  VIOLATIONS=$((VIOLATIONS + $(echo "$js_file_violations" | wc -l)))
fi

# HTMLファイル: ケバブケース [a-z0-9-]+.html
html_file_violations=$(find "$WEB_PATH" -name "*.html" -type f | grep -E '[A-Z_]' || true)

if [ -n "$html_file_violations" ]; then
  echo "❌ HTMLファイル名（ケバブケース必須）:" >> "$TEMP_VIOLATIONS"
  echo "$html_file_violations" | while read file; do
    echo "   $file:1" >> "$TEMP_VIOLATIONS"
  done
  VIOLATIONS=$((VIOLATIONS + $(echo "$html_file_violations" | wc -l)))
fi

# CSSファイル: ケバブケース [a-z0-9-]+.css
css_file_violations=$(find "$WEB_PATH" -name "*.css" -type f | grep -E '[A-Z_]' || true)

if [ -n "$css_file_violations" ]; then
  echo "❌ CSSファイル名（ケバブケース必須）:" >> "$TEMP_VIOLATIONS"
  echo "$css_file_violations" | while read file; do
    echo "   $file:1" >> "$TEMP_VIOLATIONS"
  done
  VIOLATIONS=$((VIOLATIONS + $(echo "$css_file_violations" | wc -l)))
fi

# ========================================
# 2. フォルダ名が小文字か
# ========================================
echo "  フォルダ名をチェック..." >&2

folder_violations=$(find "$WEB_PATH" -type d | grep -E '[A-Z_]' | grep -v "^$WEB_PATH$" || true)

if [ -n "$folder_violations" ]; then
  echo "❌ フォルダ名（小文字必須）:" >> "$TEMP_VIOLATIONS"
  echo "$folder_violations" | while read file; do
    echo "   $file:1" >> "$TEMP_VIOLATIONS"
  done
  VIOLATIONS=$((VIOLATIONS + $(echo "$folder_violations" | wc -l)))
fi

# ========================================
# 3. IIFEモジュール名がPascalCaseか
# ========================================
echo "  IIFEモジュール名をチェック..." >&2

# const モジュール名 = (function() { ... }());
# ✅ OK例: const MyLogic = (function() または const JobView = (function()
# ❌ NG例: const myLogic = (function() または const MY_LOGIC = (function()
iife_violations=$(grep -rn "const [a-z_][a-zA-Z0-9_]*\s*=\s*(function" "$WEB_PATH" --include="*.js" 2>/dev/null || true)

if [ -n "$iife_violations" ]; then
  echo "❌ IIFEモジュール名（PascalCase必須）:" >> "$TEMP_VIOLATIONS"
  echo "$iife_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$iife_violations" | wc -l)))
fi

# ========================================
# 4. 関数名がcamelCaseか
# ========================================
echo "  関数名をチェック..." >&2

# function MyFunction(...) または function my_function(...) はNG
# ❌ NG例: function MyFunction(...) または function my_function(...)
# ✅ OK例: function myFunction(...)
js_func_violations=$(grep -rn "function [A-Z_][a-zA-Z0-9_]*\s*(" "$WEB_PATH" --include="*.js" 2>/dev/null || true)

if [ -n "$js_func_violations" ]; then
  echo "❌ 関数名（camelCase必須）:" >> "$TEMP_VIOLATIONS"
  echo "$js_func_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$js_func_violations" | wc -l)))
fi

# ========================================
# 5. グローバル定数がUPPER_SNAKE_CASE か
# ========================================
echo "  グローバル定数をチェック..." >&2

# const グローバル定数 = ... の形式をチェック
# ✅ OK例: const MAX_SLOTS = 3; const EXT_ICON = {...};
# ❌ NG例: const maxSlots = 3; (グローバル定数がcamelCase)
# 注意: IIFE内のローカル定数や関数の戻り値はcamelCaseで良い
global_const_violations=$(grep -rn "^const [a-z][a-z0-9]*\s*=" "$WEB_PATH" --include="*.js" | grep -v "= (function\|= {\|= \[\|= null\|= fetch\|= document\|= window\|= require\|= import" 2>/dev/null || true)

# さらに絞り込み: MAX_XXX, DIFFICULTY_XXX, DEFAULT_XXX パターンでない場合のみ
global_const_violations=$(echo "$global_const_violations" | grep -v "[A-Z_]" || true)

if [ -n "$global_const_violations" ]; then
  # 実際の定数（グローバルスコープ）かどうかを二重チェック - IIFE内のローカル定数を除外
  filtered=$(echo "$global_const_violations" | while read line; do
    file=$(echo "$line" | cut -d: -f1)
    lineno=$(echo "$line" | cut -d: -f2)
    # その行の前後を確認してIIFE内かどうかを判定
    if ! sed -n "1,${lineno}p" "$file" | grep -q "(function"; then
      echo "$line"
    fi
  done)

  if [ -n "$filtered" ]; then
    echo "⚠️  グローバル定数（UPPER_SNAKE_CASE推奨）:" >> "$TEMP_VIOLATIONS"
    echo "$filtered" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
    VIOLATIONS=$((VIOLATIONS + $(echo "$filtered" | wc -l)))
  fi
fi

# ========================================
# 6. HTMLのid属性がケバブケースか
# ========================================
echo "  HTML id属性をチェック..." >&2

# id="..." で大文字またはアンダースコアを含むもの
html_id_violations=$(grep -rn 'id="[^"]*[A-Z_]' "$WEB_PATH" --include="*.html" 2>/dev/null || true)

if [ -n "$html_id_violations" ]; then
  echo "❌ HTML id属性（ケバブケース必須）:" >> "$TEMP_VIOLATIONS"
  echo "$html_id_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$html_id_violations" | wc -l)))
fi

# ========================================
# 7. HTMLのdata-属性がケバブケースか
# ========================================
echo "  HTML data-属性をチェック..." >&2

# data-XXX で大文字またはアンダースコアを含むもの
html_data_violations=$(grep -rn 'data-[A-Z_]' "$WEB_PATH" --include="*.html" 2>/dev/null || true)

if [ -n "$html_data_violations" ]; then
  echo "❌ HTML data-属性（ケバブケース必須）:" >> "$TEMP_VIOLATIONS"
  echo "$html_data_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$html_data_violations" | wc -l)))
fi

# ========================================
# 8. HTMLのclass属性がケバブケースか
# ========================================
echo "  HTML class属性をチェック..." >&2

# class="..." で大文字またはアンダースコアを含むもの（スペース区切りのクラス名すべて）
html_class_violations=$(grep -rn 'class="[^"]*[A-Z_]' "$WEB_PATH" --include="*.html" 2>/dev/null || true)

if [ -n "$html_class_violations" ]; then
  echo "❌ HTML class属性（ケバブケース必須）:" >> "$TEMP_VIOLATIONS"
  echo "$html_class_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$html_class_violations" | wc -l)))
fi

# ========================================
# 9. CSSクラス名がケバブケースか
# ========================================
echo "  CSSクラス名をチェック..." >&2

# \.ClassName または \.ClassName--modifier で大文字を含むもの
css_class_violations=$(grep -rn '\.[A-Z_][a-zA-Z0-9_-]*\|[A-Z_][a-zA-Z0-9_-]*--[a-zA-Z0-9_-]*' "$WEB_PATH" --include="*.css" 2>/dev/null || true)

if [ -n "$css_class_violations" ]; then
  echo "❌ CSSクラス名（ケバブケース必須）:" >> "$TEMP_VIOLATIONS"
  echo "$css_class_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$css_class_violations" | wc -l)))
fi

# ========================================
# 10. コールバック関数が on + camelCase か
# ========================================
echo "  コールバック関数をチェック..." >&2

# ✅ OK例: .onJobSelected() や .onStatsUpdate()
# ❌ NG例: .on_job_selected() や .onJobSelected() 以外の命名
callback_violations=$(grep -rn '\.on[a-z_][a-zA-Z0-9_]*' "$WEB_PATH" --include="*.js" 2>/dev/null | grep -v '\.on[a-z][a-zA-Z0-9]*' || true)

if [ -n "$callback_violations" ]; then
  echo "⚠️  コールバック関数名（on + camelCase）:" >> "$TEMP_VIOLATIONS"
  echo "$callback_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$callback_violations" | wc -l)))
fi

# ========================================
# チェック結果を出力
# ========================================
echo ""

if [ "$VIOLATIONS" -eq 0 ]; then
  echo "✅ JavaScript命名規則コンプライアンス: 合格"
  exit 0
else
  echo "❌ JavaScript命名規則コンプライアンス: 失敗（$VIOLATIONS 件の違反検出）"
  echo ""
  echo "=== 違反一覧（ファイル:行番号） ===" >&2

  # ファイル名:行番号 の形式で出力（最初の5件）
  grep -E "^\s+[^:]+:[0-9]+$" "$TEMP_VIOLATIONS" | head -5 >&2

  violation_count=$(grep -c "^\s*[^:]*:[0-9]*$" "$TEMP_VIOLATIONS" || true)
  if [ "$violation_count" -gt 5 ]; then
    echo "   ... さらに $((violation_count - 5)) 件の違反があります" >&2
  fi

  exit 1
fi
