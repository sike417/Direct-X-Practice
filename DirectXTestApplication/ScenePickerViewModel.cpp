#include "pch.h"
#include "ScenePickerViewModel.h"

#include "RenderCubeScene.h"

using namespace Platform::Collections;
using namespace GraphicsScenes;
using namespace DirectXTestApplication::Common;

DirectXTestApplication::ScenePickerViewModel::ScenePickerViewModel(Platform::IntPtr resources)
    : m_pResources(static_cast<ResourcesWrapper*>((void*)resources))
{
    initializeScenes();
}

void DirectXTestApplication::ScenePickerViewModel::SelectionChanged(Platform::Object^ sender, SelectionChangedEventArgs^ args)
{
    if (CurrentlySelectedSceneItem != nullptr && m_pResources != nullptr)
    {
        IScene* nextScene = static_cast<IScene*>((void*)CurrentlySelectedSceneItem->GetSceneModel());
        m_pResources->M_spDirectxMain->SetCurrentScene(nextScene);
    }
}

void DirectXTestApplication::ScenePickerViewModel::initializeScenes()
{
    SceneViewModel^ renderCubeScene = ref new SceneViewModel(new RenderCubeScene(m_pResources->M_spDeviceResources, m_pResources->M_spGameCamera));
    renderCubeScene->ImageLocation = "Assets/Scenes/RenderCube.png";
    renderCubeScene->SceneName = "RenderCube";

    ObservableSceneList->Append(renderCubeScene);

    CurrentlySelectedSceneItem = ObservableSceneList->First()->Current;
}
