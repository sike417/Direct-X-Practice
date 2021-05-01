#include "pch.h"
#include "CircleRenderableV2.h"
#include "DirectXHelper.h"


#define _USE_MATH_DEFINES
#include <math.h>

using namespace GraphicsScenes;
using namespace DirectX;

GraphicsScenes::CircleRenderableV2::CircleRenderableV2() : IRenderable()
{
    createDeviceDependentResources();
}

void GraphicsScenes::CircleRenderableV2::drawShape()
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
    //d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

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

void GraphicsScenes::CircleRenderableV2::UpdateNumberOfVertices(int desiredVertices)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    auto d3dContext = s_spDeviceResources->GetD3DDevice();

    auto circleVertices = calculateVerticesForCircle(DXResources::RoundUp(desiredVertices, quadrantsInCircle));

    m_vertexCount = circleVertices.size();

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = &circleVertices[0];
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPositionColor) * m_vertexCount, D3D11_BIND_VERTEX_BUFFER);
    DXResources::ThrowIfFailed(
        d3dContext->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            &m_vertexBuffer
        )
    );

    auto indices = calculateIndicesFromNumberOfVertices(circleVertices.size());

    m_indexCount = indices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &indices[0];
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * m_indexCount, D3D11_BIND_INDEX_BUFFER);
    DXResources::ThrowIfFailed(
        d3dContext->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            &m_indexBuffer
        )
    );

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

    OutputDebugString((L"duration: " + duration.ToString() + L"\n")->Data());
}

void GraphicsScenes::CircleRenderableV2::createDeviceDependentResources()
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


            CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
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
    auto createCircleTask = (createPSTask && createVSTask).then([this, d3dContext]() {

        static const int numOfVertices = 8;

        UpdateNumberOfVertices(numOfVertices);
        });

    // Once the cube is loaded, the object is ready to be rendered.
    createCircleTask.then([this]() {
        m_loadingComplete = true;
        });
}

std::vector<VertexPositionColor> GraphicsScenes::CircleRenderableV2::calculateVerticesForCircle(const int desiredVertices)
{
    static const XMFLOAT3 beginningPoint = XMFLOAT3(0, .5, 0);
    static const float float_tolerance = .0000001;
    static const float radius = .5;
    static const float circleTheta = 2 * M_PI;
    const float incrementTheta = circleTheta / desiredVertices;
    const int verticesPerQuadrant = desiredVertices / quadrantsInCircle;

    std::vector<VertexPositionColor> circleVertices;
    circleVertices.push_back(VertexPositionColor{ XMFLOAT3(0,0,0), getNextColor(Quadrant::CENTER, 0, 0) });

    int step = 0;

    for (float currentTheta = 0; step < desiredVertices; currentTheta += incrementTheta, step++)
    {
        // using the following two formula:
        // yp_2 = yp_1 - r * (1-cos(theta))
        // xp_2 = xp_1 + r * sin(theta)

        float newXPoint = beginningPoint.x + radius * sinf(currentTheta);
        float newYPoint = beginningPoint.y - radius * (1 - cosf(currentTheta));

        Quadrant currentQuadrant = Quadrant::FIRST_QUADRANT;
        if (step < verticesPerQuadrant)
        {
            currentQuadrant = Quadrant::FIRST_QUADRANT;
        }
        else if (step < verticesPerQuadrant * 2)
        {
            currentQuadrant = Quadrant::SECOND_QUADRANT;
        }
        else if (step < verticesPerQuadrant * 3)
        {
            currentQuadrant = Quadrant::THIRD_QUADRANT;
        }
        else
        {
            currentQuadrant = Quadrant::FOURTH_QUADRANT;
        }

        circleVertices.push_back(VertexPositionColor{ XMFLOAT3(newXPoint, newYPoint, 0), getNextColor(currentQuadrant, step % verticesPerQuadrant, verticesPerQuadrant) });
    }

    return circleVertices;
}

std::vector<unsigned short> GraphicsScenes::CircleRenderableV2::calculateIndicesFromNumberOfVertices(const int numberOfVertices)
{
    // several assumptions are being made.
    // 1. first vertice is the center point of the circle.
    // 2. the vertices are in clockwise order.
    std::vector<unsigned short> indices;

    // Load mesh indices. Each trio of indices represents
    // a triangle to be rendered on the screen.
    // For example: 0,2,1 means that the vertices with indexes
    // 0, 2 and 1 from the vertex buffer compose the 
    // first triangle of this mesh.
    for (int i = 1; i < numberOfVertices - 1; i++)
    {
        indices.push_back(0); // The center point of the circle.
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // Manually add the last triangle;
    indices.push_back(0);
    indices.push_back(numberOfVertices - 1);
    indices.push_back(1);

    return indices;
}

DirectX::XMFLOAT3 GraphicsScenes::CircleRenderableV2::getNextColor(Quadrant currentQuadrant, int currentStep, int maxSteps)
{
    static const XMFLOAT3 firstQuadrantColor(0, 0, 1);
    static const XMFLOAT3 secondQuadrantColor(0, 1, 1);
    static const XMFLOAT3 thirdQuadrantColor(0, 1, 0);
    static const XMFLOAT3 fourthQuadrantColor(1, 0, 0);

    XMFLOAT3 nextColor = defaultCircleColor;
    // Optional to return a single color if a color is set.
    switch (currentQuadrant)
    {
    case Quadrant::FIRST_QUADRANT:
        nextColor = DXResources::InterpolateColor(firstQuadrantColor, secondQuadrantColor, currentStep, maxSteps);
        break;
    case Quadrant::SECOND_QUADRANT:
        nextColor = DXResources::InterpolateColor(secondQuadrantColor, thirdQuadrantColor, currentStep, maxSteps);
        break;
    case Quadrant::THIRD_QUADRANT:
        nextColor = DXResources::InterpolateColor(thirdQuadrantColor, fourthQuadrantColor, currentStep, maxSteps);
        break;
    case Quadrant::FOURTH_QUADRANT:
        nextColor = DXResources::InterpolateColor(fourthQuadrantColor, firstQuadrantColor, currentStep, maxSteps);
        break;
    case Quadrant::CENTER:
        nextColor = DXResources::InterpolateFourEqualColor(firstQuadrantColor, secondQuadrantColor, thirdQuadrantColor, fourthQuadrantColor);
    default:
        break;
    }

    return nextColor;
}
