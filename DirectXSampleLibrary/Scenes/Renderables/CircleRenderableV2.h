#pragma once
#include "IRenderable.h"

namespace GraphicsScenes
{
    class CircleRenderableV2 : public IRenderable
    {
    public:
        CircleRenderableV2();

        // Inherited via IRenderable
        virtual void drawShape() override;

        // desiredVertices should be a multiple of four, otherwise this function will round it up to the nearest multiple.
        void UpdateNumberOfVertices(int desiredVertices);

        int GetCurrentNumberOfVertices()
        {
            return m_vertexCount - 1;
        }

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
        std::vector<VertexPositionColor> calculateVerticesForCircle(const int desiredVertices);
        std::vector<unsigned short> calculateIndicesFromNumberOfVertices(const int numberOfVertices);
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