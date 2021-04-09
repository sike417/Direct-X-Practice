#include "pch.h"
#include "IScene.h"

using namespace GraphicsScenes;

std::shared_ptr<DXResources::DeviceResources> IScene::s_spDeviceResources = {};
std::shared_ptr<DXResources::GameCamera> IScene::s_spCamera = {};