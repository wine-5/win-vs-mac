#pragma once
#include "core/data/FileExtensionType.h"
#include "game/constant/EnemyType.h"
#include <vector>

namespace game::component::stage
{
	/**
	 * @brief 殴って壊せる配置物のコンポーネント
	 *
	 * 壊れるまでの寿命をHPではなく「打撃回数」で持つ。
	 * 攻撃力が伸びても壊すのに必要な手数が変わらないため、
	 * ひびの段階が必ず全部見える。HP制にすると育ったビルドで一撃になり、
	 * 段階を用意した意味が無くなる。
	 */
	struct DestructibleComponent
	{
		// 破壊に必要な打撃回数（stageCatalog.jsonのhitsToBreak）
		int m_hitsToBreak{ 0 };

		// これまでに受けた打撃回数
		int m_hitCount{ 0 };

		// 壊したときに落とす拡張子の種別と個数
		core::data::FileExtensionType m_dropType{ core::data::FileExtensionType::Unknown };
		int m_dropCount{ 0 };

		// 種別を1個ごとに抽選するか。ZIPのように「中身が分からない」ブロック用。
		// trueのとき m_dropType は使わない
		bool m_isDropRandom{ false };

		// 壊すと拡張子を挿せる枠が1つ増えるか（RAMブロック）。
		// 欠片とは別の報酬なので、落とす種別・個数とは独立に持つ
		bool m_grantsEquipSlot{ false };

		// 壊したときに湧く敵の種類と数（隔離フォルダ）。0体なら敵は出ない
		constant::EnemyType m_spawnEnemyType{ constant::EnemyType::Xcode };
		int m_spawnEnemyCount{ 0 };

		// 敵の代わりに当たりを引く確率（0〜1）と、当たったときに装備中の拡張子の
		// 効果へ掛ける倍率。倍率は重ねがけせず、一度掛かったら以降の当たりでは伸びない
		float m_jackpotChance{ 0.0f };
		float m_jackpotStatMultiplier{ 1.0f };

		// 無傷のあいだ描くモデル
		int m_intactHandle{ -1 };

		// あらかじめ割ってあるモデル。破壊した瞬間からこちらへ切り替える
		int m_fracturedHandle{ -1 };

		// ひび段階のテクスチャ。[0]が無傷で、以降が段階1〜のひび
		std::vector<int> m_crackTextures{};

		/**
		 * @brief 現在の被弾回数に対応するひび段階を返す
		 *
		 * 打撃回数とひびの枚数は一致しないため（例: 3回で壊れるブロックへ
		 * ひび3枚）、最後の1発手前が一番派手なひびになるよう割り振る。
		 * 素直に「回数＝段階」にすると、3回で壊れるブロックでは最後のひびが
		 * 表示される前に破壊が走り、一度も見られない
		 * @return テクスチャ配列の添字（0=無傷）
		 */
		[[nodiscard]] int crackStage() const noexcept
		{
			if (m_crackTextures.size() <= 1 || m_hitsToBreak <= 1 || m_hitCount <= 0)
				return 0;

			// 破壊する打撃（m_hitsToBreak回目）はひびを進めないので、
			// ひびが進むのは m_hitsToBreak - 1 回ぶん
			const int lastStage{ static_cast<int>(m_crackTextures.size()) - 1 };
			const int crackHits{ m_hitsToBreak - 1 };
			const int stage{ (m_hitCount * lastStage + crackHits - 1) / crackHits };
			return stage < lastStage ? stage : lastStage;
		}

		/**
		 * @brief 次の打撃で壊れるか
		 * @return 壊れるならtrue
		 */
		[[nodiscard]] bool isBroken() const noexcept
		{
			return m_hitsToBreak > 0 && m_hitCount >= m_hitsToBreak;
		}
	};
} // namespace game::component::stage
