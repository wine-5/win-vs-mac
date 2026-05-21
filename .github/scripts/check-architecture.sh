#!/bin/bash

# アーキテクチャコンプライアンスチェックスクリプト
# 許可された依存方向: platform → infrastructure → game → core

set -e

SRC_PATH="src"
VIOLATIONS=0
TEMP_VIOLATIONS="/tmp/arch_violations.txt"

> "$TEMP_VIOLATIONS"

# ========================================
# 1. Core層が他層をインクルードしていないかチェック
# ========================================
core_violations=$(grep -rn "^#include.*\(game\|infrastructure\|platform\)/" "$SRC_PATH/core" 2>/dev/null || true)

if [ -n "$core_violations" ]; then
  echo "❌ Core層の違反:" >> "$TEMP_VIOLATIONS"
  echo "$core_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$core_violations" | wc -l)))
fi

# ========================================
# 2. Game層がinfrastructure/platformをインクルードしていないかチェック
# ========================================

# ヘッダーファイルのチェック（厳密）
game_h_violations=$(grep -rn "^#include.*\(infrastructure\|platform\)/" "$SRC_PATH/game" --include="*.h" 2>/dev/null || true)

if [ -n "$game_h_violations" ]; then
  echo "❌ Game層（ヘッダ）→ Infrastructure/Platform:" >> "$TEMP_VIOLATIONS"
  echo "$game_h_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$game_h_violations" | wc -l)))
fi

# 実装ファイルのチェック
game_cpp_platform=$(grep -rn "^#include.*platform/" "$SRC_PATH/game" --include="*.cpp" 2>/dev/null || true)

if [ -n "$game_cpp_platform" ]; then
  echo "⚠️  Game層（実装）→ Platform:" >> "$TEMP_VIOLATIONS"
  echo "$game_cpp_platform" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$game_cpp_platform" | wc -l)))
fi

# ========================================
# 3. Infrastructure層がplatformをインクルードしていないかチェック
# ========================================
infra_violations=$(grep -rn "^#include.*platform/" "$SRC_PATH/infrastructure" 2>/dev/null || true)

if [ -n "$infra_violations" ]; then
  echo "❌ Infrastructure層 → Platform:" >> "$TEMP_VIOLATIONS"
  echo "$infra_violations" | sed 's|^|   |' >> "$TEMP_VIOLATIONS"
  VIOLATIONS=$((VIOLATIONS + $(echo "$infra_violations" | wc -l)))
fi

# ========================================
# チェック結果を出力
# ========================================
echo ""

if [ "$VIOLATIONS" -eq 0 ]; then
  echo "✅ アーキテクチャコンプライアンス: 合格"
  exit 0
else
  echo "❌ アーキテクチャコンプライアンス: 失敗（$VIOLATIONS 件の違反検出）"
  echo ""
  echo "=== 違反一覧（ファイル:行番号） ==="
  echo ""

  # ファイル名:行番号 の形式で出力
  grep -E "^\s+src/" "$TEMP_VIOLATIONS" | cut -d':' -f1-2 | sed 's|^\s*||' | head -10

  TOTAL_FILES=$(grep -c '^\s*src/' "$TEMP_VIOLATIONS" || true)
  if [ "$TOTAL_FILES" -gt 5 ]; then
    echo "   ... さらに $(($TOTAL_FILES - 5)) 件の違反があります"
  fi

  # CI用に詳細ファイルを生成
  if [ -n "$CI" ] || [ -n "$GITHUB_ACTIONS" ]; then
    cat "$TEMP_VIOLATIONS" > arch_violations_detail.txt 2>/dev/null || true
  fi

  exit 1
fi
