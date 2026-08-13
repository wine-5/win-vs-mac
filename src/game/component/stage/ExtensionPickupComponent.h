#pragma once
#include "core/data/FileExtensionType.h"
#include "core/utility/Vector3.h"

namespace game::component::stage
{
	/**
	 * @brief 落ちている拡張子の欠片を表すコンポーネント
	 *
	 * ブロックを壊すと出現し、プレイヤーが近づくと取得できる。
	 * 見た目はHUDのスロットと同じ拡張子アイコンをビルボードで描くため、
	 * 「このブロックを壊すとこれが手に入る」が絵で繋がる。
	 */
	struct ExtensionPickupComponent
	{
		// 拾ったときに手に入る拡張子の種別
		core::data::FileExtensionType m_type{ core::data::FileExtensionType::Unknown };

		// 出現してからの経過時間（浮遊の上下と、取得可能になるまでの待ちに使う）
		float m_elapsed{ 0.0f };

		// 飛び出した勢い。ブロックから弾け出てから落ちて着地する
		core::Vector3 m_velocity{};

		// 着地する高さ（ワールドY）。ここまで落ちたら浮遊へ切り替える
		float m_restY{ 0.0f };

		// 着地済みか。着地するまでは重力で落ち、着地後はその場で浮遊する
		bool m_isGrounded{ false };
	};
} // namespace game::component::stage
