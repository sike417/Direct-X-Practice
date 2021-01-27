#include "pch.h"
#include "DirectXMain.h"

using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

DirectX::DirectXMain::DirectXMain(std::shared_ptr<DeviceResources> deviceResource)
    : m_spDeviceResource(deviceResource)
    , m_renderLoopWorker(nullptr)
    , m_criticalSection()
{

}

void DirectX::DirectXMain::StartRenderLoop()
{
    if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
    {
        return;
    }

    auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action)
    {
        while (action->Status == AsyncStatus::Started)
        {
            critical_section::scoped_lock lock(m_criticalSection);
            update();
            if (render())
            {
                // Present
                m_spDeviceResource->PresentView();
            }
        }
    });

    m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DirectX::DirectXMain::update()
{
}

bool DirectX::DirectXMain::render()
{
    return true;
}
