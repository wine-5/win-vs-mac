#pragma once

namespace core::data
{ /**
   * @brief 拡張子によるパラメータボーナス値を保持する構造体
   */
	struct FileExtensionBonus
	{
		float atk{ 0.0f };
		float spd{ 0.0f };
		float def{ 0.0f };
		float hp{ 0.0f };
		float attackRange{ 0.0f };

		// クリティカルの発生率への加算（0.05なら発生率+5ポイント。倍率ではなく確率のほう）
		float criticalRate{ 0.0f };

		// Window弾の弾速への加算（ワールド単位/秒）
		float projectileSpeed{ 0.0f };

		// Window弾の飛距離への加算（ワールド単位）。弾速とは独立して効く
		float projectileRange{ 0.0f };
	};
} // namespace core::data