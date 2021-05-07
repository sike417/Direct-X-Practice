#include "pch.h"
#include "SphereRenderable.h"
#include "DirectXHelper.h"

using namespace DirectX;
using namespace GraphicsScenes;

GraphicsScenes::SphereRenderable::SphereRenderable() : IRenderable()
{
    createDeviceDependentResources();
}

void GraphicsScenes::SphereRenderable::drawShape()
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
    //d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

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

void GraphicsScenes::SphereRenderable::createDeviceDependentResources()
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
    auto createSphereTask = (createPSTask && createVSTask).then([this, d3dContext]() {

        static const int numOfVertices = 48;

        updateNumberOfVertices(numOfVertices);
        });

    // Once the cube is loaded, the object is ready to be rendered.
    createSphereTask.then([this]() {
        m_loadingComplete = true;
        });
}

void GraphicsScenes::SphereRenderable::updateNumberOfVertices(int desiredVertices)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    auto d3dContext = s_spDeviceResources->GetD3DDevice();

    int numberOfInnerCircles = 0;

    auto circleVertices = calculateVerticesForCircle(desiredVertices, numberOfInnerCircles);

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

    auto indices = calculateIndicesFromNumberOfVertices(desiredVertices, numberOfInnerCircles);

    m_indexCount = indices.size();

    if (m_indexCount == 0)
    {
        return;
    }

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
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    OutputDebugString((L"duration: " + duration.ToString() + L"\n")->Data());
}

std::vector<VertexPositionColor> GraphicsScenes::SphereRenderable::calculateVerticesForCircle(const int desiredVertices, int& numberOfInnerCircles)
{
    numberOfInnerCircles = 0;

    XMFLOAT3 beginningPoint = XMFLOAT3(0, .5, 0);
    static const float float_tolerance = .0000001;
    static const float radius = .5;
    const float circleIncrementTheta = XM_2PI / (desiredVertices + 1);
    const int clampedVertices = DXResources::RoundUp(desiredVertices, quadrantsInCircle);
    const float halfCircleIncrementTheta = M_PI / (clampedVertices / 2);


    std::vector<VertexPositionColor> SphereVertices;

    // Points are plotted around the corresponding axes.
    // the points for the zAxesCircleVertices will have the same zPoint, but vary in x and y. 
    std::vector<XMFLOAT3> zAxesCircleVertices;

    for (float currentTheta = 0; currentTheta <= M_PI + float_tolerance; currentTheta += halfCircleIncrementTheta)
    {
        // using the following two formula:
        // yp_2 = yp_1 - r * (1-cos(theta))
        // xp_2 = xp_1 + r * sin(theta)

        float newXPoint = beginningPoint.x + radius * sinf(currentTheta);
        float newYPoint = beginningPoint.y - radius * (1 - cosf(currentTheta));

        zAxesCircleVertices.push_back(XMFLOAT3(newXPoint, newYPoint, 0));
    }

    SphereVertices.push_back(VertexPositionColor{ zAxesCircleVertices.front(), defaultCircleColor });
    
    // ignore the first and last vertices as those are the top and bottom of the sphere
    for (int i = 1; i < zAxesCircleVertices.size() - 1; i++)
    {
        const float currentCircleRadius = zAxesCircleVertices[i].x;

        numberOfInnerCircles++;
        auto currentBeginningPoint = XMFLOAT3{ 0, 0, zAxesCircleVertices[i].x };
        int currentVerticesCount = 0;
        for (float currentTheta = 0; currentVerticesCount < desiredVertices; currentTheta += circleIncrementTheta, currentVerticesCount++)
        {
            // using the following two formula:
            // yp_2 = yp_1 - r * (1-cos(theta))
            // xp_2 = xp_1 + r * sin(theta)

            float newXPoint = currentBeginningPoint.x + currentCircleRadius * sinf(currentTheta);
            float newZPoint = currentBeginningPoint.z - currentCircleRadius * (1 - cosf(currentTheta));

            SphereVertices.push_back(VertexPositionColor{ XMFLOAT3(newXPoint, zAxesCircleVertices[i].y, newZPoint), defaultCircleColor });
        }
    }

    SphereVertices.push_back(VertexPositionColor{ zAxesCircleVertices[zAxesCircleVertices.size() - 1], defaultCircleColor });

    return SphereVertices;
}

std::vector<unsigned short> GraphicsScenes::SphereRenderable::calculateIndicesFromNumberOfVertices(const int numberOfVerticesPerCircle, const int numberOfInnerCircles)
{

    int numberOfVertices = numberOfVerticesPerCircle * numberOfInnerCircles + 2;

    // several assumptions are being made.
    // 1. first vertice is the top of the sphere, and last vertices is the bottom.
    std::vector<unsigned short> indices;

    // Start with the top
    for (int i = 1; i < numberOfVerticesPerCircle; i++)
    {
        indices.push_back(0);   // top of the sphere.
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // add the final triangle for the top
    indices.push_back(0);
    indices.push_back(numberOfVerticesPerCircle);
    indices.push_back(1);

    for (int currentInnerCircle = 0; currentInnerCircle < numberOfInnerCircles - 1; currentInnerCircle++)
    {
        for (int currentVertex = 0; currentVertex < numberOfVerticesPerCircle - 1; currentVertex++)
        {
            auto adjustedIndex = currentInnerCircle * numberOfVerticesPerCircle + currentVertex + 1;
            auto firstConnectedVertex = adjustedIndex + numberOfVerticesPerCircle;
            indices.push_back(adjustedIndex);
            indices.push_back(firstConnectedVertex);
            indices.push_back(firstConnectedVertex + 1);

            indices.push_back(adjustedIndex);
            indices.push_back(firstConnectedVertex + 1);
            indices.push_back(adjustedIndex + 1);
        }

        auto firstIndexInCircle = currentInnerCircle * numberOfVerticesPerCircle + 1;
        auto lastIndexInCircle = firstIndexInCircle + numberOfVerticesPerCircle - 1;
        auto firstConnectedIndex = lastIndexInCircle + numberOfVerticesPerCircle;

        indices.push_back(lastIndexInCircle);
        indices.push_back(firstConnectedIndex);
        indices.push_back(lastIndexInCircle + 1);

        indices.push_back(lastIndexInCircle);
        indices.push_back(lastIndexInCircle + 1);
        indices.push_back(firstIndexInCircle);
    }

    // Now do the bottom
    for (int i = numberOfVertices - (numberOfVerticesPerCircle + 1); i < numberOfVertices - 1; i++)
    {
        indices.push_back(numberOfVertices - 1);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    indices.push_back(numberOfVertices - 1);
    indices.push_back(numberOfVertices - (numberOfVerticesPerCircle + 1));
    indices.push_back(numberOfVertices - 2);

    return indices;
}

DirectX::XMFLOAT3 GraphicsScenes::SphereRenderable::getNextColor(Quadrant currentQuadrant, int currentStep, int maxSteps)
{
    return defaultCircleColor;
}
