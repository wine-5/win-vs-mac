#include "SingletonInitializer.h"
#include "game/GameManager.h"

void SingletonInitializer::init()
{
	// GameManager Singleton の初期化
	game::GameManager::initialize();
}
