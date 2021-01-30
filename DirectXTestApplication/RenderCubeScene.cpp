#include "pch.h"
#include "RenderCubeScene.h"
#include "DirectXHelper.h"

using namespace GraphicsScenes;
using namespace DirectX;
using namespace Windows::Foundation;

GraphicsScenes::RenderCubeScene::RenderCubeScene(std::shared_ptr<DirectX::DeviceResources> spDeviceResource)
    : m_spDeviceResources(spDeviceResource)
    , m_loadingComplete(false)
    , m_vertexCount(0)
    , m_indexCount(0)
    , m_tracking(false)
    , m_fTotalTime(0)
{
    createDeviceDependentResources();
}

void GraphicsScenes::RenderCubeScene::Update()
{
    if (!m_tracking)
    {
        static const float TimePerUpdate = (float)(1) / 60;

        m_fTotalTime += TimePerUpdate;

        // Convert degrees to radians, then convert seconds to rotation angle
        float radiansPerSecond = XMConvertToRadians(40);
        double totalRotation = m_fTotalTime * radiansPerSecond;// timer.GetTotalSeconds()* radiansPerSecond;
        float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

        rotate(radians);
    }
}

void GraphicsScenes::RenderCubeScene::Render()
{
    if (!m_loadingComplete)
    {
        return;
    }

    auto d3dContext = m_spDeviceResources->GetD3DDeviceContext();

    // Prepare the constant buffer to send it to the graphics device.
    d3dContext->UpdateSubresource1(
        m_constantBuffer.Get(),
        0,
        NULL,
        &m_constantBufferData,
        0,
        0,
        0
    );

    // Each vertex is one instance of the VertexPositionColor struct.
    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(
        0,
        1,
        m_vertexBuffer.GetAddressOf(),
        &stride,
        &offset
    );

    d3dContext->IASetIndexBuffer(
        m_indexBuffer.Get(),
        DXGI_FORMAT_R16_UINT,
        0
    );

    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    d3dContext->IASetInputLayout(m_inputLayout.Get());

    // Attach our vertex shader.
    d3dContext->VSSetShader(
        m_vertexShader.Get(),
        nullptr,
        0
    );

    // Send the constant buffer to the graphics device.
    d3dContext->VSSetConstantBuffers1(
        0,
        1,
        m_constantBuffer.GetAddressOf(),
        nullptr,
        nullptr
    );

    // Attach our pixel shader.
    d3dContext->PSSetShader(
        m_pixelShader.Get(),
        nullptr,
        0
    );

    d3dContext->DrawIndexed(m_indexCount, 0, 0);
}

void GraphicsScenes::RenderCubeScene::TrackingUpdate(float positionX)
{
    if (m_tracking)
    {
        float radians = XM_2PI * 2.0f * positionX / m_spDeviceResources->GetOutputSize().Width;
        rotate(radians);
    }
}

void GraphicsScenes::RenderCubeScene::CreateWindowSizeDependentResources()
{
    Size outputSize = m_spDeviceResources->GetOutputSize();
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // Note that the OrientationTransform3D matrix is post-multiplied here
    // in order to correctly orient the scene to match the display orientation.
    // This post-multiplication step is required for any draw calls that are
    // made to the swap chain render target. For draw calls to other targets,
    // this transform should not be applied.

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
    );

    //XMFLOAT4X4 orientation = XMFloat4x4::;// XMFLOAT4X4::ide m_spDeviceResources->GetOrientationTransform3D();

    XMMATRIX orientationMatrix = XMMatrixIdentity();// XMLoadFloat4x4(&orientation);

    XMStoreFloat4x4(
        &m_constantBufferData.projection,
        XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
    );

    // Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
    static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
    static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

