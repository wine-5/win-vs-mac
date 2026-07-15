#pragma once

namespace game::data
{ /**
   * @brief 拡張子によるパラメータボーナス値を保持する構造体
   */
	struct FileExtensionBonus
	{
		float atk{0.0f};
		float spd{0.0f};
		float def{0.0f};
		float hp{0.0f};
		float attackRange{0.0f};
	};
} // namespace game::data