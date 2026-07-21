#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 描画の純粋仮想クラス
	 * Game層がInfrastructre層に直接依存しないための抽象化
	 */
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		
		/**
		 * @brief 3Dモデルを描画する
		 * @param modelHandle モデルハンドル
		 * @param position 位置
		 * @param rotation 回転（ラジアン）
		 * @param scale スケール
		 */
		virtual void drawModel(int modelHandle, const core::Vector3& position, const::core::Vector3& rotation, const core::Vector3& scale) = 0;
		/**
		 * @brief 敵撃破時の赤化＋ディゾルブ（消失）演出をモデルに適用する
		 *
		 * 初回呼び出し時にモデルの元の色を内部に保存し、以後はそこから赤へブレンドする。
		 * 赤化と消失フェードは独立に制御できる（落下バウンド中は赤くしつつ不透明を保つ等）。
		 * モデルハンドルをプールへ返却する前に resetModelAppearance を呼んで元に戻すこと
		 * @param modelHandle 対象のモデルハンドル
		 * @param redProgress 赤化の進行度（0.0=元の色 〜 1.0=赤）
		 * @param alpha 不透明度（1.0=不透明 〜 0.0=完全に消失）
		 */
		virtual void applyDeathDissolve(int modelHandle, float redProgress, float alpha) = 0;

		/**
		 * @brief applyDeathDissolveで変更した見た目を元に戻す
		 *
		 * モデルハンドルをプールへ返却し使い回す前に必ず呼ぶこと。
		 * 呼ばないと次にこのハンドルを使う敵が赤く透けた状態のまま出現してしまう
		 * @param modelHandle 対象のモデルハンドル
		 */
		virtual void resetModelAppearance(int modelHandle) = 0;

		/**
		 * @brief デバッグ用にコライダーを可視化する
		 * @param center 中心座標
		 * @param size サイズ
		 * @param color 色（ARGB）
		 */
		virtual void drawCollider(const core::Vector3& center, const core::Vector3& size, unsigned int color) = 0;

		/**
		 * @brief デバッグ用に球（範囲）を可視化する
		 * @param center 中心座標
		 * @param radius 半径
		 * @param color 色（ARGB）
		 */
		virtual void drawDebugSphere(const core::Vector3& center, float radius, unsigned int color) = 0;

		/**
		 * @brief デバッグ用にカプセル（範囲）を可視化する
		 * @param bottom カプセル軸の下端座標
		 * @param top カプセル軸の上端座標
		 * @param radius 半径
		 * @param color 色（ARGB）
		 */
		virtual void drawDebugCapsule(const core::Vector3& bottom, const core::Vector3& top, float radius, unsigned int color) = 0;

		/**
		 * @brief 常にカメラの方を向く板（ビルボード）として画像を描画する
		 * @param imageHandle 画像ハンドル（loadImageByIdで取得）
		 * @param position ワールド座標（板の中心）
		 * @param size 描画サイズ（ワールド単位・一辺の長さ）
		 */
		virtual void drawBillboard(int imageHandle, const core::Vector3& position, float size) = 0;

		/**
		 * @brief ワールド座標をスクリーン座標へ変換する
		 * @param worldPos ワールド座標
		 * @return x/yはスクリーン座標、zは深度（0.0〜1.0の範囲内なら画面に映っている）
		 */
		virtual core::Vector3 worldToScreen(const core::Vector3& worldPos) = 0;
	};
} // namespace core::iface