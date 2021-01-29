//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "DeviceResources.h"
#include "DirectXMain.h"

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
        void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    private:
        std::shared_ptr<DirectX::DeviceResources> m_spDeviceResources;
        std::unique_ptr<DirectX::DirectXMain> m_spDirectxMain;
        std::unique_ptr<GraphicsScenes::IScene> m_spRenderScene;

        void swapChainPanel_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
    };
}
