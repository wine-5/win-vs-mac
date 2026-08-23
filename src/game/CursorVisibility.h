#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IInputProvider.h"

namespace game
{
	/**
	 * @brief マウスカーソルを出すかどうかを1か所で決めるクラス
	 *
	 * 以前は「出す／隠す」を各所が直接呼んでいたが、それぞれ理由が違ううえに
	 * 後から呼んだ方が勝つため、順序に依存した不具合が起きていた。
	 *
	 * そこで各所は真偽値ではなく**出したい理由**を立て下ろしするだけにし、
	 * 実際に出すかはここで毎フレーム計算する。理由ごとに書き手が1つに定まるので、
	 * 呼ぶ順番で結果が変わらなくなる。
	 */
	class CursorVisibility : private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief カーソルを出したい理由
		 *
		 * 書き手はそれぞれ1か所に限る。同じ理由を2か所から立てると、
		 * 片方が下ろしたときにもう片方の意図まで消える
		 */
		enum class Reason
		{
			Scene,     // いまの画面がポインタ前提（タイトル・セレクト・リザルトなど）
			PauseMenu, // ポーズメニューを開いている
			Settings,  // 設定パネルを開いている
			Inventory, // インベントリ（付け替え）を開いている
			Count,
		};

		/**
		 * @brief 理由ごとに「カーソルを出したい」を立て下ろしする
		 * @param reason 理由
		 * @param isNeeded 出したいなら true
		 */
		void setNeeded(Reason reason, bool isNeeded) noexcept;

		/**
		 * @brief パッドで操作していてもカーソルを使う画面かを設定する
		 *
		 * セレクト画面はパッドでカーソルそのものを動かすため、
		 * パッドを触っているからといって隠してはいけない
		 * @param isDriven パッドでカーソルを動かす画面なら true
		 */
		void setPointerDrivenByPad(bool isDriven) noexcept;

		/**
		 * @brief 毎フレーム呼び、必要なら表示を切り替える
		 *
		 * パッドを触ったら隠し、マウスを動かしたら出す。パッドが抜かれたときも
		 * 判定がキーボード側へ落ちるのでそのまま出る。
		 * 実際に呼ぶのは変化したときだけにする（毎フレーム同じ値を投げない）
		 * @param inputProvider 最後に触った機器を見て、表示を切り替えるのに使う
		 */
		void update(core::iface::IInputProvider& inputProvider);

	  private:
		static constexpr int REASON_COUNT{ static_cast<int>(Reason::Count) };

		/**
		 * @brief いまカーソルを出すべきかを返す
		 * @param inputProvider 最後に触った機器を見るのに使う
		 * @return 出すべきなら true
		 */
		[[nodiscard]] bool shouldShow(const core::iface::IInputProvider& inputProvider) const;

		bool m_needed[REASON_COUNT]{};
		bool m_isPointerDrivenByPad{ false };

		// 前回反映した状態。初期値は DxLib の既定（表示）に合わせる
		bool m_isVisible{ true };
	};
} // namespace game
