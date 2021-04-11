#include "pch.h"
#include "IRenderable.h"

using namespace GraphicsScenes;

std::shared_ptr <DXResources::DeviceResources> IRenderable::s_spDeviceResources = {};
std::shared_ptr <DXResources::GameCamera> IRenderable::s_spCamera = {};