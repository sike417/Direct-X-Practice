#include "pch.h"
#include "RenderCircleSceneV2.h"
#include "DirectXHelper.h"
#include "CircleRenderableV2.h"
#include "TextRenderable.h"

using namespace GraphicsScenes;
using namespace DXResources;
using namespace DirectX;
using namespace Windows::Foundation;

GraphicsScenes::RenderCircleSceneV2::RenderCircleSceneV2()
    : m_fTotalTime(0)
    , m_iCurrentVertexCount(0)
{
    if (s_spCamera == nullptr || s_spDeviceResources == nullptr)
    {
        throw ref new FailureException();
    }

    createDeviceDependentResources();
}

void GraphicsScenes::RenderCircleSceneV2::Update()
{
    static const float TimePerUpdate = (float)(1) / 60;
    m_fTotalTime += TimePerUpdate;

    // calculate current number of vertices.
    int numberOfSeconds = m_fTotalTime;

    if (numberOfSeconds > 10)
    {
        m_fTotalTime = 0;
        numberOfSeconds = 0;
    }

    static const int secondsPerChange = 1;
    static const int minimumVerticesPerQuadrant = 2;
    int numberOfVertices = (numberOfSeconds / secondsPerChange) + minimumVerticesPerQuadrant;

    if (numberOfVertices == m_iCurrentVertexCount)
    {
        return;
    }

    m_iCurrentVertexCount = numberOfVertices;
    ((CircleRenderableV2*)m_vScenePrimitives.front())->UpdateNumberOfVertices(numberOfVertices);

    ((TextRenderable*)m_vScenePrimitives[1])->UpdateDisplayText(std::to_wstring(numberOfVertices * 4) + L" Vertices");
}

void GraphicsScenes::RenderCircleSceneV2::Render()
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

void GraphicsScenes::RenderCircleSceneV2::ActivateScene()
{
    // Eye is at (0,0,1.5), looking at point (0,0,0) with the up-vector along the y-axis.
    static const XMVECTORF32 eye = { 0.0f, 0.0f, 1.5f, 0.0f };
    static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    s_spCamera->SetCameraView(eye, at, up);
}

void GraphicsScenes::RenderCircleSceneV2::createDeviceDependentResources()
{
    auto d3dContext = s_spDeviceResources->GetD3DDevice();

    CD3D11_BUFFER_DESC constantBufferDesc(sizeof(CombinedViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    DXResources::ThrowIfFailed(
        d3dContext->CreateBuffer(
            &constantBufferDesc,
            nullptr,
            &m_vpConstantBuffer
        ));

    m_vScenePrimitives.push_back(new CircleRenderableV2());

    auto textRenderable = new TextRenderable();
    m_vScenePrimitives.push_back(textRenderable);

    // bottom right of screen
    textRenderable->GetTransform().SetPosition(1, 0, 0);
}
