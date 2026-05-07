#pragma once
#include "IScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IFileProvider.h"
#include "game/data/FileEquipmentData.h"   
#include "game/ui/UIManager.h"

namespace game::scene
{
    /**
     * @brief ステージ選択シーンのクラス
     */
    class Select : public IScene
    {
    public:
        /**
         * @brief StageSelectのコンストラクタ
         * @param inputProvider 入力インターフェース
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面情報インターフェース
         * @param fileProvider ファイル選択インターフェース
         * @param fileEquipmentData 選択ファイルデータの参照
         */
        Select(core::iface::IInputProvider& inputProvider,
            core::iface::IUIRenderer& uiRenderer,
            core::iface::IScreen& screen,
            core::iface::IFileProvider& fileProvider,
            data::FileEquipmentData& fileEquipmentData);

        /**
         * @brief シーンの更新処理
         * @param deltaTime フレーム間の時間差
         */
        void update(float deltaTime) override;

        /**
         * @brief シーンの描画処理
         */
        void draw() override;

    private:
        /**
         * @brief UIを構築する
         */
        void setupUI();

        core::iface::IInputProvider& m_inputProvider;
        core::iface::IUIRenderer& m_uiRenderer;
        core::iface::IScreen& m_screen;
        core::iface::IFileProvider& m_fileProvider;
        data::FileEquipmentData& m_fileEquipmentData; 
        ui::UIManager m_uiManager;
    };
}