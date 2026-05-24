#include "infrastructure/EffectFactory.h"

namespace infrastructure
{
	void EffectFactory::initialize()
	{
		const std::vector<std::pair<core::constant::EffectType, int>> typePoolSizes
		{
			{core::constant::EffectType::Hit,HIT_POOL_SIZE}
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
		return 0;
	}

	void EffectFactory::stop(int handle)
	{
	}

	void EffectFactory::update()
	{
	}
}