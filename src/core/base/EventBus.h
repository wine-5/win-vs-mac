#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <utility>
#include "core/base/NonCopyable.h"

namespace core::base
{
	/**
	 * @brief イベントの発行と購読を管理するクラス
	 *
	 * 購読すると Subscription（RAIIハンドル）が返る。購読側はこれをメンバとして保持し、
	 * 自身が破棄されるときに自動で解除される。購読者がEventBusより先に死んでも
	 * ダングリングしないことを、寿命の偶然ではなく型で保証する。
	 *
	 * 注意：コールバックの中から subscribe / unsubscribe を呼んではいけない。
	 * publish中の解除自体は安全に扱えるが、購読の追加は次回のpublishからの反映になる。
	 *
	 * Subscription が this を握るため、コピー・ムーブされると解除先がずれて壊れる。
	 * NonCopyable を継承して型レベルで禁止する。
	 */
	class EventBus : private NonCopyable
	{
	public:
	  /**
	   * @brief 購読を表すRAIIハンドル
	   *
	   * 破棄・ムーブ代入・reset()のいずれでも購読が解除される。
	   * コピーはできない（購読の所有者を1つに保つため）。
	   */
	  class Subscription
	  {
		public:
		  /** @brief 何も購読していないハンドルを作る */
		  Subscription() = default;

		  /**
		   * @brief 購読中のハンドルを作る（EventBusのみが生成する）
		   * @param bus 購読先のEventBus
		   * @param type イベント型のtype_index
		   * @param id 購読ID
		   */
		  Subscription(EventBus* bus, std::type_index type, std::uint64_t id) noexcept
			  : m_bus{ bus }
			  , m_type{ type }
			  , m_id{ id }
		  {
		  }

		  ~Subscription()
		  {
			  reset();
		  }

		  Subscription(const Subscription&) = delete;
		  Subscription& operator=(const Subscription&) = delete;

		  /** @brief ムーブコンストラクタ */
		  Subscription(Subscription&& other) noexcept
			  : m_bus{ other.m_bus }
			  , m_type{ other.m_type }
			  , m_id{ other.m_id }
		  {
			  other.m_bus = nullptr;
		  }

		  /** @brief ムーブ代入（自身が持っていた購読は先に解除する） */
		  Subscription& operator=(Subscription&& other) noexcept
		  {
			  if (this != &other)
			  {
				  reset();
				  m_bus = other.m_bus;
				  m_type = other.m_type;
				  m_id = other.m_id;
				  other.m_bus = nullptr;
			  }
			  return *this;
		  }

		  /** @brief 購読を解除する（未購読なら何もしない） */
		  void reset() noexcept
		  {
			  if (m_bus != nullptr)
			  {
				  m_bus->unsubscribe(m_type, m_id);
				  m_bus = nullptr;
			  }
		  }

		private:
		  EventBus* m_bus{ nullptr };
		  std::type_index m_type{ typeid(void) };
		  std::uint64_t m_id{ 0 };
	  };

	  /**
	   * @brief イベントを購読する
	   * @tparam TEvent イベントの型
	   * @param callback イベント発行時に呼ばれるコールバック関数
	   * @return 購読ハンドル。破棄すると購読が解除されるため必ず保持すること
	   */
	  template <typename TEvent>
	  [[nodiscard]] Subscription subscribe(std::function<void(const TEvent&)> callback)
	  {
		  const std::type_index key{ typeid(TEvent) };
		  const std::uint64_t id{ m_nextId++ };

		  // 型消去して保持する。std::anyを介さないためイベントのコピーは発生しない
		  auto invoker{ [callback = std::move(callback)](const void* e)
			  {
				  callback(*static_cast<const TEvent*>(e));
			  } };

		  auto& listeners{ m_listeners[key] };

		  // 解除済みの穴があれば再利用する（購読と解除を繰り返しても際限なく伸びないように）
		  for (auto& listener : listeners)
		  {
			  if (!listener.m_fn)
			  {
				  listener.m_id = id;
				  listener.m_fn = std::move(invoker);
				  return Subscription{ this, key, id };
			  }
		  }

		  listeners.push_back(Listener{ id, std::move(invoker) });
		  return Subscription{ this, key, id };
		}

		/**
		 * @brief イベントを発行する
		 * @tparam TEvent イベントの型
		 * @param event 発行するイベント
		 */
		template<typename TEvent>
		void publish(const TEvent& event)
		{
			const auto it{ m_listeners.find(std::type_index(typeid(TEvent))) };
			if (it == m_listeners.end()) return;

			// コールバック内で解除されるとm_fnが空になるため、都度チェックしてから呼ぶ。
			// 追加された分は今回の発行では呼ばないよう、開始時点の要素数までで打ち切る
			auto& listeners{ it->second };
			const std::size_t count{ listeners.size() };
			for (std::size_t i{ 0 }; i < count && i < listeners.size(); ++i)
			{
				if (listeners[i].m_fn)
					listeners[i].m_fn(&event);
			}
		}

	private:
	  struct Listener
	  {
		  std::uint64_t m_id{ 0 };
		  std::function<void(const void*)> m_fn{};
	  };

	  /**
	   * @brief 購読を解除する
	   *
	   * 要素を削除せず空にするだけにして、publish中に解除されても
	   * 反復中のインデックスがずれないようにする。
	   * @param type イベント型のtype_index
	   * @param id 購読ID
	   */
	  void unsubscribe(std::type_index type, std::uint64_t id) noexcept
	  {
		  const auto it{ m_listeners.find(type) };
		  if (it == m_listeners.end())
			  return;

		  for (auto& listener : it->second)
		  {
			  if (listener.m_id == id)
			  {
				  listener.m_fn = nullptr;
				  return;
			  }
		  }
	  }

	  std::unordered_map<std::type_index, std::vector<Listener>> m_listeners;
	  std::uint64_t m_nextId{ 1 };
	};
} // namespace core::base
