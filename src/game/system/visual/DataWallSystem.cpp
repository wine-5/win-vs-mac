#include "DataWallSystem.h"
#include "core/interface/ITextureCanvas.h"
#include "core/interface/IUIRenderer.h"
#include "core/utility/Color.h"
#include "game/constant/DataWallLines.h"
#include <array>
#include <random>
#include <string_view>

namespace
{
	// 壁テクスチャの解像度
	constexpr int CANVAS_SIZE{ 512 };

	// 下地（虚無より明るく、面として認識できる濃紺）
	constexpr int BG_R{ 14 };
	constexpr int BG_G{ 26 };
	constexpr int BG_B{ 40 };

	constexpr int FONT_SIZE{ 16 };
	constexpr int GRID_STEP{ 64 };

	// 縦に流す列の本数と、横に流すティッカーの本数
	constexpr int VERTICAL_COUNT{ 5 };
	constexpr int HORIZONTAL_COUNT{ 3 };

	constexpr unsigned int GRID_COLOR{ core::utility::Color::argb(40, 0, 164, 239) };

	/** @brief 行の種別で色を決める（侵入者=赤、警告=黄、正常=青、コード=淡い水色） */
	unsigned int lineColor(std::string_view text) noexcept
	{
		if (text.starts_with("[intruder]"))
			return core::utility::Color::rgb(232, 17, 35);
		if (text.starts_with("[warn]"))
			return core::utility::Color::rgb(255, 200, 61);
		if (text.starts_with("[ok]"))
			return core::utility::Color::rgb(0, 164, 239);
		return core::utility::Color::rgb(120, 200, 245);
	}
} // namespace

namespace game::system::visual
{
	DataWallSystem::DataWallSystem(core::iface::ITextureCanvas& canvas,
	    core::iface::IUIRenderer& uiRenderer,
	    int wallModelHandle)
	    : m_canvas{ canvas }
	    , m_uiRenderer{ uiRenderer }
	    , m_wallModelHandle{ wallModelHandle }
	{
		if (m_wallModelHandle == -1)
			return;

		m_canvasHandle = m_canvas.create(CANVAS_SIZE, CANVAS_SIZE);
		if (m_canvasHandle == -1)
			return;

		m_canvas.applyToModel(m_wallModelHandle, m_canvasHandle);
		buildStreams();
	}

	DataWallSystem::~DataWallSystem()
	{
		m_canvas.destroy(m_canvasHandle);
	}

	std::string_view DataWallSystem::pickLine()
	{
		std::uniform_int_distribution<size_t> pick{ 0, constant::data_wall::LINES.size() - 1 };
		return constant::data_wall::LINES[pick(m_random)];
	}

	void DataWallSystem::buildStreams()
	{
		std::uniform_real_distribution<float> speed{ 45.0f, 110.0f };
		std::uniform_real_distribution<float> offset{ 0.0f, static_cast<float>(CANVAS_SIZE) };

		// 縦の列：下から上へ流す。速さと開始位置をばらけさせて単調さを消す
		for (int i{ 0 }; i < VERTICAL_COUNT; ++i)
		{
			Stream stream{};
			stream.m_text = pickLine();
			stream.m_x = static_cast<float>(20 + i * (CANVAS_SIZE / VERTICAL_COUNT));
			stream.m_y = static_cast<float>(CANVAS_SIZE) + offset(m_random);
			stream.m_speed = speed(m_random);
			stream.m_isVertical = true;
			stream.m_color = lineColor(stream.m_text);
			m_streams.push_back(stream);
		}

		// 横のティッカー：右から左へ流す
		for (int i{ 0 }; i < HORIZONTAL_COUNT; ++i)
		{
			Stream stream{};
			stream.m_text = pickLine();
			stream.m_x = static_cast<float>(CANVAS_SIZE) + offset(m_random);
			stream.m_y = static_cast<float>(70 + i * 150);
			stream.m_speed = speed(m_random);
			stream.m_isVertical = false;
			stream.m_color = lineColor(stream.m_text);
			m_streams.push_back(stream);
		}
	}

	void DataWallSystem::redraw()
	{
		m_canvas.beginDraw(m_canvasHandle, BG_R, BG_G, BG_B);

		// 下地のグリッド（面として認識させるため。文字が無い所も虚無に溶けない）
		for (int p{ GRID_STEP }; p < CANVAS_SIZE; p += GRID_STEP)
		{
			m_uiRenderer.drawBox(p, 0, 1, CANVAS_SIZE, GRID_COLOR, true);
			m_uiRenderer.drawBox(0, p, CANVAS_SIZE, 1, GRID_COLOR, true);
		}

		for (const auto& stream : m_streams)
			m_uiRenderer.drawText(static_cast<int>(stream.m_x), static_cast<int>(stream.m_y),
			    stream.m_text.c_str(), stream.m_color, FONT_SIZE);

		// 左右の縁を光らせる（壁の輪郭を示す）
		constexpr unsigned int EDGE_COLOR{ core::utility::Color::rgb(0, 164, 239) };
		constexpr int EDGE_WIDTH{ 6 };
		m_uiRenderer.drawBox(0, 0, EDGE_WIDTH, CANVAS_SIZE, EDGE_COLOR, true);
		m_uiRenderer.drawBox(CANVAS_SIZE - EDGE_WIDTH, 0, EDGE_WIDTH, CANVAS_SIZE, EDGE_COLOR, true);

		m_canvas.endDraw();
	}

	void DataWallSystem::update(float deltaTime)
	{
		if (m_canvasHandle == -1)
			return;

		for (auto& stream : m_streams)
		{
			if (stream.m_isVertical)
			{
				stream.m_y -= stream.m_speed * deltaTime;
				// 上へ抜けたら下から出し直す。そのとき別の行を引いて内容を入れ替える
				if (stream.m_y < -FONT_SIZE)
				{
					stream.m_y = static_cast<float>(CANVAS_SIZE);
					stream.m_text = pickLine();
					stream.m_color = lineColor(stream.m_text);
				}
				continue;
			}

			stream.m_x -= stream.m_speed * deltaTime;
			// 左へ抜けたら右から出し直す（文字幅ぶん余裕をみる）
			if (stream.m_x < -static_cast<float>(m_uiRenderer.getTextWidth(stream.m_text.c_str(), FONT_SIZE)))
			{
				stream.m_x = static_cast<float>(CANVAS_SIZE);
				stream.m_text = pickLine();
				stream.m_color = lineColor(stream.m_text);
			}
		}

		redraw();
	}
} // namespace game::system::visual
