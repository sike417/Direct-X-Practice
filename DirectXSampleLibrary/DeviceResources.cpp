#include "pch.h"
#include "DeviceResources.h"
#include <windows.ui.xaml.media.dxinterop.h>
#include "DirectXHelper.h"

using namespace D2D1;
using namespace DXResources;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;
using namespace Concurrency;

namespace DisplayMetrics
{
    // High resolution displays can require a lot of GPU and battery power to render.
    // High resolution phones, for example, may suffer from poor battery life if
    // games attempt to render at 60 frames per second at full fidelity.
    // The decision to render at full fidelity across all platforms and form factors
    // should be deliberate.
    static const bool SupportHighResolutions = false;

    // The default thresholds that define a "high resolution" display. If the thresholds
    // are exceeded and SupportHighResolutions is false, the dimensions will be scaled
    // by 50%.
    static const float DpiThreshold = 192.0f;        // 200% of standard desktop display.
    static const float WidthThreshold = 1920.0f;    // 1080p width.
    static const float HeightThreshold = 1080.0f;    // 1080p height.
};

DXResources::DeviceResources::DeviceResources()
    : m_d3dDevice(nullptr)
    , m_swapChainPanel(nullptr)
    , m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1)
    , m_d3dRenderTargetSize(0, 0)
    , m_outputSize(0, 0)
    , m_logicalSize(0, 0)
    , m_dpi(0)
    , m_effectiveDpi(0)
{
    initializeIndependentDeviceResources();
    initializeDeviceResources();
}

void DXResources::DeviceResources::SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel)
{
    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

    m_swapChainPanel = panel;
    m_logicalSize = Windows::Foundation::Size(static_cast<float>(panel->ActualWidth), static_cast<float>(panel->ActualHeight));
    m_dpi = currentDisplayInformation->LogicalDpi;

    configureSwapChain();
}

void DXResources::DeviceResources::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
    if (m_logicalSize != logicalSize)
    {
        m_logicalSize = logicalSize;
        configureSwapChain();
    }
}

void DXResources::DeviceResources::PresentView()
{
    HRESULT hr = S_OK;

    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    DXGI_PRESENT_PARAMETERS parameters = { 0 };

    hr = m_swapChain->Present1(1, 0, &parameters);

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be modified.

    // TODO: Determine if this should be added back in. just uncommenting this prevents capture cause the back buffer is blank.
    // There may be performance improvements by copying off the back buffer, storing it in a member variable and then discarding the view on presenting time.
    // and then allow the capture manager to simply copy the member variable when it needs to.
    //m_d3dContext->DiscardView1(m_renderTargetView.Get(), nullptr, 0);

    // Discard the contents of the depth stencil.
    //m_d3dContext->DiscardView1(m_depthStencilView.Get(), nullptr, 0);

    // If the device was removed either by a disconnection or a driver upgrade, we 
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        //HandleDeviceLost();
    }
    else
    {
        DXResources::ThrowIfFailed(hr);
    }
}

uint8_t* DXResources::DeviceResources::GetLastRenderedFrame(UINT& width, UINT& height, size_t& bufferLength)
{
    width = height = bufferLength = 0;

    //ATL::CComCritSecLock<CComAutoCriticalSection> swapChainLock(m_swapChainCriticalSection);

    // get the backbuffer from the swapchain.
    ComPtr<ID3D11Texture2D1> backBuffer;
    DXResources::ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
    );

    // copy the back buffer to a staging area
    D3D11_TEXTURE2D_DESC description;
    backBuffer->GetDesc(&description);

    // modify the description to allow cpu access
    description.BindFlags = 0;
    description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    description.Usage = D3D11_USAGE_STAGING;

    // create temporary surface.
    ComPtr<ID3D11Texture2D> pNewSurface;
    HRESULT hr = m_d3dDevice->CreateTexture2D(&description, NULL, &pNewSurface);

    // Copy the back buffer to the new staging area.
    if (pNewSurface)
    {
        m_d3dContext->CopyResource(pNewSurface.Get(), backBuffer.Get());

        D3D11_MAPPED_SUBRESOURCE resource;
        m_d3dContext->Map(pNewSurface.Get(), D3D11CalcSubresource(0, 0, 0), D3D11_MAP_READ, 0, &resource);

        const unsigned char* source = static_cast<const unsigned char*>(resource.pData);
        size_t slicePitch, rowPitch, rowCount;
        hr = DXResources::GetSurfaceInfo(description.Width, description.Height, description.Format, &slicePitch, &rowPitch, &rowCount);

        if (FAILED(hr))
        {
            // DO SOMETHING
        }

        width = description.Width;
        height = description.Height;

        std::unique_ptr<uint8_t[]> pixels(new (std::nothrow) uint8_t[slicePitch]);
        bufferLength = slicePitch;
        uint8_t* dptr = pixels.get();

        size_t msize = std::min<size_t>(rowPitch, resource.RowPitch);

        for (size_t h = 0; h < rowCount; ++h)
        {
            memcpy(dptr, source, msize);

            source += resource.RowPitch;
            dptr += rowPitch;
        }

        m_d3dContext->Unmap(pNewSurface.Get(), 0);

        return pixels.release();
    }

    return nullptr;
}

