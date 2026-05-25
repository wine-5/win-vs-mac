#include "infrastructure/EffectFactory.h"
#include <vector>
#include "thirdparty/effekseer/EffekseerForDXLib.h"
#include <core\interface\ILogger.h>

namespace infrastructure
{
	EffectFactory::~EffectFactory()
	{
		// Effekseer 終了処理
		Effkseer_End();
		m_pools.clear();
		m_handleToType.clear();
	}

	void EffectFactory::initialize()
	{
		// Effekseer初期化: DirectX11, Z-buffer有効, 最大パーティクル数8000
		// 注: DxLib のドキュメントに従い、カメラ設定後に初期化する必要がある
		int initResult{ Effekseer_Init(8000) };

		if (initResult == -1)
		{
			return;
		}

		// Effekseer を 3D モードに設定
		Effekseer_Sync3DSetting();

		m_repository.initialize();

		const std::vector<std::pair<core::constant::EffectType, int>> typePoolSizes
		{
			{core::constant::EffectType::Hit, HIT_POOL_SIZE}
		};

		for (const auto& [type, poolSize] : typePoolSizes)
		{
			int resourceHandle{ m_repository.getHandle(type) };
			if (resourceHandle == -1) continue;
			m_pools[type].initialize(resourceHandle, poolSize);
		}
	}

	int EffectFactory::play(core::constant::EffectType type, core::Vector3 position)
	{
		// 対応するプールを検索する
		auto it{ m_pools.find(type) };
		if (it == m_pools.end()) return -1;

		// プールからスロットを取得してエフェクトを再生する
		int handle{ it->second.getEffect(position) };
		if (handle == -1) return -1;

		// stop()で正しいプールに返却できるようハンドルを記録する
		m_handleToType[handle] = type;
		return handle;
	}

	void EffectFactory::stop(int handle)
	{
		// ハンドルからプールの種類を逆引きする
		auto it{ m_handleToType.find(handle) };
		if (it == m_handleToType.end()) return;

		// 対応するプールにスロットを返却する
		auto poolIt{ m_pools.find(it->second) };
		if (poolIt != m_pools.end())
			poolIt->second.returnEffect(handle);

		m_handleToType.erase(it);
	}

	void EffectFactory::update()
	{
		UpdateEffekseer3D();

		// 全プールの自然終了チェックを行う
		for (auto& [type, pool] : m_pools)
		{
			pool.update();
		}

		std::erase_if(m_handleToType, [this](const auto& pair)
			{
				auto poolIt{ m_pools.find(pair.second) };
				if (poolIt == m_pools.end()) return true;

				return !poolIt->second.isActive(pair.first); // 再生が終わっていれば除去する
			});
	}

	bool EffectFactory::isPlaying(int handle) const
	{
		// m_handleToType にハンドルが存在すれば再生中
		return m_handleToType.contains(handle);
	}

	void EffectFactory::draw()
	{
		DrawEffekseer3D();
	}

}