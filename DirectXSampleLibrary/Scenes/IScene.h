#pragma once

#include "DeviceResources.h"
#include "IRenderable.h"
#include "GameCamera.h"

#include <vector>

namespace GraphicsScenes
{
    class IScene
    {
    public:
        virtual ~IScene()
        {
            for (int i = 0; i < m_vScenePrimitives.size(); i++)
            {
                delete m_vScenePrimitives[i];
            }

            m_vScenePrimitives.clear();
        }

        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void ActivateScene() = 0;

        static void SetSharedSceneResources(std::shared_ptr<DXResources::DeviceResources> spDeviceResource, std::shared_ptr<DXResources::GameCamera> spCamera)
        {
            s_spDeviceResources = spDeviceResource;
            s_spCamera = spCamera;
        }

    protected:
        static std::shared_ptr<DXResources::DeviceResources> s_spDeviceResources;
        static std::shared_ptr<DXResources::GameCamera> s_spCamera;

        std::vector<IRenderable*> m_vScenePrimitives;
    };
}