#include "pch.h"
#include "CaptureManager.h"

#include <wrl.h>
#include <robuffer.h>
#include <ppltasks.h>

using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;
using namespace Microsoft::WRL;
using namespace concurrency;

DirectX::CaptureManager::CaptureManager(std::shared_ptr<DirectX::DeviceResources> spDeviceResource)
    : m_spDeviceResource(spDeviceResource)
    , m_saveLocation("C:\\Users\\Public\\Pictures\\")
{
}

void DirectX::CaptureManager::SaveImage()
{
    UINT pixelWidth, pixelHeight;
    size_t bufferLength;
    std::unique_ptr<uint8_t[]> pixelBuffer;
    pixelBuffer.reset(m_spDeviceResource->GetLastRenderedFrame(pixelWidth, pixelHeight, bufferLength));
    Platform::Array<unsigned char>^ managedPixelBuffer = ref new Platform::Array<unsigned char>(&pixelBuffer[0], bufferLength);

    create_task(StorageFolder::GetFolderFromPathAsync(m_saveLocation)).then([](StorageFolder^ folder)
        {
            return folder->CreateFileAsync("data.png", CreationCollisionOption::ReplaceExisting);
        }).then([](StorageFile^ file)
            {
                return file->OpenAsync(FileAccessMode::ReadWrite);
            }).then([](IRandomAccessStream^ stream)
                {
                    return BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId, stream);
                }).then([pixelWidth, pixelHeight, managedPixelBuffer](BitmapEncoder^ encoder)
                    {
                        encoder->SetPixelData(BitmapPixelFormat::Bgra8, BitmapAlphaMode::Straight, pixelWidth, pixelHeight, 96.0, 96.0, managedPixelBuffer);
                        return encoder->FlushAsync();
                    });
}

void DirectX::CaptureManager::SetSaveLocation(const std::string& saveLocation)
{
}
