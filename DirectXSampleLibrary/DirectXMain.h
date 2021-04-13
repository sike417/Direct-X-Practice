#pragma once

#include "DeviceResources.h"
#include "IScene.h"

using namespace Concurrency;

namespace DXResources
{
    class DirectXMain
    {
    public:
        DirectXMain(std::shared_ptr<DeviceResources> deviceResource);

        Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

        void SetCurrentScene(GraphicsScenes::IScene* nextScene) 
        {
            critical_section::scoped_lock lock(m_criticalSection);
            m_pCurrentScene = nextScene; 

            if (m_pCurrentScene != nullptr)
            {
                m_pCurrentScene->ActivateScene();
            }
        }

        void SetUpdateStatus(bool shouldUpdate)
        {
            m_bShouldUpdate = shouldUpdate;
        }

        void StartRenderLoop();

    private:
        void update();
        bool render();

        std::shared_ptr<DeviceResources> m_spDeviceResource;
        Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
        Concurrency::critical_section m_criticalSection;
        bool m_bShouldUpdate;

        // Not owned by DirectXMain so don't delete.
        GraphicsScenes::IScene* m_pCurrentScene;

    };
}
