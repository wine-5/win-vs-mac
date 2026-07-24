#include "DataWallSystem.h"
#include "core/interface/ITextureCanvas.h"
#include "core/interface/IUIRenderer.h"
#include "core/utility/Color.h"
#include <array>

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

	/**
	 * @brief 壁に流す文字列
	 *
	 * ターゲットがプログラマーなので、命令・ログ・16進ダンプらしい行を混ぜて
	 * 「システムの内側を覗いている」感じを出す。侵入者としてApple側の名前も混ぜる。
	 */
	constexpr std::array<const char*, 16> DATA_LINES{
		"[ok] kernel32.dll mapped 0x7ffb2c40",
		"mov rax, qword ptr [rsp+28h]",
		"[warn] handle leak pid=4812",
		"thread 0x1a4 state=WAITING",
		"[ok] ntfs journal flush 12.4MB/s",
		"if (hr != S_OK) return hr;",
		"[intruder] apple.process detected",
		"call QueryPerformanceCounter",
		"[ok] page fault soft=1284 hard=3",
		"lock cmpxchg [rbx], rcx",
		"[warn] gdi objects 9821/10000",
		"0x4d 0x5a 0x90 0x00 0x03",
		"[ok] scheduler quantum=15ms",
		"while (!queue.empty()) pop();",
		"[intruder] safari.exe spawned",
		"ret",
	};

	/** @brief 行の種別で色を決める（侵入者=赤、警告=黄、正常=青、コード=淡い水色） */
	unsigned int lineColor(const char* text) noexcept
	{
		if (text[1] == 'i') // "[intruder]"
			return core::utility::Color::rgb(232, 17, 35);
		if (text[1] == 'w') // "[warn]"
			return core::utility::Color::rgb(255, 200, 61);
		if (text[1] == 'o') // "[ok]"
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

	void DataWallSystem::buildStreams()
	{
		// 縦の列：下から上へ流す。列ごとに速さと開始位置をずらして単調さを消す
		for (int i{ 0 }; i < VERTICAL_COUNT; ++i)
		{
			const char* text{ DATA_LINES[static_cast<size_t>(i * 3) % DATA_LINES.size()] };
			Stream stream{};
			stream.m_text = text;
			stream.m_x = static_cast<float>(20 + i * (CANVAS_SIZE / VERTICAL_COUNT));
			stream.m_y = static_cast<float>(CANVAS_SIZE + i * 90);
			stream.m_speed = 55.0f + static_cast<float>(i) * 13.0f;
			stream.m_isVertical = true;
			stream.m_color = lineColor(text);
			m_streams.push_back(stream);
		}

		// 横のティッカー：右から左へ流す
		for (int i{ 0 }; i < HORIZONTAL_COUNT; ++i)
		{
			const char* text{ DATA_LINES[static_cast<size_t>(i * 5 + 1) % DATA_LINES.size()] };
			Stream stream{};
			stream.m_text = text;
			stream.m_x = static_cast<float>(CANVAS_SIZE + i * 160);
			stream.m_y = static_cast<float>(70 + i * 150);
			stream.m_speed = 90.0f + static_cast<float>(i) * 25.0f;
			stream.m_isVertical = false;
			stream.m_color = lineColor(text);
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
				// 上へ抜けたら下から出し直す
				if (stream.m_y < -FONT_SIZE)
					stream.m_y = static_cast<float>(CANVAS_SIZE);
				continue;
			}

			stream.m_x -= stream.m_speed * deltaTime;
			// 左へ抜けたら右から出し直す（文字幅ぶん余裕をみる）
			if (stream.m_x < -static_cast<float>(m_uiRenderer.getTextWidth(stream.m_text.c_str(), FONT_SIZE)))
				stream.m_x = static_cast<float>(CANVAS_SIZE);
		}

		redraw();
	}
} // namespace game::system::visual
