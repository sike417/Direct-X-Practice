#pragma once

#include "DeviceResources.h"
#include "GameCamera.h"
#include "DirectXMain.h"

#include <memory>


namespace DXResources
{
    class ResourcesWrapper
    {
    public:

        std::shared_ptr<DXResources::DeviceResources> M_spDeviceResources;
        std::shared_ptr<DXResources::GameCamera> M_spGameCamera;
        std::shared_ptr<DXResources::DirectXMain> M_spDirectxMain;
    };
}