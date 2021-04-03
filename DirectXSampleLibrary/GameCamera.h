#pragma once
#include <ShaderResources.h>
#include <DeviceResources.h>

namespace DXResources
{
    class GameCamera
    {
    public: 
        GameCamera(std::shared_ptr<DXResources::DeviceResources> spDeviceResource);
        void LockCamera();
        void UnlockCamera();
        bool IsCameraLocked();

        void SyncCameraWithWindowSize();

        auto GetMVPBuffer() { return m_constantBufferData; }

        void RotateCamera(float radians);

    private:
        std::shared_ptr<DXResources::DeviceResources> m_spDeviceResource;

        bool m_bIsCameraLocked;
        ModelViewProjectionConstantBuffer m_constantBufferData;
    };
}