void DXResources::DeviceResources::initializeIndependentDeviceResources()
{
}

void DXResources::DeviceResources::initializeDeviceResources()
{
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    // If the project is in a debug build, enable debugging via SDK Layers with this flag.
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // This array defines the set of DirectX hardware feature levels this app will support.
    // Note the ordering should be preserved.
    // Don't forget to declare your application's minimum required feature level in its
    // description.  All applications are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // Create the Direct3D 11 API device object and a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,    // Create a device using the hardware graphics driver.
        0,                            // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        creationFlags,                // Set debug and Direct2D compatibility flags.
        featureLevels,                // List of feature levels this app can support.
        ARRAYSIZE(featureLevels),    // Size of the list above.
        D3D11_SDK_VERSION,            // Always set this to D3D11_SDK_VERSION for Microsoft Store apps.
        &device,                    // Returns the Direct3D device created.
        &m_d3dFeatureLevel,            // Returns feature level of device created.
        &context                    // Returns the device immediate context.
    );

    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create d3d device, falling back to warp device");

        //throw if failed
        DXResources::ThrowIfFailed(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
            0,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &device,
            &m_d3dFeatureLevel,
            &context
        ));
    }

    DXResources::ThrowIfFailed(
        device.As(&m_d3dDevice)
    );

    DXResources::ThrowIfFailed(
        context.As(&m_d3dContext)
    );
}

