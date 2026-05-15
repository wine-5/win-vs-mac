#pragma once

namespace core::base
{
	/**
	 * @brief シングルトン基底クラスのテンプレート
	 * @tparam T シングルトンにするクラス型
	 */
	template<typename T>
	class Singleton
	{
	public:
		/**
		 * @brief シングルトンのインスタンスを初期化（依存関係注入用）
		 * 初期化処理を行うクラス（ServiceLocatorInitializer など）から呼び出される
		 * @tparam Args コンストラクタの引数型
		 * @param args コンストラクタの引数
		 */
		template<typename... Args>
		static void initialize(Args&&... args)
		{
			if (!m_instance)
				m_instance = std::make_unique<T>(std::forward<Args>(args)...);
		}

		/**
		 * @brief シングルトンのインスタンスを取得
		 * initialize() が呼び出されていることが前提。
		 * @return 唯一のインスタンスへの参照
		 */
		static T& getInstance()
		{
			return *m_instance;
		}

		/**
		 * @brief シングルトンのインスタンスをクリア
		 */
		static void destroy()
		{
			m_instance.reset();
		}

		// コピーコンストラクタ削除にして禁止にする
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;

		// ムーブセマンティクス削除
		Singleton(Singleton&&) = delete;
		Singleton& operator=(Singleton&&) = delete;

	protected:
		/**
		 * @brief デフォルトコンストラクタ
		 */
		Singleton() = default;

		/**
		 * @brief デフォルトデストラクタ
		 */
		~Singleton() = default;

	private:
		static std::unique_ptr<T> m_instance;
	};
}
