#pragma once

#include "DeviceResources.h"
#include "IScene.h"

namespace DirectX
{
    class DirectXMain
    {
    public:
        DirectXMain(std::shared_ptr<DeviceResources> deviceResource);

        Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }
        void SetCurrentScene(GraphicsScenes::IScene* nextScene) { m_pCurrentScene = nextScene; }

        void StartRenderLoop();

    private:
        void update();
        bool render();

        std::shared_ptr<DeviceResources> m_spDeviceResource;
        Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
        Concurrency::critical_section m_criticalSection;

        // Not owned by DirectXMain so don't delete.
        GraphicsScenes::IScene* m_pCurrentScene;

    };
}
