#include "infrastructure/EffectPool.h"
#include "thirdparty/effekseer/EffekseerForDXLib.h"

namespace infrastructure
{
	void EffectPool::initialize(int resourceHandle, int poolSize, float yOffset, float scale)
	{
		m_resourceHandle = resourceHandle;
		m_yOffset        = yOffset;
		m_scale          = scale;

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

		// 再生位置を設定する（足元座標に Y オフセットを加算してモデル中心に合わせる）
		SetPosPlayingEffekseer3DEffect(handle, position.x, position.y + m_yOffset, position.z);

		// エフェクトスケールを設定
		SetScalePlayingEffekseer3DEffect(handle, m_scale, m_scale, m_scale);

		slot->m_playHandle = handle;
		m_activeSlots.push_back(slot);

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
			int playingState{ IsEffekseer3DEffectPlaying((*it)->m_playHandle) };

			// -1 = 無効または終了状態
			if (playingState == -1)
			{
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
} // namespace infrastructure