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
		 * @brief シングルトンのインスタンスを取得
		 * @return 唯一のインスタンスへの参照
		 */
		static T& getInstance()
		{
			static T instance;
			return instance;
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
	};
}
