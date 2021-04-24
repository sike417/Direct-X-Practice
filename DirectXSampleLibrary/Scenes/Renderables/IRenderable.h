#pragma once
#include "ShaderResources.h"
#include "DeviceResources.h"
#include "GameCamera.h"
#include "Transform.h"

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
        }

        static void SetSharedRenderableResources(std::shared_ptr<DXResources::DeviceResources> spDeviceResource, std::shared_ptr<DXResources::GameCamera> spCamera)
        {
            s_spDeviceResources = spDeviceResource;
            s_spCamera = spCamera;
        }

        virtual void drawShape() = 0;

        DXResources::Transform& GetTransform()
        {
            return m_transform;
        }

    protected:
        Microsoft::WRL::ComPtr<ID3D11Buffer>    m_modelTransformConstantBuffer;
        DXResources::Transform                  m_transform;

        static std::shared_ptr<DXResources::DeviceResources> s_spDeviceResources;
        static std::shared_ptr<DXResources::GameCamera> s_spCamera;
    };
}
