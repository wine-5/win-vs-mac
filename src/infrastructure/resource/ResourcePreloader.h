#pragma once
#include <deque>
#include <string>
#include <string_view>
#include "core/interface/IResourcePreloader.h"

namespace core::iface
{
	class IResourceManager; // 前方宣言
}

namespace infrastructure::resource
{
	/**
	 * @brief リソースをフレーム分割で先読みするクラス
	 *
	 * キューに積まれたIDを、1フレームあたり決められた時間だけ消化する。
	 * 実際の読み込みは IResourceManager に委譲するだけで、
	 * ハンドルのキャッシュもそちら側の責務。
	 */
	class ResourcePreloader final : public core::iface::IResourcePreloader
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param resourceManager 実際の読み込みを行うリソース管理
		 */
		explicit ResourcePreloader(core::iface::IResourceManager& resourceManager) noexcept;

		/**
		 * @brief 先読みするリソースを1件キューに積む
		 * @param kind リソース種別
		 * @param id リソースID
		 */
		void enqueue(core::iface::PreloadKind kind, std::string_view id) override;

		/**
		 * @brief 登録されている全リソースをキューに積む
		 */
		void enqueueAll() override;

		/**
		 * @brief キューを予算の範囲で消化する
		 * @param budgetMilliseconds このフレームで先読みに使ってよい時間（ミリ秒）
		 * @return 実際に処理した件数
		 */
		int step(int budgetMilliseconds) override;

		/**
		 * @brief キューが空になったかどうかを返す
		 * @return 先読みが完了している場合true
		 */
		[[nodiscard]] bool isComplete() const noexcept override;

		/**
		 * @brief 先読みの進捗を返す
		 * @return 進捗（0.0〜1.0）。積んだものが無い場合は1.0
		 */
		[[nodiscard]] float getProgress() const noexcept override;

	  private:
		/// @brief 先読み待ちの1件
		struct Request
		{
			core::iface::PreloadKind m_kind{};
			std::string m_id{};
		};

		core::iface::IResourceManager& m_resourceManager;

		std::deque<Request> m_queue{};

		// 進捗計算用。分母は enqueue のたびに増える（途中で積み増しても破綻しないように）
		int m_totalCount{ 0 };
		int m_doneCount{ 0 };
	};
} // namespace infrastructure::resource
