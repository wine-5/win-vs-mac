#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/utility/Vector3.h"
#include <random>
#include <vector>

namespace core::iface
{
	class IRenderer;        // 前方宣言
	class IResourceManager; // 前方宣言
} // namespace core::iface

namespace game::system::visual
{
	/**
	 * @brief 遠景の空に瞬く星を描画するSystem
	 *
	 * ステージの外は何も無い暗闇なので、そのままでは距離感も広がりも得られない。
	 * はるか遠方の空にだけ光を置き、瞬きで「見上げる空」を作る。
	 *
	 * 星はプレイヤーに追従するため視差が生まれず、無限遠の空として振る舞う。
	 * 描画は加算合成なので、暗い背景に対してだけ強く光る
	 */
	class BackgroundParticleSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief BackgroundParticleSystemのコンストラクタ
		 * @param componentManager プレイヤー位置の読み出しに使うComponentManagerの参照
		 * @param playerId 空を追従させる中心となるプレイヤーのEntityID
		 * @param renderer ビルボード描画・座標投影に使うインターフェース
		 * @param resourceManager 光の画像の読み込みに使うインターフェース
		 */
		BackgroundParticleSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId playerId,
		    core::iface::IRenderer& renderer,
		    core::iface::IResourceManager& resourceManager);

		/**
		 * @brief 瞬きの時間を進める
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 空を描画する
		 *
		 * 3D描画フェーズの先頭（他の全てより奥）で呼ぶこと
		 */
		void draw();

	  private:
		/** @brief 瞬く星1つぶんの状態 */
		struct Star
		{
			core::Vector3 m_direction{}; // 空の中心から見た方向（単位ベクトル）
			float m_distance{ 0.0f };    // 中心からの距離
			float m_size{ 0.0f };
			float m_twinklePhase{ 0.0f };
			float m_twinkleSpeed{ 0.0f };
			int m_brightness{ 0 };
		};

		/** @brief 転送中の1ファイル */
		struct FlyingFile
		{
			float m_progress{ 0.0f }; // 0.0＝転送元、1.0＝転送先
			float m_speed{ 0.0f };
			float m_spin{ 0.0f }; // 面内の傾き（ひらひら感を出す）
			float m_spinSpeed{ 0.0f };
			int m_imageHandle{ -1 };
		};

		/**
		 * @brief フォルダ間のファイル転送1組
		 *
		 * 空に2つのフォルダを並べ、その間をファイルが列をなして渡っていく。
		 * Windowsのコピー中アニメーションを遠景で再現し、
		 * 「PCの中でデータが運ばれている」ことを一目で伝える
		 */
		struct FileTransfer
		{
			core::Vector3 m_fromPosition{}; // 空の中心から見た相対位置
			core::Vector3 m_toPosition{};
			std::vector<FlyingFile> m_files{};
		};

		/**
		 * @brief 星を空のシェル上へ配置する
		 * @param star 対象の星
		 */
		void placeStar(Star& star);

		/**
		 * @brief ファイル転送1組を空に配置する
		 * @param transfer 対象の転送
		 * @param azimuth 空のどの方角に置くか（ラジアン）
		 */
		void placeTransfer(FileTransfer& transfer, float azimuth);

		/**
		 * @brief 転送中のファイルの現在位置を求める
		 * @param transfer 所属する転送
		 * @param progress 進行度（0.0〜1.0）
		 * @param center 空の中心座標
		 * @return ワールド座標
		 */
		[[nodiscard]] core::Vector3 computeFilePosition(const FileTransfer& transfer,
		    float progress, const core::Vector3& center) const;

		/**
		 * @brief フォルダ間のファイル転送を描画する
		 * @param center 空の中心座標
		 */
		void drawFileTransfers(const core::Vector3& center);

		/**
		 * @brief 空の中心座標を返す
		 * @return プレイヤーの上空（取得できなければ原点の上空）
		 */
		[[nodiscard]] core::Vector3 getSkyCenter() const;

		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId{};
		core::iface::IRenderer& m_renderer;

		int m_glowHandle{ -1 };
		int m_folderHandle{ -1 };

		std::vector<Star> m_stars{};
		std::vector<FileTransfer> m_transfers{};

		float m_elapsedTime{ 0.0f };

		// 起動のたびに空の配置を変える。他のSystemと同じくmt19937を使う
		// （std::randはこのプロジェクトのどこでもsrandしておらず、毎回同じ並びになる）
		std::mt19937 m_rng{ std::random_device{}() };
	};
} // namespace game::system::visual
