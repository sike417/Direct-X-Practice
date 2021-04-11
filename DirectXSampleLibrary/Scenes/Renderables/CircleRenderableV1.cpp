#include "pch.h"
#include "CircleRenderableV1.h"
#include "DirectXHelper.h"

using namespace GraphicsScenes;
using namespace DirectX;

GraphicsScenes::CircleRenderableV1::CircleRenderableV1() : IRenderable()
{
    createDeviceDependentResources();
}

void GraphicsScenes::CircleRenderableV1::drawShape()
{
    if (!m_loadingComplete)
    {
        return;
    }

    auto d3dContext = s_spDeviceResources->GetD3DDeviceContext();

    // load the individual primitative transform
    d3dContext->UpdateSubresource1(
        m_modelTransformConstantBuffer.Get(),
        0,
        nullptr,
        &m_modelTransform,
        0,
        0,
        0);

    auto vpMatrix = s_spCamera->GetCombinedVPBuffer().viewProjectionMatrix;

    //TODO: Generate this from the vertices.
    XMFLOAT3 centerPosition = XMFLOAT3(0, 0, 0);
    m_circleParameters.centerPosition = DXResources::CalculateNDCForPosition(centerPosition, m_modelTransform.model, vpMatrix);
    m_circleParameters.centerPosition = DXResources::ConvertNDCToPixelCoord(m_circleParameters.centerPosition, s_spDeviceResources->GetScreenViewport());

    XMFLOAT3 edgePosition = XMFLOAT3(.5, 0, 0);
    edgePosition = DXResources::CalculateNDCForPosition(edgePosition, m_modelTransform.model, vpMatrix);
    edgePosition = DXResources::ConvertNDCToPixelCoord(edgePosition, s_spDeviceResources->GetScreenViewport());

    m_circleParameters.radius = DXResources::CalculateDistanceBetweenPoints(m_circleParameters.centerPosition, edgePosition);

    d3dContext->UpdateSubresource1(
        m_circleParametersConstantBuffer.Get(),
        0,
        nullptr,
        &m_circleParameters,
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

    d3dContext->PSSetConstantBuffers1(
        0,
        1,
        m_circleParametersConstantBuffer.GetAddressOf(),
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

void GraphicsScenes::CircleRenderableV1::createDeviceDependentResources()
{
    auto d3dContext = s_spDeviceResources->GetD3DDevice();
    auto loadVSTask = DXResources::ReadDataAsync(L"DefaultVertexShader.cso");
    auto loadPSTask = DXResources::ReadDataAsync(L"RenderCircleV1PixelShader.cso");

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


            CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
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

        CD3D11_BUFFER_DESC constantBufferDesc(sizeof(CircleParametersConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
        DXResources::ThrowIfFailed(
            d3dContext->CreateBuffer(
                &constantBufferDesc,
                nullptr,
                &m_circleParametersConstantBuffer
            )
        );
        });

    // Once both shaders are loaded, create the mesh.
    auto createCircleTask = (createPSTask && createVSTask).then([this, d3dContext]() {

        static const VertexPositionColor vertices[] =
        {
            {XMFLOAT3(.5, .5, 0), XMFLOAT3(0.0f, 1.0f, 1.0f)},
            {XMFLOAT3(-.5, .5, 0), XMFLOAT3(0.0f, 0.0f, 1.0f)},
            {XMFLOAT3(.5, -.5, 0), XMFLOAT3(1.0f, 1.0f, 1.0f)},
            {XMFLOAT3(-.5, -.5, 0), XMFLOAT3(1.0f, 0.0f, 1.0f)},
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
        static const unsigned short squareIndices[] =
        {
            0, 2, 1,
            2, 3, 1
        };

        m_indexCount = ARRAYSIZE(squareIndices);

        D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
        indexBufferData.pSysMem = squareIndices;
        indexBufferData.SysMemPitch = 0;
        indexBufferData.SysMemSlicePitch = 0;
        CD3D11_BUFFER_DESC indexBufferDesc(sizeof(squareIndices), D3D11_BIND_INDEX_BUFFER);
        DXResources::ThrowIfFailed(
            d3dContext->CreateBuffer(
                &indexBufferDesc,
                &indexBufferData,
                &m_indexBuffer
            )
        );
        });

    // Once the cube is loaded, the object is ready to be rendered.
    createCircleTask.then([this]() {
        m_loadingComplete = true;
        });
}
