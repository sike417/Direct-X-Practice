#pragma once
#include "ShaderResources.h"
#include "DeviceResources.h"
#include "GameCamera.h"

#include <memory>

namespace GraphicsScenes
{
    class IRenderable
    {
    public:
        IRenderable()
        {
            if (s_spDeviceResources == nullptr || s_spCamera == nullptr)
            {
                throw ref new FailureException();
            }

            DirectX::XMStoreFloat4x4(&m_modelTransform.model, DirectX::XMMatrixIdentity());
        }

        static void SetSharedRenderableResources(std::shared_ptr<DXResources::DeviceResources> spDeviceResource, std::shared_ptr<DXResources::GameCamera> spCamera)
        {
            s_spDeviceResources = spDeviceResource;
            s_spCamera = spCamera;
        }

        virtual void drawShape() = 0;

        void SetModelTransform(DirectX::XMFLOAT4X4 model)
        {
            m_modelTransform.model = model;
        }

    protected:
        Microsoft::WRL::ComPtr<ID3D11Buffer>    m_modelTransformConstantBuffer;
        ModelConstantBuffer                     m_modelTransform;

        static std::shared_ptr<DXResources::DeviceResources> s_spDeviceResources;
        static std::shared_ptr<DXResources::GameCamera> s_spCamera;
    };
}
