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
        }

        void StartRenderLoop();

        void TrackingUpdate(float positionX);

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
