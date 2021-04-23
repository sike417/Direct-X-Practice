#include "pch.h"
#include "TextRenderable.h"
#include "DirectXHelper.h"

using namespace Microsoft::WRL;

GraphicsScenes::TextRenderable::TextRenderable() : IRenderable()
{
    createDeviceDependentResources();
}

void GraphicsScenes::TextRenderable::drawShape()
{
    if (m_stateBlock == nullptr)
    {
        return;
    }

    auto context = s_spDeviceResources->GetD2DDeviceContext();
    auto logicalSize = s_spDeviceResources->GetLogicalSize();

    context->SaveDrawingState(m_stateBlock.Get());
    context->BeginDraw();

    auto screenTranslation = D2D1::Matrix3x2F::Translation(
        logicalSize.Width - m_textMetrics.layoutWidth,
        logicalSize.Height - m_textMetrics.height);

    // TODO: Handle orientation
    context->SetTransform(screenTranslation);

    DXResources::ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));

    context->DrawTextLayout(
        D2D1::Point2(0.f, 0.f),
        m_textLayout.Get(),
        m_whiteBrush.Get());

    HRESULT hr = context->EndDraw();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        DXResources::ThrowIfFailed(hr);
    }

    context->RestoreDrawingState(m_stateBlock.Get());
}

void GraphicsScenes::TextRenderable::UpdateDisplayText(std::wstring desiredText)
{
    m_text = desiredText;

    ComPtr<IDWriteTextLayout> textLayout;
    DXResources::ThrowIfFailed(
        s_spDeviceResources->GetDWriteFactory()->CreateTextLayout(
            m_text.c_str(),
            (uint32_t)m_text.length(),
            m_textFormat.Get(),
            240.0f,
            50.0f,
            &textLayout
        )
    );

    DXResources::ThrowIfFailed(
        textLayout.As(&m_textLayout)
    );
    
    DXResources::ThrowIfFailed(
        m_textLayout->GetMetrics(&m_textMetrics)
    );
}

void GraphicsScenes::TextRenderable::createDeviceDependentResources()
{
    ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

    ComPtr<IDWriteTextFormat> textFormat;
    DXResources::ThrowIfFailed(
        s_spDeviceResources->GetDWriteFactory()->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            32.0f,
            L"en-us",
            &textFormat
        )
    );

    DXResources::ThrowIfFailed(textFormat.As(&m_textFormat));

    DXResources::ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    DXResources::ThrowIfFailed(
        s_spDeviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
    );

    DXResources::ThrowIfFailed(s_spDeviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush));
}
