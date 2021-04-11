#pragma once
#include "IScene.h"
#include <ShaderResources.h>

namespace GraphicsScenes
{
    class RenderCubeScene : public IScene
    {
    public:
        RenderCubeScene();

        // Inherited via IScene
        virtual void Update() override;
        virtual void Render() override;
        virtual void ActivateScene() override;


    private:
        void createDeviceDependentResources();

    private:

        // Direct3D resources for cube geometry.
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_vpConstantBuffer;

        float m_fTotalTime;
    };
}

