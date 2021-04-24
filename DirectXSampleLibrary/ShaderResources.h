#pragma once
#include "pch.h"

// Constant buffer used to send MVP matrices to the vertex shader.
struct ViewProjectionConstantBuffer
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
};

struct CombinedViewProjectionConstantBuffer
{
    DirectX::XMFLOAT4X4 viewProjectionMatrix;
};

struct CircleParametersConstantBuffer
{
    DirectX::XMFLOAT3 centerPosition;
    float radius;
};

struct VertexPositionColor
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
};