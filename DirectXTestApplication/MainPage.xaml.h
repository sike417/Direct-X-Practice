//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "DeviceResources.h"
#include "DirectXMain.h"
#include "GameCamera.h"
#include "CaptureManager.h"

namespace DirectXTestApplication
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();

    private:
        std::shared_ptr<DirectX::DeviceResources> m_spDeviceResources;
        std::shared_ptr<DirectX::GameCamera> m_spGameCamera;
        std::shared_ptr<DirectX::DirectXMain> m_spDirectxMain;
        std::unique_ptr<GraphicsScenes::IScene> m_spRenderScene;
        std::unique_ptr<MediaUtils::CaptureManager> m_spCaptureManager;

        // Track our independent input on a background worker thread.
        Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
        Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

        // Independent input handling functions.
        void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

        void swapChainPanel_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        void captureScreenImage(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void toggleClipCapture(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    };
}
