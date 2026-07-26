#include "ResourcePreloader.h"
#include <chrono>
#include "core/interface/IResourceManager.h"

namespace infrastructure::resource
{
	using core::iface::PreloadKind;

	ResourcePreloader::ResourcePreloader(core::iface::IResourceManager& resourceManager) noexcept
	    : m_resourceManager{ resourceManager }
	{
	}

	void ResourcePreloader::enqueue(PreloadKind kind, std::string_view id)
	{
		m_queue.push_back(Request{ kind, std::string{ id } });
		++m_totalCount;
	}

	void ResourcePreloader::enqueueAll()
	{
		// 画像は1件あたりが軽いので先に積み、重いモデル・アニメーションを後ろに回す。
		// こうすると序盤のフレームが詰まりにくい
		for (const auto& id : m_resourceManager.getAllImageIds())
			enqueue(PreloadKind::Image, id);
		for (const auto& id : m_resourceManager.getAllModelIds())
			enqueue(PreloadKind::Model, id);
		for (const auto& id : m_resourceManager.getAllAnimationIds())
			enqueue(PreloadKind::Animation, id);
	}

	int ResourcePreloader::step(int budgetMilliseconds)
	{
		const auto start{ std::chrono::steady_clock::now() };
		int processed{ 0 };

		while (!m_queue.empty())
		{
			const Request request{ m_queue.front() };
			m_queue.pop_front();

			switch (request.m_kind)
			{
			case PreloadKind::Model:
				m_resourceManager.loadModelById(request.m_id);
				break;
			case PreloadKind::Animation:
				m_resourceManager.loadAnimationById(request.m_id);
				break;
			case PreloadKind::Image:
				m_resourceManager.loadImageById(request.m_id);
				break;
			}

			++m_doneCount;
			++processed;

			// 予算判定はループ末尾で行う（＝1回のstepで最低1件は必ず処理する）。
			// 先頭で判定すると、単体で予算を超えるリソースが永久に消化されない
			const auto elapsed{ std::chrono::steady_clock::now() - start };
			if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() >= budgetMilliseconds)
				break;
		}

		return processed;
	}

	bool ResourcePreloader::isComplete() const noexcept
	{
		return m_queue.empty();
	}

	float ResourcePreloader::getProgress() const noexcept
	{
		if (m_totalCount == 0)
			return 1.0f;

		return static_cast<float>(m_doneCount) / static_cast<float>(m_totalCount);
	}
} // namespace infrastructure::resource
