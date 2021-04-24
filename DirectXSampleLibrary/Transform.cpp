#include "pch.h"
#include "Transform.h"

using namespace DirectX;

DXResources::Transform::Transform()
    : m_location()
    , m_size(1, 1, 1)
    , m_rotationInRadians(0.0f)
{
}

void DXResources::Transform::SetPosition(float xPos, float yPos, float zPos)
{
    m_location.X = xPos;
    m_location.Y = yPos;
    m_location.Z = zPos;
}

void DXResources::Transform::SetSize(float width, float height, float depth)
{
    m_size.X = width;
    m_size.Y = height;
    m_size.Z = depth;
}

void DXResources::Transform::SetRotation(float radians)
{
    m_rotationInRadians = radians;
}

XMFLOAT4X4 DXResources::Transform::GetTransform()
{
    XMFLOAT4X4 transform;
    XMMATRIX translationMatrix = XMMatrixTranslation(m_location.X, m_location.Y, m_location.Z);
    XMMATRIX rotationMatrix = XMMatrixRotationY(m_rotationInRadians);

    XMStoreFloat4x4(&transform, XMMatrixMultiply(XMMatrixTranspose(translationMatrix), XMMatrixTranspose(rotationMatrix)));
    return transform;
}
