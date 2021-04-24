#include "pch.h"
#include "CubeRenderable.h"
#include "DirectXHelper.h"

using namespace DirectX;
using namespace GraphicsScenes;

GraphicsScenes::CubeRenderable::CubeRenderable() : IRenderable()
{
    createDeviceDependentResources();
}

void GraphicsScenes::CubeRenderable::drawShape()
{
    if (!m_loadingComplete)
    {
        return;
    }

    auto d3dContext = s_spDeviceResources->GetD3DDeviceContext();

    DirectX::XMFLOAT4X4 model = m_transform.GetTransform();

    // load the individual primitative transform
    d3dContext->UpdateSubresource1(
        m_modelTransformConstantBuffer.Get(),
        0,
        nullptr,
        &model,
        0,
        0,
        0);

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
        1,
        1,
        m_modelTransformConstantBuffer.GetAddressOf(),
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

void GraphicsScenes::CubeRenderable::createDeviceDependentResources()
{
    auto d3dContext = s_spDeviceResources->GetD3DDevice();
    auto loadVSTask = DXResources::ReadDataAsync(L"DefaultVertexShader.cso");
    auto loadPSTask = DXResources::ReadDataAsync(L"DefaultPixelShader.cso");

    auto createVSTask = loadVSTask.then([this, d3dContext](const std::vector<byte>& fileData)
        {
            DXResources::ThrowIfFailed(
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

            DXResources::ThrowIfFailed(
                d3dContext->CreateInputLayout(
                    vertexDesc,
                    ARRAYSIZE(vertexDesc),
                    &fileData[0],
                    fileData.size(),
                    &m_inputLayout
                ));

            CD3D11_BUFFER_DESC constantBufferDesc(sizeof(DirectX::XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
            DXResources::ThrowIfFailed(
                d3dContext->CreateBuffer(
                    &constantBufferDesc,
                    nullptr,
                    &m_modelTransformConstantBuffer
                )
            );
        });

    auto createPSTask = loadPSTask.then([this, d3dContext](const std::vector<byte>& fileData) {
        DXResources::ThrowIfFailed(
            d3dContext->CreatePixelShader(
                &fileData[0],
                fileData.size(),
                nullptr,
                &m_pixelShader
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
        DXResources::ThrowIfFailed(
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
        DXResources::ThrowIfFailed(
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
