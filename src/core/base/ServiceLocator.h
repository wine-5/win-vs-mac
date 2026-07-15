#pragma once
#include <unordered_map>
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
			auto key{std::type_index(typeid(T))};
			m_services[key] = std::shared_ptr<void>(std::move(service));
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
			auto key{std::type_index(typeid(TInterface))};
			m_services[key] = std::shared_ptr<void>(std::move(service));
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
		 * @brief 既存のサービスインスタンスを登録する（所有権は持たない）
		 * @tparam TInterface インターフェース型（キーとして使用）
		 * @tparam TImpl 実装型のポインタ
		 * @param service 既存のサービスインスタンスへのポインタ
		 *
		 * 注：ServiceLocatorは所有権を持たず、参照するだけ。
		 * インスタンスのライフサイクルは登録元が管理する。
		 */
		template<typename TInterface, typename TImpl>
		static void provideExisting(TImpl* service)
		{
			auto key{std::type_index(typeid(TInterface))};
			// デリータを空にして、削除時にインスタンスを破棄しない
			m_services[key] = std::shared_ptr<void>(
				static_cast<void*>(static_cast<TInterface*>(service)),
				[](void*) {}  // 削除しない（インスタンスの所有権を持たないため）
			);
		}

		/**
		 * @brief 全てのサービスをクリアする
		 */
		static void clear()
		{
			m_services.clear();
		}

	private:
		static std::unordered_map<std::type_index, std::shared_ptr<void>> m_services;
	};
} // namespace core::base