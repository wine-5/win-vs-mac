#pragma once
#include "IScene.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Vector3.h"
#include <vector>

namespace game::scene
{
	/**
	 * @brief DEBUG: ブロック破壊演出の検証シーン
	 *
	 * 「mqoモデルを破壊して破片を飛ばす演出がDxLibで本当に成立するか」を
	 * 実機で確かめるための使い捨てシーン。方式はプロトタイプ（docs/prototype/
	 * destruction.html）の「ボクセル分解」に相当し、ブロック1個を小さな立方体の
	 * 集合として描き、破壊時にそれぞれを飛散させる。
	 *
	 * @note ECS を通さずシーン内で完結させている。検証が済んだら破棄する前提であり、
	 *       Component/System を作ると本体側に消し忘れが残るため
	 * @note 破片モデルの用意が不要（ブロックモデルを複製して使う）なので、
	 *       あらかじめ割った mqo を作る前に「見た目」と「負荷」だけを先に確認できる
	 */
	class DebugDestruction : public IScene
	{
	  public:
		/**
		 * @brief DebugDestruction のコンストラクタ
		 * @param camera カメラインターフェース
		 * @param renderer 3D描画インターフェース
		 * @param resourceManager リソース管理インターフェース
		 * @param inputProvider 入力インターフェース
		 * @param uiRenderer UI描画インターフェース
		 * @param screen 画面情報インターフェース
		 */
		DebugDestruction(core::iface::ICamera& camera,
		    core::iface::IRenderer& renderer,
		    core::iface::IResourceManager& resourceManager,
		    core::iface::IInputProvider& inputProvider,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen);

		~DebugDestruction() override;

		/**
		 * @brief シーンの更新処理
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief シーンの描画処理
		 */
		void draw() override;

	  private:
		/// @brief 検証の進行段階
		enum class Phase
		{
			Intact, // 無傷〜ひび（Spaceで殴る）
			Broken  // 破片が飛散している
		};

		/// @brief 破片1つ分の状態
		struct Fragment
		{
			int m_modelHandle{ -1 };
			core::Vector3 m_home{}; // 破壊前の位置（リセット先）
			core::Vector3 m_position{};
			core::Vector3 m_velocity{};
			core::Vector3 m_rotation{}; // ラジアン
			core::Vector3 m_angular{};  // 回転速度（ラジアン/秒）
			float m_scale{ 1.0f };
			float m_alpha{ 1.0f };
		};

		void buildFragments();
		void hit();
		void explode();
		void reset();
		void updateFragments(float deltaTime);
		void drawHud();

		/// @brief 1辺あたりの分割数（GRID^3 個の破片になる）
		static constexpr int GRID{ 4 };

		/// @brief ブロックの実寸（stageCatalog.json の block_exe の defaultSize に合わせる）
		static constexpr float BLOCK_SIZE{ 200.0f };

		/// @brief block_exe のモデル素材の実寸（scale = 実寸 / BASE_SIZE）
		static constexpr float BASE_SIZE{ 100.0f };

		/// @brief 破壊までに必要な打撃回数
		static constexpr int HITS_TO_BREAK{ 3 };

		/// @brief 破片の初速（ユニット/秒）
		static constexpr float BURST_SPEED{ 520.0f };

		/// @brief 重力加速度（ユニット/秒^2）
		static constexpr float GRAVITY{ 1400.0f };

		/// @brief 破片が消えるまでの時間（秒）
		static constexpr float FRAGMENT_LIFE{ 1.4f };

		core::iface::ICamera& m_camera;
		core::iface::IRenderer& m_renderer;
		core::iface::IResourceManager& m_resourceManager;
		core::iface::IInputProvider& m_inputProvider;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;

		std::vector<Fragment> m_fragments;

		Phase m_phase{ Phase::Intact };
		int m_hitCount{ 0 };
		float m_phaseTime{ 0.0f };
		float m_shake{ 0.0f };
		float m_cameraAngle{ 0.0f };
	};
} // namespace game::scene
