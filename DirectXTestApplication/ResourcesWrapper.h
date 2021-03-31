#pragma once

#include "DeviceResources.h"
#include "GameCamera.h"
#include "DirectXMain.h"

#include <memory>


namespace DirectXTestApplication::Common
{
    class ResourcesWrapper
    {
    public:

        std::shared_ptr<DirectX::DeviceResources> M_spDeviceResources;
        std::shared_ptr<DirectX::GameCamera> M_spGameCamera;
        std::shared_ptr<DirectX::DirectXMain> M_spDirectxMain;
    };
}