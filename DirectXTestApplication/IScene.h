#pragma once

namespace GraphicsScenes
{
    class IScene
    {
    public:
        virtual ~IScene() = default;

        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void StartTracking() = 0;
        virtual void TrackingUpdate(float positionX) = 0;
        virtual void StopTracking() = 0;
        virtual void CreateWindowSizeDependentResources() = 0;
    };
}