void GraphicsScenes::RenderCubeScene::createDeviceDependentResources()
{
    auto d3dContext = m_spDeviceResources->GetD3DDevice();
    auto loadVSTask = DirectX::ReadDataAsync(L"RenderCubeVertexShader.cso");
    auto loadPSTask = DirectX::ReadDataAsync(L"RenderCubePixelShader.cso");

    auto createVSTask = loadVSTask.then([this, d3dContext](const std::vector<byte>& fileData) 
        {
            DirectX::ThrowIfFailed(
                d3dContext->CreateVertexShader(
                    &fileData[0],
                    fileData.size(),
                    nullptr,
                    &m_vertexShader));

            static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            DirectX::ThrowIfFailed(
                d3dContext->CreateInputLayout(
                    vertexDesc,
                    ARRAYSIZE(vertexDesc),
                    &fileData[0],
                    fileData.size(),
                    &m_inputLayout
                ));
        });

    auto createPSTask = loadPSTask.then([this, d3dContext](const std::vector<byte>& fileData) {
        DirectX::ThrowIfFailed(
            d3dContext->CreatePixelShader(
                &fileData[0],
                fileData.size(),
                nullptr,
                &m_pixelShader
            )
        );

        CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
        DirectX::ThrowIfFailed(
            d3dContext->CreateBuffer(
                &constantBufferDesc,
                nullptr,
                &m_constantBuffer
            )
        );
        });

    // Once both shaders are loaded, create the mesh.
    auto createCubeTask = (createPSTask && createVSTask).then([this, d3dContext]() {
        
        static const VertexPositionColor vertices[] =
        {
            {XMFLOAT3(-.5, .5, .5), XMFLOAT3(0.0f, 1.0f, 1.0f)},
            {XMFLOAT3(-.5, -.5, .5), XMFLOAT3(0.0f, 0.0f, 1.0f)},
            {XMFLOAT3(.5, .5, .5), XMFLOAT3(1.0f, 1.0f, 1.0f)},
            {XMFLOAT3(.5, -.5, .5), XMFLOAT3(1.0f, 0.0f, 1.0f)},
            {XMFLOAT3(.5, .5, -.5), XMFLOAT3(1.0f, 1.0f, 0.0f)},
            {XMFLOAT3(.5, -.5, -.5), XMFLOAT3(1.0f, 0.0f, 0.0f)},
            {XMFLOAT3(-.5, .5, -.5), XMFLOAT3(0.0f, 1.0f, 0.0f)},
            {XMFLOAT3(-.5, -.5, -.5), XMFLOAT3(0.0f, 0.0f, 0.0f)},
        };

        m_vertexCount = ARRAYSIZE(vertices);

        D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
        vertexBufferData.pSysMem = vertices;
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;
        CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
        DirectX::ThrowIfFailed(
            d3dContext->CreateBuffer(
                &vertexBufferDesc,
                &vertexBufferData,
                &m_vertexBuffer
            )
        );

        // Load mesh indices. Each trio of indices represents
        // a triangle to be rendered on the screen.
        // For example: 0,2,1 means that the vertices with indexes
        // 0, 2 and 1 from the vertex buffer compose the 
        // first triangle of this mesh.
        static const unsigned short cubeIndices[] =
        {
            1, 0, 3,  // Front
            0, 2, 3,

            6, 7, 5, // Back, needs to wind counter clockwise from front
            5, 4, 6,

            3, 2, 5, // Right
            2, 4, 5,

            7, 6, 1,  // Left
            6, 0, 1,

            0, 6, 2, // Top
            6, 4, 2,

            1, 7, 3, // Bottom
            7, 5, 3
        };

        m_indexCount = ARRAYSIZE(cubeIndices);

        D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
        indexBufferData.pSysMem = cubeIndices;
        indexBufferData.SysMemPitch = 0;
        indexBufferData.SysMemSlicePitch = 0;
        CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
        DirectX::ThrowIfFailed(
            d3dContext->CreateBuffer(
                &indexBufferDesc,
                &indexBufferData,
                &m_indexBuffer
            )
        );
        });

    // Once the cube is loaded, the object is ready to be rendered.
    createCubeTask.then([this]() {
        m_loadingComplete = true;
        });
}

// Rotate the 3D cube model a set amount of radians.
void GraphicsScenes::RenderCubeScene::rotate(float radians)
{
    // Prepare to pass the updated model matrix to the shader
    XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}
