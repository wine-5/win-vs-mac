#include "SingletonInitializer.h"
#include "infrastructure/ResourceManager.h"
#include "game/GameManager.h"

void SingletonInitializer::init()
{
	// Singleton インスタンスを初期化
	infrastructure::ResourceManager::getInstance();
	game::GameManager::getInstance();
}
