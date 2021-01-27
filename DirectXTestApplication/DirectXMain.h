#pragma once

#include "DeviceResources.h"

namespace DirectX
{
    class DirectXMain
    {
    public:
        DirectXMain(std::shared_ptr<DeviceResources> deviceResource);

        void StartRenderLoop();

    private:
        void update();
        bool render();

        std::shared_ptr<DeviceResources> m_spDeviceResource;
        Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
        Concurrency::critical_section m_criticalSection;

    };
}
