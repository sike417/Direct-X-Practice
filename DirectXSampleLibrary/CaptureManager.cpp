#include "pch.h"
#include "CaptureManager.h"
#include "DirectXHelper.h"

using namespace Windows::System::Threading;
using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;
using namespace Microsoft::WRL;
using namespace concurrency;

MediaUtils::CaptureManager::CaptureManager(std::shared_ptr<DXResources::DeviceResources> spDeviceResource, std::shared_ptr<DXResources::DirectXMain> spDirectXMain)
    : m_spDeviceResource(spDeviceResource)
    , m_spDirectXMain(spDirectXMain)
    , m_saveLocation("")
    , m_pCurrentEncoder(nullptr)
    , m_iCurrentFrameCount(0)
{
}

void MediaUtils::CaptureManager::SaveImage()
{
    UINT pixelWidth, pixelHeight;
    size_t bufferLength;
    std::unique_ptr<uint8_t[]> pixelBuffer;

    {
        Concurrency::critical_section::scoped_lock lock(m_spDirectXMain->GetCriticalSection());
        pixelBuffer.reset(m_spDeviceResource->GetLastRenderedFrame(pixelWidth, pixelHeight, bufferLength));
    }

    Platform::Array<unsigned char>^ managedPixelBuffer = ref new Platform::Array<unsigned char>(&pixelBuffer[0], bufferLength);

    // For now use the local app folder, eventually allow user customizable.
    auto storageFolder = ApplicationData::Current->LocalFolder;

    create_task(storageFolder->CreateFileAsync("data.png", CreationCollisionOption::ReplaceExisting))
        .then([](StorageFile^ file) {
        return file->OpenAsync(FileAccessMode::ReadWrite);
            }).then([](IRandomAccessStream^ stream) {
                return BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId, stream);
                }).then([pixelWidth, pixelHeight, managedPixelBuffer](BitmapEncoder^ encoder) {
                    encoder->SetPixelData(BitmapPixelFormat::Bgra8, BitmapAlphaMode::Straight, pixelWidth, pixelHeight, 96.0, 96.0, managedPixelBuffer);
                    return encoder->FlushAsync();
                    });
}

void MediaUtils::CaptureManager::SaveClip()
{
    critical_section::scoped_lock lock(m_pCaptureCriticalSection);

    if (m_pCurrentEncoder)
    {
        return;
    }

    auto storageFolder = ApplicationData::Current->LocalFolder;
    auto fullFilePath = storageFolder->Path + "\\data.mp4";

    m_pCurrentEncoder = new FFMPEGEncoder(
        DXResources::make_string(std::wstring(fullFilePath->Data())),
        m_spDeviceResource->GetOutputSize().Width,
        m_spDeviceResource->GetOutputSize().Height,
        30);

    if (!m_pCurrentEncoder->InitializeEncoder())
    {
        delete m_pCurrentEncoder;
        m_pCurrentEncoder = nullptr;
        return;
    }

    TimeSpan period;
    period.Duration = (float)10000000 / 30; // 30 FPS

    ThreadPoolTimer^ periodicTimer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler([this](ThreadPoolTimer^ source) 
        {
            // Add thread handling
            critical_section::scoped_lock lock(m_pCaptureCriticalSection);

            if (!m_pCurrentEncoder)
            {
                return;
            }

            if (m_iCurrentFrameCount == 300)
            {
                m_pCurrentEncoder->FinalizeEncoding();
                delete m_pCurrentEncoder;
                m_pCurrentEncoder = nullptr;

                m_iCurrentFrameCount = 0;

                source->Cancel();

                return;
            }

            // Grab the next frame
            UINT pixelWidth, pixelHeight;
            size_t bufferLength;
            std::unique_ptr<uint8_t[]> pixelBuffer;

            {
                Concurrency::critical_section::scoped_lock lock(m_spDirectXMain->GetCriticalSection());
                pixelBuffer.reset(m_spDeviceResource->GetLastRenderedFrame(pixelWidth, pixelHeight, bufferLength));
            }

            m_pCurrentEncoder->AddFrame((char*)pixelBuffer.get());
            m_iCurrentFrameCount++;

        }), period);
}

void MediaUtils::CaptureManager::SetSaveLocation(const std::string& saveLocation)
{
}
