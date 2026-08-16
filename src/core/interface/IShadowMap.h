#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 平行光源のシャドウマップを操作するインターフェース
	 *
	 * Game層がDxLibへ直接触らずに「誰が影を落とし、どこへ影が落ちるか」を制御するための抽象化。
	 *
	 * 使い方は2パス構成になる。まず影を落とす物だけを beginCasterPass()〜endCasterPass() の
	 * 間に描いて深度を焼き、続けて影を受ける物を beginReceivePass()〜endReceivePass() の
	 * 間に描く。落とす側と受ける側の両方に描けば、自分自身にも影が落ちる。
	 *
	 * @note 影を受ける側のメッシュで背面カリングを無効（DX_CULLING_NONE）にすると、
	 *       DxLibは影を一切適用しない。受け側のモデルではカリングを切らないこと。
	 */
	class IShadowMap
	{
	  public:
		virtual ~IShadowMap() = default;

		/**
		 * @brief シャドウマップを作成する
		 * @param resolution 一辺の解像度（2の冪であること）
		 * @return 作成できたらtrue
		 */
		[[nodiscard]] virtual bool create(int resolution) = 0;

		/**
		 * @brief シャドウマップを破棄する
		 *
		 * 作成していなければ何もしない。
		 */
		virtual void destroy() noexcept = 0;

		/**
		 * @brief 使用可能な状態か
		 * @return 作成済みならtrue
		 */
		[[nodiscard]] virtual bool isValid() const noexcept = 0;

		/**
		 * @brief 影を落とす平行光の向きを設定する
		 *
		 * ライティングの平行光と同じ向きを渡さないと、影の伸びる向きと
		 * 面の明暗が食い違って見える。
		 * @param direction 光の進む向き（正規化していなくてよい）
		 */
		virtual void setLightDirection(const core::Vector3& direction) noexcept = 0;

		/**
		 * @brief シャドウマップが受け持つワールドの範囲を設定する
		 *
		 * 範囲を広げるほど1ピクセルあたりの実寸が粗くなり、影の輪郭がぼやける。
		 * ステージ全体ではなくプレイヤーの周囲だけを毎フレーム指定して追従させる。
		 * @param center 範囲の中心（通常はプレイヤーの足元）
		 * @param halfSize 中心からの水平方向の広がり
		 * @param halfHeight 中心からの垂直方向の広がり
		 */
		virtual void setDrawArea(const core::Vector3& center, float halfSize,
		    float halfHeight) noexcept = 0;

		/**
		 * @brief 影の判定に加える補正深度を設定する
		 *
		 * 0に近いほど接地部分の影が密着するが、小さすぎると自分自身の面が
		 * 自分の影に入り、縞状のノイズ（シャドウアクネ）が出る。
		 * @param depth 補正深度
		 */
		virtual void setAdjustDepth(float depth) noexcept = 0;

		/**
		 * @brief 影を落とす物の描画を開始する
		 *
		 * これ以降 endCasterPass() までの描画は画面には出ず、深度だけが記録される。
		 */
		virtual void beginCasterPass() noexcept = 0;

		/** @brief 影を落とす物の描画を終了する */
		virtual void endCasterPass() noexcept = 0;

		/**
		 * @brief 影を受ける物の描画を開始する
		 *
		 * これ以降 endReceivePass() までに描いた物へ、記録済みの影が落ちる。
		 */
		virtual void beginReceivePass() noexcept = 0;

		/** @brief 影を受ける物の描画を終了する */
		virtual void endReceivePass() noexcept = 0;
	};
} // namespace core::iface
