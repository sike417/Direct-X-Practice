#pragma once
#include "IRenderable.h"

namespace GraphicsScenes
{
    class CircleRenderableV1 : public IRenderable
    {
    public:
        CircleRenderableV1();

        // Inherited via IRenderable
        virtual void drawShape() override;

    private:
        void createDeviceDependentResources();

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_indexBuffer;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_pixelShader;

        CircleParametersConstantBuffer          m_circleParameters;
        Microsoft::WRL::ComPtr<ID3D11Buffer>    m_circleParametersConstantBuffer;


        uint32 m_indexCount;
        bool m_loadingComplete;
        int m_vertexCount;
    };
}