void DXResources::DeviceResources::configureSwapChain()
{
    updateRenderTargetSize();

    if (m_swapChain != nullptr)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(
            2,
            lround(m_d3dRenderTargetSize.Width),
            lround(m_d3dRenderTargetSize.Height),
            DXGI_FORMAT_B8G8R8A8_UNORM,
            0
        );

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            //HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        DXResources::ThrowIfFailed(hr);
    }
    else
    {
        // Otherwise, create a new one using the same adapter as the existing Direct3D device.
        DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

        swapChainDesc.Width = lround(m_d3dRenderTargetSize.Width);        // Match the size of the window.
        swapChainDesc.Height = lround(m_d3dRenderTargetSize.Height);
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;                // This is the most common swap chain format.
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;                                // Don't use multi-sampling.
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;                                    // Use double-buffering to minimize latency.
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;    // All Microsoft Store apps must use _FLIP_ SwapEffects.
        swapChainDesc.Flags = 0;
        swapChainDesc.Scaling = scaling;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        // This sequence obtains the DXGI factory that was used to create the Direct3D device above.
        ComPtr<IDXGIDevice3> dxgiDevice;
        DXResources::ThrowIfFailed(
            m_d3dDevice.As(&dxgiDevice)
        );

        ComPtr<IDXGIAdapter> dxgiAdapter;
        DXResources::ThrowIfFailed(
            dxgiDevice->GetAdapter(&dxgiAdapter)
        );

        ComPtr<IDXGIFactory4> dxgiFactory;
        DXResources::ThrowIfFailed(
            dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
        );

        // When using XAML interop, the swap chain must be created for composition.
        ComPtr<IDXGISwapChain1> swapChain;
        DXResources::ThrowIfFailed(
            dxgiFactory->CreateSwapChainForComposition(
                m_d3dDevice.Get(),
                &swapChainDesc,
                nullptr,
                &swapChain
            )
        );

        DXResources::ThrowIfFailed(
            swapChain.As(&m_swapChain)
        );

        // Associate swap chain with SwapChainPanel
        // UI changes will need to be dispatched back to the UI thread
        m_swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
            {
                // Get backing native interface for SwapChainPanel
                ComPtr<ISwapChainPanelNative> panelNative;
                DXResources::ThrowIfFailed(
                    reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative))
                );

                DXResources::ThrowIfFailed(
                    panelNative->SetSwapChain(m_swapChain.Get())
                );
            }, CallbackContext::Any));

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
        // ensures that the application will only render after each VSync, minimizing power consumption.
        DXResources::ThrowIfFailed(
            dxgiDevice->SetMaximumFrameLatency(1)
        );
    }

    // Create a render target view of the swap chain back buffer.
    ComPtr<ID3D11Texture2D1> backBuffer;
    DXResources::ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
    );

    DXResources::ThrowIfFailed(
        m_d3dDevice->CreateRenderTargetView1(
            backBuffer.Get(),
            nullptr,
            &m_renderTargetView
        )
    );

    // Create a depth stencil view for use with 3D rendering if needed.
    CD3D11_TEXTURE2D_DESC1 depthStencilDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,      // TODO: What is this?
        lround(m_d3dRenderTargetSize.Width),
        lround(m_d3dRenderTargetSize.Height),
        1, // This depth stencil view has only one texture.
        1, // Use a single mipmap level.
        D3D11_BIND_DEPTH_STENCIL
    );

    ComPtr<ID3D11Texture2D1> depthStencil;
    DXResources::ThrowIfFailed(
        m_d3dDevice->CreateTexture2D1(
            &depthStencilDesc,
            nullptr,
            &depthStencil
        )
    );

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DXResources::ThrowIfFailed(
        m_d3dDevice->CreateDepthStencilView(
            depthStencil.Get(),
            &depthStencilViewDesc,
            &m_depthStencilView
        )
    );

    // Set the 3D rendering viewport to target the entire window.
    m_screenViewport = CD3D11_VIEWPORT(
        0.0f,
        0.0f,
        m_d3dRenderTargetSize.Width,
        m_d3dRenderTargetSize.Height
    );

    m_d3dContext->RSSetViewports(1, &m_screenViewport);

    //// Create a Direct2D target bitmap associated with the
    //// swap chain back buffer and set it as the current target.
    //D2D1_BITMAP_PROPERTIES1 bitmapProperties =
    //    D2D1::BitmapProperties1(
    //        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
    //        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
    //        m_dpi,
    //        m_dpi
    //    );

    //ComPtr<IDXGISurface2> dxgiBackBuffer;
    //DirectX::ThrowIfFailed(
    //    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
    //);

    //DX::ThrowIfFailed(
    //    m_d2dContext->CreateBitmapFromDxgiSurface(
    //        dxgiBackBuffer.Get(),
    //        &bitmapProperties,
    //        &m_d2dTargetBitmap
    //    )
    //);

    //m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());
    //m_d2dContext->SetDpi(m_effectiveDpi, m_effectiveDpi);

    //// Grayscale text anti-aliasing is recommended for all Microsoft Store apps.
    //m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

void DXResources::DeviceResources::updateRenderTargetSize()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
    m_renderTargetView = nullptr;
    //m_depthStencilView = nullptr;
    m_d3dContext->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);

    m_effectiveDpi = m_dpi;

    // TODO
    // To improve battery life on high resolution devices, render to a smaller render target
    // and allow the GPU to scale the output when it is presented.

    //// Calculate the necessary render target size in pixels.
    m_outputSize.Width = DXResources::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
    m_outputSize.Height = DXResources::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

    // Prevent zero size DirectX content from being created.
    m_outputSize.Width = max(m_outputSize.Width, 1);
    m_outputSize.Height = max(m_outputSize.Height, 1);

    m_d3dRenderTargetSize.Width = m_outputSize.Width;
    m_d3dRenderTargetSize.Height = m_outputSize.Height;
}