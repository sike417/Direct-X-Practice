#pragma once

using namespace Windows::UI::Xaml::Data;
using namespace Platform;

namespace DirectXSampleLibrary::Converters
{
    public ref class TestDataConverter sealed : IValueConverter
    {
    public:
        // Inherited via IValueConverter
        virtual Platform::Object^ Convert(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language)
        {
            return value;
        }

        virtual Platform::Object^ ConvertBack(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language)
        {
            return value;
        }
    };

}