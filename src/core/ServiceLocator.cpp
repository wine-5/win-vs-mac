#include "ServiceLocator.h"

namespace core
{
    std::unordered_map<std::type_index, std::shared_ptr<void>> ServiceLocator::m_services;
}