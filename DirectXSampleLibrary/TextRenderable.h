#pragma once
#include "IRenderable.h"

namespace GraphicsScenes
{
    class TextRenderable : public IRenderable
    {
    public:
        TextRenderable();

        // Inherited via IRenderable
        virtual void drawShape() override;

        void UpdateDisplayText(std::wstring desiredText);

    private:
        void createDeviceDependentResources();

    private:

        std::wstring m_text;
        DWRITE_TEXT_METRICS  m_textMetrics;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_whiteBrush;
        Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
        Microsoft::WRL::ComPtr<IDWriteTextLayout3> m_textLayout;
        Microsoft::WRL::ComPtr<IDWriteTextFormat2> m_textFormat;
    };
}

