#pragma once

#include "IRenderable.h"


namespace GraphicsScenes
{
    class SphereRenderable : public IRenderable
    {
    public:
        SphereRenderable();

        virtual void drawShape() override;

    private:

        enum class Quadrant
        {
            FIRST_QUADRANT,
            SECOND_QUADRANT,
            THIRD_QUADRANT,
            FOURTH_QUADRANT,
            CENTER
        };

        void createDeviceDependentResources();
        // Note: desiredVertices needs to be a multiple of four, otherwise the function will clamp it to the next highest multiple of four.
        void updateNumberOfVertices(int desiredVertices);
        std::vector<VertexPositionColor> calculateVerticesForCircle(const int desiredVertices, int& numberOfInnerCircles);
        std::vector<unsigned short> calculateIndicesFromNumberOfVertices(const int numberOfVertices, const int numberOfInnerCircles);
        DirectX::XMFLOAT3 getNextColor(Quadrant currentQuadrant, int currentStep, int maxSteps);

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_indexBuffer;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_pixelShader;

        uint32 m_indexCount;
        bool m_loadingComplete;
        int m_vertexCount;

        const DirectX::XMFLOAT3 defaultCircleColor = XMFLOAT3(1.0f, 0.0f, 0.0f);
        const int quadrantsInCircle = 4;

    };
}

