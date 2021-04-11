#pragma once

#include "IRenderable.h"


namespace GraphicsScenes
{
    class CubeRenderable : public IRenderable
    {
    public:
        CubeRenderable();

        virtual void drawShape() override;

    private:
        void createDeviceDependentResources();

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

