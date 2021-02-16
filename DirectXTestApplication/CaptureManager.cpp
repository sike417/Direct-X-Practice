#include "pch.h"
#include "CaptureManager.h"
#include "FFMPEGEncoder.h"
#include "DirectXHelper.h"

using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;
using namespace Microsoft::WRL;
using namespace concurrency;

MediaUtils::CaptureManager::CaptureManager(std::shared_ptr<DirectX::DeviceResources> spDeviceResource)
    : m_spDeviceResource(spDeviceResource)
    , m_saveLocation("")
{
}

void MediaUtils::CaptureManager::SaveImage()
{
    UINT pixelWidth, pixelHeight;
    size_t bufferLength;
    std::unique_ptr<uint8_t[]> pixelBuffer;
    pixelBuffer.reset(m_spDeviceResource->GetLastRenderedFrame(pixelWidth, pixelHeight, bufferLength));
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
    auto storageFolder = ApplicationData::Current->LocalFolder;
    auto fullFilePath = storageFolder->Path + "\\data.mp4";

    auto encoder = new FFMPEGEncoder(
        DirectX::make_string(std::wstring(fullFilePath->Data())),
        m_spDeviceResource->GetOutputSize().Width,
        m_spDeviceResource->GetOutputSize().Height,
        30);

    if (!encoder->InitializeEncoder())
    {
        return;
    }
}

void MediaUtils::CaptureManager::SetSaveLocation(const std::string& saveLocation)
{
}
