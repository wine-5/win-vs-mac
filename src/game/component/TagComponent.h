#pragma once
#include "game/constant/Tag.h"

namespace game::component
{
    /**
     * @brief オブジェクトの種類を識別するコンポーネント
     */
    struct TagComponent
    {
        constant::Tag m_tag{ constant::Tag::None };
    };
} // namespace game::component