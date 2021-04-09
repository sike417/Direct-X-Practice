//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "ResourcesWrapper.h"

using namespace DirectXSampleLibrary;
using namespace DirectXSampleLibrary::View;
using namespace DirectXSampleLibrary::ViewModel;
using namespace DXResources;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::System::Threading;

MainPage::MainPage()
{
    InitializeComponent();

    m_spDeviceResources = std::make_shared<DeviceResources>();
    m_spDeviceResources->SetSwapChainPanel(swapChainPanel);

    m_spDirectxMain = std::make_shared<DirectXMain>(m_spDeviceResources);

    m_spGameCamera = std::make_shared<GameCamera>(m_spDeviceResources);
    m_spCaptureManager = std::make_unique<MediaUtils::CaptureManager>(m_spDeviceResources, m_spDirectxMain);

    // Register our SwapChainPanel to get independent input pointer events
    auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^)
        {
            // The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
            m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
                Windows::UI::Core::CoreInputDeviceTypes::Mouse |
                Windows::UI::Core::CoreInputDeviceTypes::Touch |
                Windows::UI::Core::CoreInputDeviceTypes::Pen
            );

            // Register for pointer events, which will be raised on the background thread.
            m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerPressed);
            m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerMoved);
            m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerReleased);

            // Begin processing input messages as they're delivered.
            m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
        });

    // Run task on a dedicated high priority background thread.
    m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

    auto pResources = new ResourcesWrapper();
    pResources->M_spDirectxMain = m_spDirectxMain;
    pResources->M_spDeviceResources = m_spDeviceResources;
    pResources->M_spGameCamera = m_spGameCamera;
    ScenePicker = ref new ScenePickerViewModel(pResources);

    m_spDirectxMain->StartRenderLoop();
}

void MainPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
    // When the pointer is pressed begin tracking the pointer movement.
    m_spGameCamera->LockCamera();
}

void MainPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
    if (m_spGameCamera->IsCameraLocked())
    {
        float radians = DirectX::XM_2PI * 2.0f * e->CurrentPoint->Position.X / m_spDeviceResources->GetOutputSize().Width;
        //m_spGameCamera->RotateCamera(radians);
    }
}

void MainPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
    // Stop tracking pointer movement when the pointer is released.
    m_spGameCamera->UnlockCamera();
}

void MainPage::swapChainPanel_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
    Concurrency::critical_section::scoped_lock lock(m_spDirectxMain->GetCriticalSection());
    m_spDeviceResources->SetLogicalSize(e->NewSize);
    m_spGameCamera->SyncCameraWithWindowSize();
}


void MainPage::captureScreenImage(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    m_spCaptureManager->SaveImage();
}


void MainPage::toggleClipCapture(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    m_spCaptureManager->SaveClip();
}
