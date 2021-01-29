#include "pch.h"
#include "RenderCubeScene.h"
#include "DirectXHelper.h"

using namespace GraphicsScenes;
using namespace DirectX;

GraphicsScenes::RenderCubeScene::RenderCubeScene(std::shared_ptr<DirectX::DeviceResources> spDeviceResource)
    : m_spDeviceResources(spDeviceResource)
    , m_loadingComplete(false)
    , m_vertexCount(0)
{
    createDeviceDependentResources();
}

void GraphicsScenes::RenderCubeScene::Update()
{
    if (!m_loadingComplete)
    {
        return;
    }
}

void GraphicsScenes::RenderCubeScene::Render()
{
    if (!m_loadingComplete)
    {
        return;
    }

    auto d3dContext = m_spDeviceResources->GetD3DDeviceContext();

    // Each vertex is one instance of the VertexPositionColor struct.
    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(
        0,
        1,
        m_vertexBuffer.GetAddressOf(),
        &stride,
        &offset
    );

    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

    d3dContext->IASetInputLayout(m_inputLayout.Get());

    // Attach our vertex shader.
    d3dContext->VSSetShader(
        m_vertexShader.Get(),
        nullptr,
        0
    );

    // Attach our pixel shader.
    d3dContext->PSSetShader(
        m_pixelShader.Get(),
        nullptr,
        0
    );

    d3dContext->Draw(m_vertexCount, 0);
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
                //{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

        //CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
        //DX::ThrowIfFailed(
        //    m_deviceResources->GetD3DDevice()->CreateBuffer(
        //        &constantBufferDesc,
        //        nullptr,
        //        &m_constantBuffer
        //    )
        //);
        });

    // Once both shaders are loaded, create the mesh.
    auto createCubeTask = (createPSTask && createVSTask).then([this, d3dContext]() {

        static const XMFLOAT3 vertices[] =
        {
            {XMFLOAT3(-.5, -.5, 0)},
            {XMFLOAT3(-.5, .5, 0)},
            {XMFLOAT3(.5, -.5, 0)},
        };
        m_vertexCount = ARRAYSIZE(vertices);
        // Load mesh vertices. Each vertex has a position and a color.
        //static const VertexPositionColor cubeVertices[] =
        //{
        //    {XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
        //    {XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
        //    {XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
        //    {XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
        //    {XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
        //    {XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
        //    {XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
        //    {XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
        //};

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
        //static const unsigned short cubeIndices[] =
        //{
        //    0,2,1, // -x
        //    1,2,3,

        //    4,5,6, // +x
        //    5,7,6,

        //    0,1,5, // -y
        //    0,5,4,

        //    2,6,7, // +y
        //    2,7,3,

        //    0,4,6, // -z
        //    0,6,2,

        //    1,3,7, // +z
        //    1,7,5,
        //};

        //m_indexCount = ARRAYSIZE(cubeIndices);

        //D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
        //indexBufferData.pSysMem = cubeIndices;
        //indexBufferData.SysMemPitch = 0;
        //indexBufferData.SysMemSlicePitch = 0;
        //CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
        //DX::ThrowIfFailed(
        //    m_deviceResources->GetD3DDevice()->CreateBuffer(
        //        &indexBufferDesc,
        //        &indexBufferData,
        //        &m_indexBuffer
        //    )
        //);
        });

    // Once the cube is loaded, the object is ready to be rendered.
    createCubeTask.then([this]() {
        m_loadingComplete = true;
        });
}
 