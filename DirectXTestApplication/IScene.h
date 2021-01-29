#pragma once

namespace GraphicsScenes
{
    class IScene
    {
    public:
        virtual ~IScene() = default;

        virtual void Update() = 0;
        virtual void Render() = 0;
    };
}