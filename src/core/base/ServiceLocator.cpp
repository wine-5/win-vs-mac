#include "ServiceLocator.h"

namespace core::base
{
    std::unordered_map<std::type_index, std::shared_ptr<void>> ServiceLocator::m_services;
	std::vector<std::type_index> ServiceLocator::m_order;
} // namespace core::base