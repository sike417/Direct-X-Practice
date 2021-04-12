#pragma once
#include "IScene.h"

namespace GraphicsScenes
{
    class RenderCircleSceneV2 : public IScene
    {
    public:
        RenderCircleSceneV2();

        // Inherited via IScene
        virtual void Update() override;
        virtual void Render() override;
        virtual void ActivateScene() override;

    private:
        void createDeviceDependentResources();

    private:

        // Direct3D resources for circle geometry.
        Microsoft::WRL::ComPtr<ID3D11Buffer>      m_vpConstantBuffer;

        float m_fTotalTime;
        int m_iCurrentVertexCount;
    };
}

