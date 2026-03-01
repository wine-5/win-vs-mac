#pragma once
#include <unordered_map>
#include <string>

namespace infrastructure
{
	/**
	 * @brief リソースの読み込み・管理を担当するクラス
	 */
	class ResourceManager
	{
	public:
		int loadModel(const std::string& filePath);
		void unloadModel(const std::string& filePath);
		void unloadAll();

	private:
		// ファイルパスをキーにしてモデルを管理
		std::unordered_map<std::string, int> m_modelCache;

	};
}