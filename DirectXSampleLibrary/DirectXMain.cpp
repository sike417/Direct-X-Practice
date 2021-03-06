#include "pch.h"
#include "DirectXMain.h"
#include "DirectXHelper.h"

using namespace Windows::Foundation;
using namespace Windows::System::Threading;

DXResources::DirectXMain::DirectXMain(std::shared_ptr<DeviceResources> deviceResource)
    : m_spDeviceResource(deviceResource)
    , m_renderLoopWorker(nullptr)
    , m_criticalSection()
    , m_bShouldUpdate(true)
    , m_pCurrentScene(nullptr)
{

}

void DXResources::DirectXMain::SetRasterizerState(bool isWireFrame)
{
    critical_section::scoped_lock lock(m_criticalSection);

    auto d3dDevice = m_spDeviceResource->GetD3DDevice();
    auto d3dContext = m_spDeviceResource->GetD3DDeviceContext();

    D3D11_RASTERIZER_DESC rs;

    rs.FillMode = isWireFrame ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
    rs.CullMode = D3D11_CULL_BACK;
    rs.FrontCounterClockwise = false;
    rs.DepthBias = 0;
    rs.SlopeScaledDepthBias = 0.0f;
    rs.DepthBiasClamp = 0.0f;
    rs.DepthClipEnable = true;
    rs.ScissorEnable = false;
    rs.MultisampleEnable = false;
    rs.AntialiasedLineEnable = false;

    ID3D11RasterizerState* pState = nullptr;
    DXResources::ThrowIfFailed(d3dDevice->CreateRasterizerState(&rs, &pState));

    d3dContext->RSSetState(pState);
}

void DXResources::DirectXMain::StartRenderLoop()
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

            if (m_bShouldUpdate)
            {
                update();
            }

            if (render())
            {
                // Present
                m_spDeviceResource->PresentView();
            }
        }
    });

    m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DXResources::DirectXMain::update()
{
    if (m_pCurrentScene != nullptr)
    {
        m_pCurrentScene->Update();
    }
}

bool DXResources::DirectXMain::render()
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
