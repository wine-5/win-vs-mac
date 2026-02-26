#include "Renderer.h"
#include <DxLib.h>
#include "utility/LogUtil.h"

namespace engine
{
    void Renderer::drawModel(int modelHandle, const core::Vector3& position)
    {
        if (modelHandle == -1)
        {
            utility::LogUtil::error("モデルが読み込まれていません");
            return;
        }

        VECTOR pos = { position.x, position.y, position.z };
        MV1SetPosition(modelHandle, pos);
        MV1DrawModel(modelHandle);
    }
}