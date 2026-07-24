#pragma once
#include "core/interface/ITextureCanvas.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLibの描画可能スクリーンを使って、テクスチャへ描き込むクラス
	 */
	class TextureCanvas : public core::iface::ITextureCanvas
	{
	  public:
		/**
		 * @brief 描画先にできるテクスチャを作る
		 * @param width 幅（ピクセル）
		 * @param height 高さ（ピクセル）
		 * @return キャンバスハンドル（失敗時は-1）
		 */
		[[nodiscard]] int create(int width, int height) override;

		/**
		 * @brief 描画先をキャンバスへ切り替え、内容を消去する
		 * @param canvasHandle キャンバスハンドル
		 * @param r 消去色の赤成分（0-255）
		 * @param g 消去色の緑成分（0-255）
		 * @param b 消去色の青成分（0-255）
		 */
		void beginDraw(int canvasHandle, int r, int g, int b) override;

		/** @brief 描画先を画面（裏画面）へ戻す */
		void endDraw() override;

		/**
		 * @brief キャンバスをモデルのテクスチャとして貼る
		 * @param modelHandle 対象のモデルハンドル
		 * @param canvasHandle キャンバスハンドル
		 */
		void applyToModel(int modelHandle, int canvasHandle) override;

		/**
		 * @brief キャンバスを破棄する
		 * @param canvasHandle キャンバスハンドル
		 */
		void destroy(int canvasHandle) override;
	};
} // namespace infrastructure::graphics
