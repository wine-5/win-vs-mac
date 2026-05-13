#pragma once

/**
 * @brief Singletonインスタンスの初期化順序を管理するクラス
 *
 * ゲーム全体で1つだけ存在すべきManagerクラスの初期化順序を定義し保証する
 * ServiceLocatorInitializer よりも先に呼ぶ必要がある
 */
class SingletonInitializer
{
public:
	/**
	 * @brief 全てのSingletonインスタンスを初期化する
	 */
	static void init();
};
