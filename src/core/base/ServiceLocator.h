#pragma once
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>
namespace core::base
{
	/**
	 * @brief グローバルなサービスへのアクセスを管理するクラス
	 */
	class ServiceLocator
	{
	public:
		/**
		 * @brief サービスを登録する（型と同じキーで登録）
		 * @tparam T サービスの型
		 * @param service 登録するサービス
		 */
		template<typename T>
		static void provide(std::unique_ptr<T> service)
		{
			registerService(std::type_index(typeid(T)), std::shared_ptr<void>(std::move(service)));
		}

		/**
		 * @brief サービスを登録する（インターフェース型をキーとして登録）
		 * @tparam TInterface インターフェース型
		 * @tparam TImpl 実装型
		 * @param service 登録するサービス
		 */
		template<typename TInterface, typename TImpl>
		static void provide(std::unique_ptr<TImpl> service)
		{
			registerService(std::type_index(typeid(TInterface)), std::shared_ptr<void>(std::move(service)));
		}

		/**
		 * @brief サービスを取得する
		 * @tparam T サービスの型
		 * @return サービスのポインタ
		 */
		template<typename T>
		static T* get()
		{
			auto key{std::type_index(typeid(T))};
			auto it{m_services.find(key)};
			assert(it != m_services.end() && "ServiceLocator: サービスが登録されていません");

			return static_cast<T*>(it->second.get());
		}

		/**
		 * @brief 全てのサービスをクリアする
		 *
		 * 登録の逆順で破棄する。後から登録したサービスが先に登録したものを参照している
		 * （例：SceneManager配下のSystemがILightingを使う）ため、順不同で壊すと
		 * デストラクタが解放済みのサービスを触って終了時に落ちる。
		 */
		static void clear()
		{
			for (auto it{ m_order.rbegin() }; it != m_order.rend(); ++it)
				m_services.erase(*it);

			m_services.clear();
			m_order.clear();
		}

	private:
	  /**
	   * @brief サービスを登録し、破棄順のために登録順を控える
	   * @param key サービスのキー
	   * @param service 登録するサービス
	   */
	  static void registerService(std::type_index key, std::shared_ptr<void> service)
	  {
		  if (m_services.find(key) == m_services.end())
			  m_order.push_back(key);

		  m_services[key] = std::move(service);
	  }

		static std::unordered_map<std::type_index, std::shared_ptr<void>> m_services;
		// 破棄を登録の逆順で行うための登録順
		static std::vector<std::type_index> m_order;
	};
} // namespace core::base