#pragma once
#include <string>
#include "core/utility/Vector3.h"

namespace game::component::visual
{
	/**
	 * @brief 武器などのモデルを自分のボーン（フレーム）へ装着する情報を持つコンポーネント
	 *
	 * 装着先は名前で指定する。ボーン名はモデルのリグによって異なるため、
	 * 名前からフレーム番号への解決は WeaponAttachSystem が起動後に一度だけ行う。
	 * 実際の描画は InGameView が本体モデルを描いた後に行う。
	 *
	 * @note m_modelHandle には武器専用のハンドルを持たせること。装着描画は行列を
	 *       直接指定するため、他の描き方と共有すると位置指定が効かなくなる
	 */
	struct WeaponAttachComponent
	{
		int m_modelHandle{ -1 };

		// 装着先ボーン名と、そこから解決したフレーム番号（-1は未解決）
		std::string m_frameName{};
		int m_frameIndex{ -1 };

		// 解決を試行済みか。見つからなかった場合に毎フレーム検索とログ出力を
		bool m_isResolved{ false };

		// 装着先ボーンのローカル空間で解釈される、握りの位置・角度の調整値。
		// スケールは親（キャラクター）のスケールに乗算される
		core::Vector3 m_offsetPosition{};
		core::Vector3 m_offsetRotation{};
		core::Vector3 m_offsetScale{ 1.0f, 1.0f, 1.0f };

		bool m_isVisible{ true };
	};
} // namespace game::component::visual
