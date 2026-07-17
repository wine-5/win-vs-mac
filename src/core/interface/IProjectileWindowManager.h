#pragma once
#include <cstddef>
#include <vector>

namespace core::iface
{
	// 同時に表示する弾ウィンドウの上限
	// （いくつまで出すかはゲーム側のポリシーなのでcore層で定義し、Platform層はこれを参照する）
	constexpr std::size_t MAX_PROJECTILE_WINDOWS{ 10 };

	/**
	 * @brief 弾に追従させる実OSウィンドウ1枚分の画面上の配置情報
	 *
	 * 座標はゲームウィンドウのクライアント座標系（DxLibのスクリーン座標）。
	 * デスクトップ座標への変換はPlatform層が行う。
	 */
	struct ProjectileWindowInfo
	{
		int m_centerX{ 0 }; // 中心X（クライアント座標）
		int m_centerY{ 0 }; // 中心Y（クライアント座標）
		int m_size{ 0 };    // 一辺のピクセルサイズ
	};

	/**
	 * @brief 弾の見た目として実OSウィンドウを表示・追従させる純粋仮想クラス
	 *
	 * Game層がPlatform層（Win32）に直接依存しないための抽象化。
	 * 当たり判定・移動はゲーム内の弾エンティティが真であり、
	 * 本インターフェースはその位置に「見た目」を追従させるだけ。
	 */
	class IProjectileWindowManager
	{
	  public:
		virtual ~IProjectileWindowManager() = default;

		/**
		 * @brief 今フレームの弾の配置一覧に合わせてウィンドウ群を表示・移動する
		 * @param infos 表示する弾の配置一覧（空なら全ウィンドウを隠す）
		 */
		virtual void updateWindows(const std::vector<ProjectileWindowInfo>& infos) = 0;

		/**
		 * @brief 全ウィンドウを非表示にする
		 */
		virtual void hideAll() = 0;
	};
} // namespace core::iface
