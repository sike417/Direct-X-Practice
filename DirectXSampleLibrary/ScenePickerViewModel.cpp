#include "pch.h"
#include "ScenePickerViewModel.h"

#include "IRenderable.h"
#include "RenderCubeScene.h"
#include "RenderCircleSceneV1.h"
#include "RenderCircleSceneV2.h"
#include "RenderSphereScene.h"

using namespace Platform::Collections;
using namespace GraphicsScenes;
using namespace DXResources;
using namespace DirectXSampleLibrary::ViewModel;

ScenePickerViewModel::ScenePickerViewModel(Platform::IntPtr resources)
    : m_pResources(static_cast<ResourcesWrapper*>((void*)resources))
{
    initializeScenes();
}

void ScenePickerViewModel::SelectionChanged(Platform::Object^ sender, SelectionChangedEventArgs^ args)
{
    if (CurrentlySelectedSceneItem != nullptr && m_pResources != nullptr)
    {
        IScene* nextScene = static_cast<IScene*>((void*)CurrentlySelectedSceneItem->GetSceneModel());
        m_pResources->M_spDirectxMain->SetCurrentScene(nextScene);
    }
}

void ScenePickerViewModel::initializeScenes()
{
    IScene::SetSharedSceneResources(m_pResources->M_spDeviceResources, m_pResources->M_spGameCamera);
    IRenderable::SetSharedRenderableResources(m_pResources->M_spDeviceResources, m_pResources->M_spGameCamera);

    SceneViewModel^ renderCubeScene = ref new SceneViewModel(new RenderCubeScene());
    renderCubeScene->ImageLocation = "Assets/Scenes/RenderCube.png";
    renderCubeScene->SceneName = "RenderCube";

    ObservableSceneList->Append(renderCubeScene);

    SceneViewModel^ RenderCircleSceneV1 = ref new SceneViewModel(new GraphicsScenes::RenderCircleSceneV1());
    RenderCircleSceneV1->ImageLocation = "Assets/Scenes/RenderCircleV1.png";
    RenderCircleSceneV1->SceneName = "RenderCircleV1";

    ObservableSceneList->Append(RenderCircleSceneV1);

    SceneViewModel^ RenderCircleSceneV2 = ref new SceneViewModel(new GraphicsScenes::RenderCircleSceneV2());
    RenderCircleSceneV2->ImageLocation = "Assets/Scenes/RenderCircleV2.png";
    RenderCircleSceneV2->SceneName = "RenderCircleV2";

    ObservableSceneList->Append(RenderCircleSceneV2);

    SceneViewModel^ RenderSphereScene = ref new SceneViewModel(new GraphicsScenes::RenderSphereScene());
    //RenderSphereScene->ImageLocation = "Assets/Scenes/RenderCircleV2.png";
    RenderSphereScene->SceneName = "RenderSphere";

    ObservableSceneList->Append(RenderSphereScene);

    CurrentlySelectedSceneItem = ObservableSceneList->First()->Current;
}
