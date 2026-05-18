#pragma once
#include "IScene.h"
#include "SceneType.h"
#include <memory>
#include <optional>

namespace game::scene
{
	class SceneFactory;  // 前方宣言

	/**
	 * @brief シーン全体を管理するクラス
	 */
	class SceneManager
	{
	public:
		/**
		 * @brief SceneManagerのコンストラクタ
		 * SceneFactoryを内部で生成して所有する
		 */
		SceneManager();
		
		/**
		 * @brief デストラクタ
		 */
		~SceneManager();

		/**
		 * @brief 現在のシーンを更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime);
		
		/**
		 * @brief 現在のシーンを描画する
		 */
		void draw();
		
		/**
		 * @brief シーンを変更する
		 * @param sceneType 変更先のシーンの種類
		 */
		void changeScene(SceneType sceneType);

	private:
		std::unique_ptr<SceneFactory> m_sceneFactory;
		IScene* m_currentScene;
		SceneType m_currentSceneType{};
		/// @brief 翌フレームに破棄するシーンの種類（use-after-free 防止のための遅延リセット）
		std::optional<SceneType> m_pendingReset{};
	};
}