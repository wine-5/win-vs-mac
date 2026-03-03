#pragma once
#include <string>

namespace core::iface
{
	/**
	 * @brief リソースの読み込み・管理を行う純粋仮想クラス
	 * game層がInfrastructure層に直接依存しないようにするための抽象化
	*/
	class IResourceManager
	{
	public:
		virtual ~IResourceManager() = default;
		virtual int loadModel(const std::string& filePath) = 0;
		virtual void unloadModel(const std::string& filePath) = 0;
		virtual void unloadAll() = 0;
	};
}
