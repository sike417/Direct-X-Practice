#pragma once

namespace DXResources
{
    struct Point3D
    {
        Point3D()
        {
            X = Y = Z = 0;
        }

        Point3D(float x, float y, float z)
            : X(x)
            , Y(y)
            , Z(z)
        {

        }
    public:
        float X;
        float Y;
        float Z;
    };

    class Transform
    {
    public:
        Transform();

        void SetPosition(float xPos, float yPos, float zPos);
        void SetSize(float width, float height, float depth);
        void SetRotation(float radians);

        DirectX::XMFLOAT4X4 GetTransform();

        Point3D GetPosition()
        {
            return m_location;
        }

        Point3D GetSize()
        {
            return m_size;
        }

        float GetXPos()
        {
            return m_location.X;
        }

        float GetYPos()
        {
            return m_location.Y;
        }

        float GetZPos()
        {
            return m_location.Z;
        }

        float GetWidth()
        {
            return m_size.X;
        }

        float GetHeight()
        {
            return m_size.Y;
        }

        float GetDepth()
        {
            return m_size.Z;
        }

    private:
        Point3D m_location;
        Point3D m_size;
        
        float m_rotationInRadians;
    };
}

