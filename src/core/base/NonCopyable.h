#pragma once

namespace core::base
{
	/**
	 * @brief コピー・ムーブを禁止する基底クラス
	 *
	 * 唯一のインスタンスを所有者（Applicationなど）が明示的に管理するクラスに
	 * 継承させ、意図しない複製を型レベルで防ぐ。
	 * Singletonと違い静的アクセスは提供しないため、参照は必ず注入で受け渡す。
	 */
	class NonCopyable
	{
	  public:
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
		NonCopyable(NonCopyable&&) = delete;
		NonCopyable& operator=(NonCopyable&&) = delete;

	  protected:
		/**
		 * @brief デフォルトコンストラクタ（派生クラスからのみ生成可能）
		 */
		NonCopyable() = default;

		/**
		 * @brief デフォルトデストラクタ
		 */
		~NonCopyable() = default;
	};
} // namespace core::base
