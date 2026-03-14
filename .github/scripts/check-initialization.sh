#!/bin/bash
# 初期化方法のチェックスクリプト
# プリミティブ型とポインタが () で初期化されていないかチェック

set +e

FAILED=0
TEMP_FILE=$(mktemp)

echo "=== Checking initialization style ==="

# srcディレクトリ内のすべての.cppと.hファイルを検索
find src -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/thirdparty/*" | while read -r file; do
    
    # メンバー初期化リストで () を検出
    # パターン: : で始まる行から20行以内の m_xxx( を検出
    awk '
    /^\s*:/ { in_init=1; init_line=NR }
    in_init && NR <= init_line + 20 {
        if (/m_[a-zA-Z_][a-zA-Z0-9_]*\(/ && 
            !/std::/ && 
            !/make_unique/ && 
            !/make_shared/ && 
            !/\.get\(/ && 
            !/->\w*\(/) {
            print FILENAME":"NR": 初期化は {} を使ってください（() は非推奨）"
            print "    "$0
        }
    }
    /{/ && in_init { in_init=0 }
    ' "$file" >> "$TEMP_FILE"
done

# 結果を表示
if [ -s "$TEMP_FILE" ]; then
    cat "$TEMP_FILE"
    ERROR_COUNT=$(grep -c "初期化は {} を使ってください" "$TEMP_FILE")
    echo ""
    echo "================================================================"
    echo "❌ 初期化スタイルエラー: ${ERROR_COUNT}件"
    echo "================================================================"
    echo ""
    echo "修正例:"
    echo "  ❌ Bad:  MyClass::MyClass() : m_count(0), m_speed(5.0f) {}"
    echo "  ✅ Good: MyClass::MyClass() : m_count{0}, m_speed{5.0f} {}"
    echo ""
    rm -f "$TEMP_FILE"
    exit 1
else
    echo ""
    echo "================================================================"
    echo "✅ すべての初期化スタイルが正しいです"
    echo "================================================================"
    rm -f "$TEMP_FILE"
    exit 0
fi
