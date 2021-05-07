#include "pch.h"
#include "RenderSphereScene.h"
#include "SphereRenderable.h"
#include "DirectXHelper.h"

using namespace GraphicsScenes;
using namespace DXResources;
using namespace DirectX;
using namespace Windows::Foundation;

GraphicsScenes::RenderSphereScene::RenderSphereScene()
    : m_fTotalTime(0)
{
    if (s_spCamera == nullptr || s_spDeviceResources == nullptr)
    {
        throw ref new FailureException();
    }

    createDeviceDependentResources();
}

void GraphicsScenes::RenderSphereScene::Update()
{
    static const float TimePerUpdate = (float)(1) / 60;
    m_fTotalTime += TimePerUpdate;

    float radiansPerSecond = XMConvertToRadians(20);
    double totalRotation = m_fTotalTime * radiansPerSecond;
    float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

    DXResources::Transform& transform = m_vScenePrimitives[0]->GetTransform();
    transform.SetRotation(radians);
}

void GraphicsScenes::RenderSphereScene::Render()
{
    auto d3dContext = s_spDeviceResources->GetD3DDeviceContext();
    auto vpBuffer = s_spCamera->GetCombinedVPBuffer();

    // load the view projection matrix, shared by all objects in a scene.
    d3dContext->UpdateSubresource1(
        m_vpConstantBuffer.Get(),
        0,
        NULL,
        &vpBuffer,
        0,
        0,
        0
    );

    // Send the constant buffer to the graphics device.
    d3dContext->VSSetConstantBuffers1(
        0,
        1,
        m_vpConstantBuffer.GetAddressOf(),
        nullptr,
        nullptr
    );

    for (int i = 0; i < m_vScenePrimitives.size(); i++)
    {
        if (m_vScenePrimitives[i] != nullptr)
        {
            m_vScenePrimitives[i]->drawShape();
        }
    }
}

void GraphicsScenes::RenderSphereScene::ActivateScene()
{
    // Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
    static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
    static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    s_spCamera->SetCameraView(eye, at, up);
}

void GraphicsScenes::RenderSphereScene::createDeviceDependentResources()
{ 
    auto d3dContext = s_spDeviceResources->GetD3DDevice();

    CD3D11_BUFFER_DESC constantBufferDesc(sizeof(CombinedViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    DXResources::ThrowIfFailed(
        d3dContext->CreateBuffer(
            &constantBufferDesc,
            nullptr,
            &m_vpConstantBuffer
        )
    );

    m_vScenePrimitives.push_back(new SphereRenderable());
}
