#pragma once
#include <ShaderResources.h>
#include <DeviceResources.h>

namespace DirectX
{
    class GameCamera
    {
    public: 
        GameCamera(std::shared_ptr<DirectX::DeviceResources> spDeviceResource);
        void LockCamera();
        void UnlockCamera();
        bool IsCameraLocked();

        void SyncCameraWithWindowSize();

        auto GetMVPBuffer() { return m_constantBufferData; }

        void RotateCamera(float radians);

    private:
        std::shared_ptr<DirectX::DeviceResources> m_spDeviceResource;

        bool m_bIsCameraLocked;
        ModelViewProjectionConstantBuffer m_constantBufferData;
    };
}
