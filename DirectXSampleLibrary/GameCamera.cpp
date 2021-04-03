#include "pch.h"
#include "GameCamera.h"

using namespace Windows::Foundation;

using namespace DXResources;
using namespace DirectX;

DXResources::GameCamera::GameCamera(std::shared_ptr<DXResources::DeviceResources> spDeviceResource)
    : m_spDeviceResource(spDeviceResource)
    , m_bIsCameraLocked(false)
{
    // Set all mvp elements to identity matrix
    XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixIdentity());
    XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixIdentity());
    XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixIdentity());
}

void DXResources::GameCamera::LockCamera()
{
    m_bIsCameraLocked = true;
}

void DXResources::GameCamera::UnlockCamera()
{
    m_bIsCameraLocked = false;
}

bool DXResources::GameCamera::IsCameraLocked()
{
    return m_bIsCameraLocked;
}

void DXResources::GameCamera::SyncCameraWithWindowSize()
{
   Size outputSize = m_spDeviceResource->GetOutputSize();
   float aspectRatio = outputSize.Width / outputSize.Height;
   float fovAngleY = 70.0f * XM_PI / 180.0f;

   // This is a simple example of change that can be made when the app is in
   // portrait or snapped view.
   if (aspectRatio < 1.0f)
   {
       fovAngleY *= 2.0f;
   }

   // Note that the OrientationTransform3D matrix is post-multiplied here
   // in order to correctly orient the scene to match the display orientation.
   // This post-multiplication step is required for any draw calls that are
   // made to the swap chain render target. For draw calls to other targets,
   // this transform should not be applied.

   // This sample makes use of a right-handed coordinate system using row-major matrices.
   XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
       fovAngleY,
       aspectRatio,
       0.01f,
       100.0f
   );

   //XMFLOAT4X4 orientation = m_spDeviceResources->GetOrientationTransform3D();

   XMMATRIX orientationMatrix = XMMatrixIdentity();// XMLoadFloat4x4(&orientation);

   XMStoreFloat4x4(
       &m_constantBufferData.projection,
       XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
   );

   // TODO: Should this be the responsibility of the individual scenes, not the camera.
   // TODO: Why is the view used to do this, not the model?

   // Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
   static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
   static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
   static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

   XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

void DXResources::GameCamera::RotateCamera(float radians)
{
    // Prepare to pass the updated model matrix to the shader
    XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}
