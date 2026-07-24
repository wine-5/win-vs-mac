#pragma once
#include "core/ecs/ISystem.h"
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace core::iface
{
	class ITextureCanvas;
	class IUIRenderer;
} // namespace core::iface

namespace game::system::visual
{
	/**
	 * @brief 「データ壁」のテクスチャに、流れる文字を毎フレーム描くSystem
	 *
	 * 「情報が壁一面に流れるサーバー内部」を作るための演出。
	 * テクスチャ自体をUVでずらす方法だと全体が一方向にしか動かせないため、
	 * 文字を1本ずつ動かせるよう、テクスチャへ直接描く方式にしている。
	 *
	 * @note テクスチャはモデル単位で差し替わるため、データ壁を何枚置いても同じ内容が映る。
	 */
	class DataWallSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief DataWallSystemのコンストラクタ
		 * @param canvas テクスチャへの描画を担うインターフェース
		 * @param uiRenderer 文字・図形の描画に使うインターフェース
		 * @param wallModelHandle データ壁のモデルハンドル（-1なら何もしない）
		 */
		DataWallSystem(core::iface::ITextureCanvas& canvas,
		    core::iface::IUIRenderer& uiRenderer,
		    int wallModelHandle);

		/** @brief 生成したキャンバスを破棄する */
		~DataWallSystem() override;

		/**
		 * @brief 文字を進めて壁を描き直す
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/** @brief 流れる文字1本ぶんの状態 */
		struct Stream
		{
			std::string m_text{};
			float m_x{}; // 現在位置（テクスチャ内のピクセル座標）
			float m_y{};
			float m_speed{};     // 進む速さ（ピクセル/秒）
			bool m_isVertical{}; // true=下から上へ、false=右から左へ
			unsigned int m_color{};
		};

		/**
		 * @brief 流す文字を1本ランダムに選ぶ
		 * @return 選ばれた行
		 */
		[[nodiscard]] std::string_view pickLine();

		/** @brief 流す文字の初期配置を組み立てる */
		void buildStreams();

		/** @brief キャンバスへ背景と文字を描く */
		void redraw();

		core::iface::ITextureCanvas& m_canvas;
		core::iface::IUIRenderer& m_uiRenderer;

		int m_wallModelHandle{ -1 };
		int m_canvasHandle{ -1 };

		std::vector<Stream> m_streams;

		// 壁ごとに違う内容になるよう、行の抽選に使う乱数
		std::mt19937 m_random{ std::random_device{}() };
	};
} // namespace game::system::visual
