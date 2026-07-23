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
		 * @brief 地面（XZ平面）に円を描く（攻撃範囲の予兆表示などに使う）
		 *
		 * center.y の高さの水平面に半径 radius の円を描く。
		 * color の上位8bit（アルファ）を見て半透明合成する。Zバッファへは書き込まない。
		 * @param center 円の中心（ワールド座標）
		 * @param radius 半径（ワールド単位）
		 * @param color 色（ARGB形式：0xAARRGGBB。アルファで半透明度を指定）
		 * @param filled true=塗りつぶし円、false=輪郭のみ
		 */
		virtual void drawGroundCircle(const core::Vector3& center, float radius, unsigned int color, bool filled) = 0;

		/**
		 * @brief 地面（XZ平面）に扇形（セクター）を描く（扇状攻撃の予兆表示などに使う）
		 *
		 * center を要として facingRad 方向を中心に、左右 halfAngleRad ずつ開いた扇を描く。
		 * @param center 扇の要（ワールド座標）
		 * @param facingRad 扇の中心方向（ラジアン。XZ平面で+X軸からの角度、atan2(dz,dx)）
		 * @param radius 半径（ワールド単位）
		 * @param halfAngleRad 中心方向からの片側の開き角（ラジアン。全開き角の半分）
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param filled true=塗りつぶし、false=輪郭のみ
		 */
		virtual void drawGroundSector(const core::Vector3& center, float facingRad, float radius,
		    float halfAngleRad, unsigned int color, bool filled) = 0;

		/**
		 * @brief 常にカメラの方を向く板（ビルボード）として画像を描画する
		 * @param imageHandle 画像ハンドル（loadImageByIdで取得）
		 * @param position ワールド座標（板の中心）
		 * @param size 描画サイズ（ワールド単位・一辺の長さ）
		 */
		virtual void drawBillboard(int imageHandle, const core::Vector3& position, float size) = 0;

		/**
		 * @brief 3Dモデルの正面(ローカル+Z)を指定方向へ向け、その面内で回転させて描画する
		 *
		 * faceDir（進行方向など）へモデルの正面を向け、faceDir軸まわりに spinAngle だけ回す。
		 * カメラには追従せず、与えた向きのまま描く（レインボー弾のように投げた方向を保つ回転体用）。
		 * centerOffset はモデル原点と見た目中心のズレ（AABB中心・スケール未適用）で、
		 * これを打ち消して見た目中心を position に合わせる（原点まわりの円運動を防ぐ）。
		 * @param modelHandle モデルハンドル
		 * @param position 見た目中心を合わせるワールド座標
		 * @param scale モデルスケール
		 * @param centerOffset モデルのAABB中心（ローカル・スケール未適用）
		 * @param faceDir モデルの正面を向ける方向（正規化不要。ゼロなら描画しない）
		 * @param spinAngle 面内回転角（ラジアン）
		 */
		virtual void drawSpinningModelFacing(int modelHandle, const core::Vector3& position,
		    const core::Vector3& scale, const core::Vector3& centerOffset,
		    const core::Vector3& faceDir, float spinAngle) = 0;

		// TODO: worldToScreen はワールド→スクリーンの射影変換であり、厳密には3D描画の責務ではない。
		//       ICameraは「計算しない薄い層」の方針のため置けない。将来のリファクタリングで
		//       screenToWorld 等とあわせて専用の IViewProjection インターフェースへ分離する。
		/**
		 * @brief ワールド座標をスクリーン座標へ変換する
		 * @param worldPos ワールド座標
		 * @return x/yはスクリーン座標、zは深度（0.0〜1.0の範囲内なら画面に映っている）
		 */
		virtual core::Vector3 worldToScreen(const core::Vector3& worldPos) = 0;
	};
} // namespace core::iface