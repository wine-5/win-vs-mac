#include "infrastructure/EffectPool.h"
#include "thirdparty/effekseer/EffekseerForDXLib.h"
#include "core/interface/ILogger.h"

namespace infrastructure
{
	void EffectPool::initialize(int resourceHandle, int poolSize)
	{
		m_resourceHandle = resourceHandle;

		core::base::ObjectPool<EffectSlot>::Config config{};
		config.m_initialSize = poolSize;
		config.m_expandSize  = poolSize;
		config.m_autoExpand  = true;

		core::base::ObjectPool<EffectSlot>::Callbacks callbacks{};
		callbacks.onCreate = []() {return std::make_unique<EffectSlot>(); };
		callbacks.onGet    = [](EffectSlot& slot) {slot.m_playHandle = -1; };
		callbacks.onReturn = [](EffectSlot& slot)
			{
				if (slot.m_playHandle != -1)
				{
					StopEffekseer3DEffect(slot.m_playHandle);
					slot.m_playHandle = -1;
				}
			};

		m_pool.initialize(config, callbacks);
	}

	int EffectPool::getEffect(core::Vector3 position)
	{
		// プールからスロットを取得（足りないかつ自動拡張をオフの場合はnullptr）
		EffectSlot* slot{ m_pool.getObject() };
		if (!slot) return -1;

		// エフェクトを再生してプレイハンドルを受け取る
		int handle{ PlayEffekseer3DEffect(m_resourceHandle) };
		if (handle == -1)
		{
			m_pool.returnObject(slot);
			return -1;
		}

		// 再生位置を設定する
		SetPosPlayingEffekseer3DEffect(handle, position.x, position.y, position.z);
		slot->m_playHandle = handle;
		m_activeSlots.push_back(slot);
		LOG("[EffectPool] エフェクト生成 handle=%d pos=(%.1f, %.1f, %.1f)", handle, position.x, position.y, position.z);
		return handle;
	}

	void EffectPool::returnEffect(int playHandle)
	{
		auto it{ m_activeSlots.begin() };
		while (it != m_activeSlots.end())
		{
			if ((*it)->m_playHandle == playHandle)
			{
				// returnObject内のonReturnコールバックでStopが呼ばれる
				LOG("[EffectPool] エフェクト返却 handle=%d", playHandle);
				m_pool.returnObject(*it);
				it = m_activeSlots.erase(it);
				return;
			}
			++it;
		}
	}

	void EffectPool::update()
	{
		// 再生が終了したスロットをプールに自動返却する
		auto it{ m_activeSlots.begin() };
		while (it != m_activeSlots.end())
		{
			if (!IsEffekseer3DEffectPlaying((*it)->m_playHandle))
			{
				// すでに終了しているためStopは不要でハンドルだけリセットして返却する
				LOG("[EffectPool] エフェクト自動返却 handle=%d", (*it)->m_playHandle);
				(*it)->m_playHandle = -1;
				m_pool.returnObject(*it);
				it = m_activeSlots.erase(it);
			}
			else
				++it;
		}
	}

	bool EffectPool::isActive(int playHandle) const
	{
		for (const auto& slot : m_activeSlots)
		{
			if (slot->m_playHandle == playHandle) return true;
		}
		return false;
	}
}