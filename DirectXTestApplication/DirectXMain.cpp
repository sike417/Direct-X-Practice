#include "pch.h"
#include "DirectXMain.h"

using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

DirectX::DirectXMain::DirectXMain(std::shared_ptr<DeviceResources> deviceResource)
    : m_spDeviceResource(deviceResource)
    , m_renderLoopWorker(nullptr)
    , m_criticalSection()
    , m_pCurrentScene(nullptr)
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
    if (m_pCurrentScene != nullptr)
    {
        m_pCurrentScene->Update();
    }
}

bool DirectX::DirectXMain::render()
{
    auto context = m_spDeviceResource->GetD3DDeviceContext();

    // reset the viewport to target the entire screen.
    auto viewport = m_spDeviceResource->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Reset render targets to the screen.
    ID3D11RenderTargetView* const targets[1] = { m_spDeviceResource->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_spDeviceResource->GetDepthStencilView());

    // Clear the back buffer and depth stencil view.
    context->ClearRenderTargetView(m_spDeviceResource->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
    context->ClearDepthStencilView(m_spDeviceResource->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (m_pCurrentScene != nullptr)
    {
        m_pCurrentScene->Render();
    }

    return true;
}
