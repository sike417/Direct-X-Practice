#pragma once
#include <ShaderResources.h>
#include <DeviceResources.h>

using namespace DirectX;

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

        // eye is where the eye is located at
        // at is the point the eye is looking at
        // up is the direction of the up vector
        void SetCameraView(XMVECTORF32 eye, XMVECTORF32 at, XMVECTORF32 up);

        CombinedViewProjectionConstantBuffer GetCombinedVPBuffer();

        auto GetVPBuffer() { return m_constantBufferData; }

    private:
        void initializeDefaultState();

    private:
        std::shared_ptr<DXResources::DeviceResources> m_spDeviceResource;

        bool m_bIsCameraLocked;
        ViewProjectionConstantBuffer m_constantBufferData;
    };
}
