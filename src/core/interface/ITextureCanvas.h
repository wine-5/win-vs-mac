#pragma once

namespace core::iface
{
	/**
	 * @brief 描画先を「画面」から「テクスチャ」へ切り替える仕組み
	 *
	 * beginDraw〜endDraw の間の描画は画面ではなくテクスチャへ入る。
	 * 描画そのものは既存の IUIRenderer をそのまま使えるので、
	 * 文字や図形の描画APIをここで二重に定義しない。
	 *
	 * 用途：壁にシステム情報（CPU使用率など）を流す「サーバー内部」の演出。
	 * @note beginDraw の内側でさらに beginDraw することはできない（描画先は1つ）。
	 */
	class ITextureCanvas
	{
	  public:
		virtual ~ITextureCanvas() = default;

		/**
		 * @brief 描画先にできるテクスチャを作る
		 * @param width 幅（ピクセル）
		 * @param height 高さ（ピクセル）
		 * @return キャンバスハンドル（失敗時は-1）
		 */
		[[nodiscard]] virtual int create(int width, int height) = 0;

		/**
		 * @brief 描画先をキャンバスへ切り替え、内容を消去する
		 * @param canvasHandle キャンバスハンドル
		 * @param r 消去色の赤成分（0-255）
		 * @param g 消去色の緑成分（0-255）
		 * @param b 消去色の青成分（0-255）
		 */
		virtual void beginDraw(int canvasHandle, int r, int g, int b) = 0;

		/**
		 * @brief 描画先を画面へ戻す
		 */
		virtual void endDraw() = 0;

		/**
		 * @brief キャンバスをモデルのテクスチャとして貼る
		 * @param modelHandle 対象のモデルハンドル
		 * @param canvasHandle キャンバスハンドル
		 */
		virtual void applyToModel(int modelHandle, int canvasHandle) = 0;

		/**
		 * @brief キャンバスを破棄する
		 * @param canvasHandle キャンバスハンドル
		 */
		virtual void destroy(int canvasHandle) = 0;
	};
} // namespace core::iface
