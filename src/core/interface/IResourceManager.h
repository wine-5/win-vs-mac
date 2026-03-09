#pragma once
#include <string>
#include <optional>
#include "core/data/ModelMetadata.h"

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
		virtual int loadModelById(const std::string& modelId) = 0;
		virtual std::optional<core::data::ModelMetadata> getMetadata(const std::string& modelId) const = 0;
	};
}
