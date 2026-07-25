#pragma once
#include <string>
#include <string_view>
#include <vector>
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
		 * @brief モデルのフレーム（ボーン）を名前から検索する
		 *
		 * 武器をキャラクターの手へ装着する際、装着先のボーン番号を得るのに使う。
		 * ボーン名はモデルの作り（リグ）によって異なるため、見つからない場合は
		 * getModelFrameNames() で実際の候補を確認すること
		 * @param modelHandle モデルハンドル
		 * @param frameName フレーム（ボーン）名
		 * @return フレーム番号。見つからない場合は -1
		 */
		[[nodiscard]] virtual int findModelFrame(int modelHandle, std::string_view frameName) = 0;

		/**
		 * @brief モデルが持つ全フレーム（ボーン）の名前を取得する
		 *
		 * 装着先のボーン名が分からないときに候補を列挙するために使う
		 * @param modelHandle モデルハンドル
		 * @return フレーム名の一覧（フレーム番号順）。失敗時は空
		 */
		[[nodiscard]] virtual std::vector<std::string> getModelFrameNames(int modelHandle) = 0;

		/**
		 * @brief モデルを他モデルのフレーム（ボーン）へ追従させて描画する
		 *
		 * 親のアニメーションが適用された後のボーン位置へ武器を貼り付ける。
		 * オフセットは装着先ボーンのローカル空間で解釈されるため、握りの位置・
		 * 角度の微調整に使える。
		 *
		 * @note 親のボーン行列にはアニメーションと親のスケールが反映済みである必要が
		 *       あるため、必ず親モデルを描画した後に呼ぶこと
		 * @note modelHandle には武器専用のハンドルを渡すこと。行列を直接指定して描く
		 *       ため、以後このハンドルには位置・回転・スケール指定が効かなくなる
		 * @param modelHandle 装着するモデル（武器）のハンドル
		 * @param parentModelHandle 装着先モデル（キャラクター）のハンドル
		 * @param frameIndex 装着先のフレーム番号（findModelFrameで取得したもの）
		 * @param offsetPosition ボーンのローカル空間での位置オフセット
		 * @param offsetRotation ボーンのローカル空間での回転オフセット（ラジアン）
		 * @param offsetScale 武器自体のスケール（親のスケールに乗算される）
		 */
		virtual void drawModelOnFrame(int modelHandle, int parentModelHandle, int frameIndex,
		    const core::Vector3& offsetPosition, const core::Vector3& offsetRotation,
		    const core::Vector3& offsetScale) = 0;

		/**
		 * @brief モデルのテクスチャ繰り返し回数を設定する
		 *
		 * 立方体を引き伸ばして作る配置物は、そのままだと模様が間延びする。
		 * 実寸に応じて繰り返し回数を上げることで、長い壁に窓が連続して並ぶようになる。
		 * 描画のたびに設定するため、同じモデルを別サイズで使い回しても混ざらない。
		 * @param modelHandle モデルハンドル
		 * @param scaleU 横方向の繰り返し回数（1.0で引き伸ばし）
		 * @param scaleV 縦方向の繰り返し回数（1.0で引き伸ばし）
		 */
		virtual void setTextureTiling(int modelHandle, float scaleU, float scaleV) = 0;

		/**
		 * @brief モデルのテクスチャをずらして貼る（模様を流す演出に使う）
		 *
		 * 繰り返し回数に加えて平行移動を指定する。offsetV を時間で増やし続ければ、
		 * 壁の模様が流れ続けて「情報が流れるサーバー内部」に見える。
		 * 描画のたびに設定するため、同じモデルを別の流し方で使い回しても混ざらない。
		 * @param modelHandle モデルハンドル
		 * @param scaleU 横方向の繰り返し回数（1.0で引き伸ばし）
		 * @param scaleV 縦方向の繰り返し回数（1.0で引き伸ばし）
		 * @param offsetU 横方向のずらし量（1.0でテクスチャ1枚ぶん）
		 * @param offsetV 縦方向のずらし量（1.0でテクスチャ1枚ぶん）
		 */
		virtual void setTextureScroll(int modelHandle, float scaleU, float scaleV,
		    float offsetU, float offsetV) = 0;
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
		 * @param rotationY Y軸まわりの向き（ラジアン）
		 * @param color 色（ARGB）
		 */
		virtual void drawCollider(const core::Vector3& center, const core::Vector3& size, float rotationY, unsigned int color) = 0;

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

		/**
		 * @brief 2D画像を常にカメラへ正対するビルボードとして3D空間に描く
		 *
		 * 深度（Zバッファ）を持つので、壁や柱の裏に回れば正しく隠れる。
		 * プレイヤーのWindow弾のように「板に絵を貼った弾」を描くのに使う。
		 * @param imageHandle 2D画像ハンドル（loadImageByIdで取得したもの）
		 * @param position ビルボード中心のワールド座標
		 * @param size ワールド単位での大きさ（画像のアスペクト比は保たれる）
		 * @param angle 面内の回転角（ラジアン）
		 */
		virtual void drawBillboard(int imageHandle, const core::Vector3& position,
		    float size, float angle) = 0;

		// 補足: worldToScreen は射影変換であり、厳密には3D描画の責務ではない。
		//       ただし現状の利用は順変換の2箇所のみで、メソッド1本のために
		//       IViewProjection を新設しても抽象が増えるだけで得るものが少ない。
		//       screenToWorld（クリック→ワールド）等の逆変換が必要になった時点で分離する。
		/**
		 * @brief ワールド座標をスクリーン座標へ変換する
		 * @param worldPos ワールド座標
		 * @return x/yはスクリーン座標、zは深度（0.0〜1.0の範囲内なら画面に映っている）
		 */
		virtual core::Vector3 worldToScreen(const core::Vector3& worldPos) = 0;

		/**
		 * @brief DEBUG: 直前の1フレームで発行された描画コール数を取得する
		 *
		 * 描画負荷の当たりをつけるための計測用。値は「前々回の画面更新〜前回の画面更新」の
		 * 区間の集計であり、フレーム中のどこで呼んでも直前フレームの確定値が返る。
		 * モデル・UI・エフェクトを含めた総数を数える
		 * @return 描画コール数
		 */
		virtual int getDrawCallCount() = 0;
	};
} // namespace core::iface