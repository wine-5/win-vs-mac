#pragma once
#include "IScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IFileProvider.h"
#include "game/data/FileEquipmentData.h"
#include <memory>

namespace game::scene
{
    class SelectView;

    /**
     * @brief 選択シーンのクラス
     */
    class Select : public IScene
    {
    public:
        /**
         * @brief Selectのコンストラクタ
         * @param inputProvider 入力インターフェース
         * @param uiRenderer UI描画インターフェース
         * @param screen 画面情報インターフェース
         * @param fileProvider ファイル選択インターフェース
         * @param fileEquipmentData 選択ファイルデータの参照
         */
        Select(core::iface::IInputProvider &inputProvider,
               core::iface::IUIRenderer &uiRenderer,
               core::iface::IScreen &screen,
               core::iface::IFileProvider &fileProvider,
               data::FileEquipmentData &fileEquipmentData);

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
        std::unique_ptr<SelectView> m_view;
    };
}
