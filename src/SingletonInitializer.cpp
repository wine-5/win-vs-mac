#include "SingletonInitializer.h"
#include "infrastructure/ResourceManager.h"

void SingletonInitializer::init()
{
	infrastructure::ResourceManager::getInstance();
}
