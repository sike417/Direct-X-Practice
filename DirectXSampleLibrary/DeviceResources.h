#pragma once

#include <atlcomcli.h>
#include <atlbase.h>

namespace DXResources
{
    class DeviceResources
    {
    public:
        DeviceResources();
        
        void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
        void SetLogicalSize(Windows::Foundation::Size logicalSize);
        void PresentView();
        uint8_t* GetLastRenderedFrame(UINT& width, UINT& height, size_t& bufferLength);

        // The size of the render target, in pixels.
        Windows::Foundation::Size   GetOutputSize() const { return m_outputSize; }

        // D3D Accessors.
        ID3D11Device3* GetD3DDevice() const { return m_d3dDevice.Get(); }
        ID3D11DeviceContext3* GetD3DDeviceContext() const { return m_d3dContext.Get(); }
        IDXGISwapChain3* GetSwapChain() const { return m_swapChain.Get(); }
        D3D_FEATURE_LEVEL      GetDeviceFeatureLevel() const { return m_d3dFeatureLevel; }
        ID3D11RenderTargetView1* GetBackBufferRenderTargetView() const { return m_renderTargetView.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthStencilView.Get(); }
        
        D3D11_VIEWPORT        GetScreenViewport() const { return m_screenViewport; }
        Windows::Foundation::Size GetLogicalSize() const { return m_logicalSize; }


        ID2D1Factory3* GetD2DFactory() const { return m_d2dFactory.Get(); }
        ID2D1DeviceContext2* GetD2DDeviceContext() const { return m_d2dContext.Get(); }
        IDWriteFactory3* GetDWriteFactory() const { return m_dwriteFactory.Get(); }

        DeviceResources(const DeviceResources&) = delete;
        DeviceResources(DeviceResources&&) = delete;
        DeviceResources& operator=(const DeviceResources&) = delete;
        DeviceResources& operator=(DeviceResources&&) = delete;

    private:
        void initializeIndependentDeviceResources();
        void initializeDeviceResources();
        void configureSwapChain();
        void updateRenderTargetSize();

        // Direct3D objects
        Microsoft::WRL::ComPtr<ID3D11Device3> m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext3> m_d3dContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> m_renderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

        // Direct2D drawing components.
        Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory;
        Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice;
        Microsoft::WRL::ComPtr<ID2D1DeviceContext2> m_d2dContext;
        Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dTargetBitmap;

        // Direct Write drawing components
        Microsoft::WRL::ComPtr<IDWriteFactory3> m_dwriteFactory;


        D3D11_VIEWPORT m_screenViewport;

        //cached reference to the XAML panel.
        Windows::UI::Xaml::Controls::SwapChainPanel^ m_swapChainPanel;

        //cached device properties.
        D3D_FEATURE_LEVEL                                m_d3dFeatureLevel;
        Windows::Foundation::Size                        m_d3dRenderTargetSize;
        Windows::Foundation::Size                        m_outputSize;
        Windows::Foundation::Size                        m_logicalSize;
        float                                            m_dpi;

        // Variables that take into account whether the app supports high resolution screens or not.
        float                                            m_effectiveDpi;

    };
}
