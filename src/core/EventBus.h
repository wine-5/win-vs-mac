#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>

class EventBus
{
public:
	/**
	 * @brief イベントを購読する
	 */
	template<typename TEvent>
	void subscribe(std::function<void(const TEvent&)> callback)
	{
		auto key = std::type_index(typeid(TEvent));
		m_listeners[key].push_back(
			[callback](const std::any& e)
			{
				callback(std::any_cast<TEvent>(e));
			}
		);
	}

	/**
	 * @brief イベントを発行する
	 */
	template<typename TEvent>
	void publish(const TEvent& event)
	{
		auto key = std::type_index(typeid(TEvent));
		auto it = m_listeners.find(key);
		if (it == m_listeners.end()) return;

		for (auto& listener : it->second)
		{
			listener(event);
		}
	}

private:
	// イベントの型→コールバックのリスト
	std::unordered_map<std::type_index, std::vector<std::function<void(const std::any&)>>> m_listeners;
};