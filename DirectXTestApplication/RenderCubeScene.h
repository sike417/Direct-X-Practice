#pragma once
#include "IScene.h"
#include "DeviceResources.h"

namespace GraphicsScenes
{
    class RenderCubeScene : public IScene
    {
    public:
        RenderCubeScene(std::shared_ptr<DirectX::DeviceResources> spDeviceResource);

        // Inherited via IScene
        virtual void Update() override;
        virtual void Render() override;

    private:
        void createDeviceDependentResources();

    private:
        std::shared_ptr<DirectX::DeviceResources> m_spDeviceResources;

        // Direct3D resources for cube geometry.
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_pixelShader;
        //Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;


        bool m_loadingComplete;
        int m_vertexCount;
    };
}

