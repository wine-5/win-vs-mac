#pragma once
#include "core/utility/Vector3.h"
#include "core/interface/IRenderer.h"
#include <unordered_map>
#include <vector>
#include <array>

namespace infrastructure
{
	/**
	 * @brief 3D描画を担当するクラス
	 */
	class Renderer : public core::iface::IRenderer
	{
	public:
		/**
		 * @brief 3Dモデルを描画する
		 * @param modelHandle モデルハンドル
		 * @param position 位置
		 * @param rotation 回転（ラジアン）
		 * @param scale スケール
		 */
		void drawModel(int modelHandle, const core::Vector3& position, const core::Vector3& rotation, const core::Vector3& scale) override;

		/**
		 * @brief 敵撃破時の赤化＋ディゾルブ（消失）演出をモデルに適用する
		 * @param modelHandle 対象のモデルハンドル
		 * @param progress 演出の進行度（0.0=死亡直後 〜 1.0=消失完了）
		 */
		void applyDeathDissolve(int modelHandle, float progress) override;

		/**
		 * @brief applyDeathDissolveで変更した見た目を元に戻す
		 * @param modelHandle 対象のモデルハンドル
		 */
		void resetModelAppearance(int modelHandle) override;

		/**
		 * @brief デバッグ用にコライダーを可視化する
		 * @param center 中心座標
		 * @param size サイズ
		 * @param color 色（ARGB）
		 */
		void drawCollider(const core::Vector3& center, const core::Vector3& size, unsigned int color) override;

		/**
		 * @brief デバッグ用に球（範囲）を可視化する
		 * @param center 中心座標
		 * @param radius 半径
		 * @param color 色（ARGB）
		 */
		void drawDebugSphere(const core::Vector3& center, float radius, unsigned int color) override;

		/**
		 * @brief デバッグ用にカプセル（範囲）を可視化する
		 * @param bottom カプセル軸の下端座標
		 * @param top カプセル軸の上端座標
		 * @param radius 半径
		 * @param color 色（ARGB）
		 */
		void drawDebugCapsule(const core::Vector3& bottom, const core::Vector3& top, float radius, unsigned int color) override;

		/**
		 * @brief 常にカメラの方を向く板（ビルボード）として画像を描画する
		 * @param imageHandle 画像ハンドル（loadImageByIdで取得）
		 * @param position ワールド座標（板の中心）
		 * @param size 描画サイズ（ワールド単位・一辺の長さ）
		 */
		void drawBillboard(int imageHandle, const core::Vector3& position, float size) override;

		/**
		 * @brief ワールド座標をスクリーン座標へ変換する
		 * @param worldPos ワールド座標
		 * @return x/yはスクリーン座標、zは深度（0.0〜1.0の範囲内なら画面に映っている）
		 */
		core::Vector3 worldToScreen(const core::Vector3& worldPos) override;

	  private:
		// applyDeathDissolveで初回に保存する、マテリアルの元のディフューズ色とエミッシブ色
		struct MaterialColors
		{
			std::array<float, 4> m_dif{};
			std::array<float, 4> m_emi{};
		};

		// modelHandle -> マテリアルごとの元の色。プールで使い回すハンドルの復元に使う
		std::unordered_map<int, std::vector<MaterialColors>> m_originalColors;
	};
} // namespace infrastructure