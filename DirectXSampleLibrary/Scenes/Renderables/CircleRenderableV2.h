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

        void UpdateNumberOfVertices(const int verticesPerQuadrant);

    private:
        void createDeviceDependentResources();
        std::vector<VertexPositionColor> calculateVerticesForCircle(const int verticesInQuadrant);
        std::vector<unsigned short> calculateIndicesFromNumberOfVertices(const int numberOfVertices);

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_indexBuffer;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_pixelShader;


        uint32 m_indexCount;
        bool m_loadingComplete;
        int m_vertexCount;
    };
}