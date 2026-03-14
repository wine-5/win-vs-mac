#!/bin/bash
# 初期化方法のチェックスクリプト
# プリミティブ型とポインタが () で初期化されていないかチェック

set +e

FAILED=0
ERROR_OUTPUT=""

echo "=== Checking initialization style ==="

# srcディレクトリ内のすべての.cppファイルを検索
FILES=$(find src -name "*.cpp" -o -name "*.h")

for file in $FILES; do
    # thirdpartyは除外
    if echo "$file" | grep -q "thirdparty"; then
        continue
    fi
    
    # コンストラクタ初期化リストで () を使っている箇所を検出
    # 例: : m_count(0), m_speed(5.0f)
    INIT_LIST_VIOLATIONS=$(grep -n "^\s*:" "$file" -A 20 | \
        grep -E "m_[a-zA-Z_][a-zA-Z0-9_]*\(" | \
        grep -v "std::" | \
        grep -v "make_unique" | \
        grep -v "make_shared")
    
    if [ -n "$INIT_LIST_VIOLATIONS" ]; then
        echo "❌ Found () initialization in member initializer list: $file"
        ERROR_OUTPUT="$ERROR_OUTPUT\n=== $file ===\n$INIT_LIST_VIOLATIONS\n"
        FAILED=1
    fi
    
    # ローカル変数でプリミティブ型が () で初期化されている箇所を検出
    # 例: int count(0);
    LOCAL_VAR_VIOLATIONS=$(grep -nE "^\s+(int|float|double|bool|char|long|short|size_t|uint|uint32_t|int32_t)\s+[a-zA-Z_][a-zA-Z0-9_]*\(" "$file" | \
        grep -v "//" | \
        grep -v "return")
    
    if [ -n "$LOCAL_VAR_VIOLATIONS" ]; then
        echo "⚠️  Found () initialization for primitive types: $file"
        echo "$LOCAL_VAR_VIOLATIONS"
        # ローカル変数は警告のみ（エラーにはしない）
    fi
done

if [ "$FAILED" -eq 1 ]; then
    echo ""
    echo "================================================================"
    echo "ERROR: Member initializer lists must use {} instead of ()"
    echo "================================================================"
    echo -e "$ERROR_OUTPUT"
    echo ""
    echo "Fix example:"
    echo "  ❌ Bad:  Player::Player(int hp) : m_hp(hp), m_speed(0.0f) {}"
    echo "  ✅ Good: Player::Player(int hp) : m_hp{hp}, m_speed{0.0f} {}"
    echo ""
    exit 1
fi

echo "✅ All initialization styles are correct"
exit 0
