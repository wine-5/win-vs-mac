#pragma once

namespace game::constant
{
    /**
     * @brief 衝突判定のタグ
     * オブジェクトの種類を識別する
     */
    enum class Tag
    {
        None,
        Player,
        Enemy,
        Ground,
        Wall,

		// 殴って壊せる配置物。床・壁と見た目は同じ立方体だが、攻撃の対象になる点が違う。
		Destructible,
	};

	/**
	 * @brief ステージの配置物（床・壁・柱・壊せるブロック）を指すタグか
	 *
	 * 押し返し・カメラの遮蔽・弾の遮断は、どれも「そこに物があるか」だけを見ており、
	 * 壊せるかどうかは関係ない。個々の判定で Ground と Destructible を並べると、
	 * タグを増やしたときに直し漏れた場所だけがすり抜けるため、ここへ集約する
	 * @param tag 判定するタグ
	 * @return 配置物ならtrue
	 */
	[[nodiscard]] constexpr bool isStageProp(Tag tag) noexcept
	{
		return tag == Tag::Ground || tag == Tag::Destructible;
	}
} // namespace game::constant