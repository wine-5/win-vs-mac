#include "MiniMapProjection.h"
#include <cmath>

namespace game::utility
{
	MiniMapPoint projectToMiniMap(float worldX, float worldZ,
	    float centerX, float centerZ, float yaw, float scale) noexcept
	{
		const float dx{ worldX - centerX };
		const float dz{ worldZ - centerZ };

		// 自機の向きが上に来るよう、中心からの差分をyawぶん回す
		const float sinYaw{ std::sin(yaw) };
		const float cosYaw{ std::cos(yaw) };
		const float rotatedX{ dx * cosYaw - dz * sinYaw };
		const float rotatedZ{ dx * sinYaw + dz * cosYaw };

		// ワールドの+Z（奥）を画面の上へ向けるためYを反転する
		return MiniMapPoint{ rotatedX * scale, -rotatedZ * scale };
	}

	float projectYawToMiniMap(float worldYaw, float yaw) noexcept
	{
		// Y反転のぶん符号が入れ替わる。位置側と同じ符号になるようマップの回転も引く
		return -worldYaw - yaw;
	}
} // namespace game::utility
