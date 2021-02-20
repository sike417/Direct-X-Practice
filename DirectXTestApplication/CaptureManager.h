#pragma once

#include "DeviceResources.h"
#include "DirectXMain.h"
#include "FFMPEGEncoder.h"

namespace MediaUtils
{
    class CaptureManager
    {
    public:
        CaptureManager(std::shared_ptr<DirectX::DeviceResources> spDeviceResource, std::shared_ptr<DirectX::DirectXMain> spDirectXMain);

        void SaveImage();
        void SaveClip();
        void SetSaveLocation(const std::string& saveLocation);

    private:
        std::shared_ptr<DirectX::DeviceResources> m_spDeviceResource;
        std::shared_ptr<DirectX::DirectXMain> m_spDirectXMain;
        Platform::String^ m_saveLocation;

        FFMPEGEncoder* m_pCurrentEncoder;
        int m_iCurrentFrameCount;
        Concurrency::critical_section m_pCaptureCriticalSection;
    };
}

