#pragma once
#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace core::base
{
	/**
	 * @brief 汎用オブジェクトプール
	 * @tparam T プールするオブジェクトの型
	 */
	template<typename T>
	class ObjectPool
	{
	public:
		/**
		 * @brief プールの初期設定
		 */
		struct Config
		{
			int m_initialSize{ 5 };    // 初期生成数
			int m_expandSize{ 5 };	   // Poolが空になったときに追加で生成する個数
			bool m_autoExpand{ true }; // 自動で追加するか
		};

		/**
		 * @brief 生成・取得・返却時のコールバック
		 */
		struct Callbacks
		{
			std::function<std::unique_ptr<T>()> onCreate;
			std::function<void(T&)>             onGet;
			std::function<void(T&)>             onReturn;
		};

		/**
		 * @brief プールを初期化する
		 * @param config プールの設定
		 * @param callbacks 生成・取得・返却時のコールバック
		 */
		void initialize(Config config, Callbacks callbacks)
		{
			m_config = config;
			m_callbacks = std::move(callbacks);
			expand(m_config.m_initialSize);
		}

		/**
		 * @brief プールからオブジェクトを取得する
		 * @return オブジェクトのポインタ（枯渇かつ非自動拡張の場合は nullptr）
		 */
		T* getObject()
		{
			if (m_available.empty())
			{
				if (!m_config.m_autoExpand) return nullptr;
				expand(m_config.m_expandSize);
			}

			T* obj{ m_available.front() };
			m_available.pop();
			if (m_callbacks.onGet)
				m_callbacks.onGet(*obj);
			return obj;
		}

		/**
		 * @brief オブジェクトをプールに返却する
		 * @param obj 返却するオブジェクトのポインタ
		 */
		void returnObject(T* obj)
		{
			if (!obj) return;
			if (m_callbacks.onReturn)
				m_callbacks.onReturn(*obj);
			m_available.push(obj);
		}

		/**
		 * @brief 現在の空きスロット数を返す
		 * @return 空きスロット数
		 */
		[[nodiscard]] int getAvailableCount() const
		{
			return static_cast<int>(m_available.size());
		}

	private:

		void expand(int count)
		{
			for (int i{}; i < count; ++i)
			{
				m_all.push_back(m_callbacks.onCreate());
				m_available.push(m_all.back().get());
			}
		}

		Config                           m_config{};
		Callbacks                        m_callbacks{};
		std::queue<T*>                   m_available{};
		std::vector<std::unique_ptr<T>>  m_all{};
	};
}