#pragma once
#include <string_view>

namespace core::iface
{
	/// @brief 先読みするリソースの種別
	enum class PreloadKind
	{
		Model,
		Animation,
		Image
	};

	/**
	 * @brief リソースを毎フレーム少しずつ先読みする純粋仮想クラス
	 *
	 * IResourceManager の loadXxxById() はID単位でハンドルをキャッシュするため、
	 * ゲーム開始直後から先に読んでおけば、実際に使う側（InGameなど）は
	 * キャッシュヒットで済み、シーン遷移時のロード待ちが消える。
	 *
	 * スレッドは使わず、メインループの空き時間をフレーム予算ぶんだけ削って進める。
	 * DxLibのハンドル操作が全てメインスレッド上に留まるため、
	 * 非同期ロード（SetUseASyncLoadFlag）で起きる未完了ハンドルへのアクセス問題が起きない。
	 */
	class IResourcePreloader
	{
	  public:
		virtual ~IResourcePreloader() = default;

		/**
		 * @brief 先読みするリソースを1件キューに積む
		 *
		 * 既に読み込み済みのIDを積んでも実害はない（ロード側がキャッシュを返すだけ）。
		 * そのため呼び出し側で「もう積んだか」を管理する必要はない。
		 * @param kind リソース種別
		 * @param id リソースID
		 */
		virtual void enqueue(PreloadKind kind, std::string_view id) = 0;

		/**
		 * @brief 登録されている全リソースをキューに積む
		 */
		virtual void enqueueAll() = 0;

		/**
		 * @brief キューを予算の範囲で消化する
		 *
		 * 1件あたりの所要時間は事前に分からないため、予算判定は「1件処理した後」に行う。
		 * 先に判定すると、予算より重いリソースが永久に処理されなくなる。
		 * @param budgetMilliseconds このフレームで先読みに使ってよい時間（ミリ秒）
		 * @param contextName ログに残す呼び出し元の名前（どのシーン中に読めたかの記録用）
		 * @return 実際に処理した件数
		 */
		virtual int step(int budgetMilliseconds, std::string_view contextName) = 0;

		/**
		 * @brief キューが空になったかどうかを返す
		 * @return 先読みが完了している場合true
		 */
		[[nodiscard]] virtual bool isComplete() const noexcept = 0;

		/**
		 * @brief 先読みの進捗を返す
		 * @return 進捗（0.0〜1.0）。積んだものが無い場合は1.0
		 */
		[[nodiscard]] virtual float getProgress() const noexcept = 0;
	};
} // namespace core::iface
