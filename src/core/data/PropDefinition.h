#pragma once
#include <string>
#include "core/utility/Vector3.h"

namespace core::data
{
	/**
	 * @brief 配置物の種類定義（stageCatalog.jsonのprops[]要素1つ分）
	 *
	 * PropMetadata.m_type からこの定義を引き、モデルパスと素材実寸（baseSize）・
	 * コライダー種別を解決する。エディタとゲームの両方がstageCatalog.jsonを読むが、
	 * ゲーム側が必要とするのはモデル解決に使うこれらの項目のみ。
	 */
	struct PropDefinition
	{
		std::string m_id{};
		std::string m_modelPath{};
		core::Vector3 m_baseSize{}; // モデル素材の実寸。size ÷ baseSize がモデルスケールになる
		std::string m_collider{};   // "box" | "ground" | "none"

		// 破壊に必要な打撃回数。0なら壊せない普通の配置物。
		// HPではなく回数にしているのは、攻撃力が伸びても壊すのに必要な手数を一定に保つため。
		// HP制にすると育ったビルドで一撃になり、ひびの段階が誰の目にも触れなくなる
		int m_hitsToBreak{ 0 };

		// 壊したときに落とす拡張子の種別名（extensionBonus.json のキーと同じ綴り）。
		// "random" なら1個ごとに種別を抽選する（ZIPのように中身が分からないブロック用）。
		// 空なら何も落とさない
		std::string m_dropExtension{};

		// 落とす欠片の個数。ZIPは複数のファイルを含むので多めに落とす
		int m_dropCount{ 0 };

		// 壊すと拡張子を挿せる枠が1つ増えるか（RAMブロック）。
		// 欠片を落とすのとは別の報酬なので、dropExtension とは独立に持つ
		bool m_grantsEquipSlot{ false };

		// 壊したときに湧く敵の種類名（"xcode" / "safari"）と数。空なら敵は出ない。
		// 種類ではなく「壊すと敵が出る」という振る舞いで持つので、
		// 見た目違いのブロックへ同じ性質を付けられる
		std::string m_spawnEnemyType{};
		int m_spawnEnemyCount{ 0 };

		// 敵の代わりに当たりを引く確率（0〜1）と、当たったときに装備中の拡張子の効果へ
		// 掛ける倍率。分の悪い賭けを成立させるのは当たりの大きさなので、
		// 確率と倍率は必ず対で調整する
		float m_extensionBoostChance{ 0.0f };
		float m_extensionBoostMultiplier{ 1.0f };

		// 特別な役割。空なら普通の配置物。"bossGate" はボス出現で閉じる扉として扱う。
		// 種類（id）ではなく役割で判定することで、見た目違いの扉を何種類でも用意できる
		std::string m_role{};
		// テクスチャ1枚が受け持つ実寸（ユニット）。0なら繰り返さず面いっぱいに引き伸ばす。
		// 引き伸ばした壁で窓が間延びしないよう、実寸に応じて模様を繰り返させる
		float m_textureTile{ 0.0f };

		// 坂を滑り落ちる加速度。0なら滑らない普通の足場。
		// 歩き速度より滑りが速くなるよう設定すると「ダッシュしないと登れない坂」になる
		float m_slideAccel{ 0.0f };

		// 動く歩道として乗っている者を運ぶ速さ（ユニット/秒）。0なら運ばない。
		// 向きは配置物のローカル+Z（長辺）で、配置をY180度回せばそのまま逆走になる。
		// テクスチャの流れる向きもこの値から導くので、見た目と力が食い違うことはない
		float m_conveyorSpeed{ 0.0f };

		// テクスチャを流す速さ（1.0でテクスチャ1枚ぶん/秒）。0なら流れない。
		// 壁の情報が流れる「サーバー内部」の演出に使う。縦横それぞれ指定できるので、
		// 縦に流れる壁と横に流れる壁を配置物の種類として作り分けられる
		float m_scrollU{ 0.0f };
		float m_scrollV{ 0.0f };
	};
} // namespace core::data
