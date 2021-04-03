#pragma once

#include "ResourcesWrapper.h"
#include "IScene.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Controls;

namespace DirectXSampleLibrary::ViewModel
{
    [Windows::UI::Xaml::Data::Bindable]
    public ref class SceneViewModel sealed
    {
    public:
        SceneViewModel(IntPtr sceneModel)
            : m_spSceneModel(static_cast<GraphicsScenes::IScene*>((void*)sceneModel))
        {

        }

        IntPtr GetSceneModel()
        {
            return m_spSceneModel.get();
        }

        property String^ SceneName;
        property String^ ImageLocation;
        
    private:
        std::unique_ptr<GraphicsScenes::IScene> m_spSceneModel;
    };

    [Windows::UI::Xaml::Data::Bindable]
    public ref class ScenePickerViewModel sealed : INotifyPropertyChanged
    {
    public:
        ScenePickerViewModel(Platform::IntPtr resources);

        property IVector<SceneViewModel^>^ ObservableSceneList
        {
            IVector<SceneViewModel^>^ get()
            {
                if (m_pSceneList == nullptr)
                {
                    m_pSceneList = ref new Vector<SceneViewModel^>();
                }

                return m_pSceneList;
            }
        }

        property SceneViewModel^ CurrentlySelectedSceneItem;

        // Inherited via INotifyPropertyChanged
        virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;

        void SelectionChanged(Platform::Object^ sender, SelectionChangedEventArgs^ args);

    private:
        void initializeScenes();
        
        void onPropertyChanged(String^ name)
        {
            PropertyChanged(this, ref new PropertyChangedEventArgs(name));
        }

    private:
        std::unique_ptr<DXResources::ResourcesWrapper> m_pResources;
        Platform::Collections::Vector<SceneViewModel^>^ m_pSceneList;
    };
}

