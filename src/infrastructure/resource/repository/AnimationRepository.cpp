#include "AnimationRepository.h"
#include <DxLib.h>
#include <stdexcept>
#include "thirdparty/nlohmann/json.hpp"
#include "core/base/ServiceLocator.h"
#include "core/interface/ILogger.h"
#include "core/utility/Log.h"

namespace infrastructure::resource::repository
{
	AnimationRepository::AnimationRepository(const nlohmann::json& j)
	{
		if (!j.contains("animations")) return;

		for (const auto& item : j["animations"])
			m_paths[item["id"].get<std::string>()] = item["path"].get<std::string>();
	}

	AnimationRepository::~AnimationRepository()
	{
		// ここで MV1DeleteModel を呼んではいけない。
		//
		// アニメーションは MV1AttachAnim でキャラのモデルへ紐付けられており、
		// アタッチ先（キャラのモデルや EnemySpawner が持つ複製ハンドル）より先に
		// アタッチ元を消すと、DxLib_End 内の MV1Terminate が残ったモデルを片付ける際に
		// 解放済みのアニメーションを detach しようとしてアクセス違反になる。
		//
		// これらのハンドルはアプリ終了まで保持し続けるキャッシュであり、
		// ここで消しても寿命は変わらないため、解放は依存関係を正しく扱える
		// DxLib_End（MV1Terminate）へ一任する。
	}

	int AnimationRepository::loadAnimationById(std::string_view animationId)
	{
		const std::string id{ animationId };

		auto handleIt{ m_handles.find(id) };
		if (handleIt != m_handles.end())
			return handleIt->second;

		auto pathIt{ m_paths.find(id) };
		if (pathIt == m_paths.end())
		{
			core::log::error("アニメーションID '{}' が見つかりません", id.c_str());
			return -1;
		}

		const int handle{ MV1LoadModel(pathIt->second.c_str()) };
		if (handle == -1)
		{
			core::log::error("アニメーションの読み込みに失敗しました: {}", pathIt->second.c_str());
			return -1;
		}

		m_handles[id] = handle;
		return handle;
	}
} // namespace infrastructure::resource::repository
