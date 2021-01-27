//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace DirectXTestApplication;
using namespace DirectX;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

MainPage::MainPage()
{
    InitializeComponent();

    m_spDeviceResources = std::make_shared<DeviceResources>();
    m_spDeviceResources->SetSwapChainPanel(swapChainPanel);

    m_spDirectxMain = std::make_unique<DirectXMain>(m_spDeviceResources);
}


void DirectXTestApplication::MainPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}

void DirectXTestApplication::MainPage::swapChainPanel_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
    //critical_section::scoped_lock lock(m_main->GetCriticalSection());
    m_spDeviceResources->SetLogicalSize(e->NewSize);
    //m_main->CreateWindowSizeDependentResources();
}
