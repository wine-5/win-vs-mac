#pragma once
#include <string>

namespace core
{
	/**
	* @brief リソースの読み込み・管理を担当するクラス
	*/
	class IResourceManager
	{
	public:
		~IResourceManager() = default;
		virtual int loadModel(const std::string& filePath) = 0;
		virtual void unloadModel(const std::string& filePath) = 0;
		virtual void unloadAll() = 0;
	};
}
