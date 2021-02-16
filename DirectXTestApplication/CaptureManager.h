#pragma once

#include "DeviceResources.h"
#include <memory>

namespace MediaUtils
{
    class CaptureManager
    {
    public:
        CaptureManager(std::shared_ptr<DirectX::DeviceResources> spDeviceResource);

        void SaveImage();
        void SaveClip();
        void SetSaveLocation(const std::string& saveLocation);
        
    private:
        std::shared_ptr<DirectX::DeviceResources> m_spDeviceResource;
        Platform::String^ m_saveLocation;
    };
}
