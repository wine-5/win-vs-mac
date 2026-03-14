#pragma once
/**
 * @brief ServiceLocatorへのサービス登録を一元管理するクラス
 * ゲーム起動時に使用するサービスをここに登録する
 */
class ServiceLocatorInitializer
{
public:
    /**
     * @brief ServiceLocatorに全サービスを登録する
     * @param screenWidth 画面幅
     * @param screenHeight 画面高さ
     */
    static void init(int screenWidth, int screenHeight);
};