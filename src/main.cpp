#include "config.hpp"
#include "hooks.hpp"
#include "logging.hpp"
#include "static-defines.hpp"

#include "custom-types/shared/register.hpp"
#include "questui/shared/QuestUI.hpp"

#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController_InitData.hpp"
#include "GlobalNamespace/GamePause.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "GlobalNamespace/BpmController.hpp"
#include "VRUIControls/VRPointer.hpp"

#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"

#include "CustomTypes/BrushBehaviour.hpp"

#include "AssetLoader.hpp"
#include "BrushTextures.hpp"
#include "Brushes.hpp"
#include "Effects.hpp"
#include "ScribbleContainer.hpp"

#include "GlobalNamespace/MainFlowCoordinator.hpp"
#include "HMUI/FlowCoordinator.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/ViewController_AnimationType.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"

#include "UI/ScribbleSettingsViewController.hpp"
#include "UI/ScribbleUI.hpp"
#include "UI/ScribbleViewController.hpp"

using namespace Scribble;
bool pastLoad = false;
bool firstMenu = true;
bool firstWarmup = true;
MAKE_AUTO_HOOK_MATCH(SceneManager_SetActiveScene, &UnityEngine::SceneManagement::SceneManager::SetActiveScene, bool, UnityEngine::SceneManagement::Scene scene)
{
    bool result = SceneManager_SetActiveScene(scene);

    auto nameCS = scene.get_name();
    std::string name = nameCS ? nameCS : "";

    INFO("Scene %s loaded", name.c_str());
    if (name == "ShaderWarmup" && firstWarmup)
    {
        INFO("main menu = true now");
        AssetLoader::LoadBundle();
        if (firstWarmup)
            Effects::LoadEffects();
        pastLoad = true;
        firstWarmup = false;
    }

    if (name == "MainMenu")
    {
        firstMenu = false;
        auto ui = UnityEngine::Resources::FindObjectsOfTypeAll<ScribbleUI*>();
        if (ui && ui->Length() > 0)
            ui->values[0]->Show();
        auto scribbles = UnityEngine::Resources::FindObjectsOfTypeAll<ScribbleContainer*>();
        if (scribbles && scribbles->Length() > 0)
            scribbles->values[0]->Show();
    }

    if (name == "GameCore")
    {
        auto ui = UnityEngine::Resources::FindObjectsOfTypeAll<ScribbleUI*>();
        if (ui && ui->Length() > 0)
            ui->values[0]->Show(false);

        if (config.visibleDuringPlay)
            ScribbleContainer::get_instance()->Show();
        else
            ScribbleContainer::get_instance()->Hide();
    }

    return result;
}

MAKE_AUTO_HOOK_MATCH(VRPointer_Awake, &VRUIControls::VRPointer::Awake, void, VRUIControls::VRPointer* self)
{
    VRPointer_Awake(self);

    if (!pastLoad)
        return;

    auto leftController = self->dyn__leftVRController();
    auto rightController = self->dyn__rightVRController();

    if (!leftController->get_gameObject()->GetComponent<BrushBehaviour*>())
    {
        auto brush = leftController->get_gameObject()->AddComponent<BrushBehaviour*>();
        brush->saberType = GlobalNamespace::SaberType::SaberA;
        brush->pointer = self;
    }

    if (!rightController->get_gameObject()->GetComponent<BrushBehaviour*>())
    {
        auto brush = rightController->get_gameObject()->AddComponent<BrushBehaviour*>();
        brush->saberType = GlobalNamespace::SaberType::SaberB;
        brush->pointer = self;
    }
}

MAKE_AUTO_HOOK_MATCH(MainFlowCoordinator_DidActivate, &GlobalNamespace::MainFlowCoordinator::DidActivate, void, GlobalNamespace::MainFlowCoordinator* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    MainFlowCoordinator_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (firstActivation)
    {
        ScribbleContainer::Create();
        ScribbleUI::Create();
    }
}

/*
MAKE_AUTO_HOOK_MATCH(MainFlowCoordinator_TopViewControllerWillChange, &GlobalNamespace::MainFlowCoordinator::TopViewControllerWillChange, void, GlobalNamespace::MainFlowCoordinator* self, HMUI::ViewController* oldViewController, HMUI::ViewController* newViewController, HMUI::ViewController::AnimationType animationType)
{
	MainFlowCoordinator_TopViewControllerWillChange(self, oldViewController, newViewController, animationType);
    if (newViewController->Equals(self->mainMenuViewController))
	{
		INFO("Presenting scribble ViewController");
		auto controller = ScribbleViewController::get_instance();
		self->SetBottomScreenViewController(controller, animationType);
	}
}
    */

MAKE_AUTO_HOOK_MATCH(BeatmapObjectSpawnController_Start, &GlobalNamespace::BeatmapObjectSpawnController::Start, void, GlobalNamespace::BeatmapObjectSpawnController* self)
{
    BeatmapObjectSpawnController_Start(self);
    ScribbleContainer::SetBPM((float)self->dyn__initData()->dyn_beatsPerMinute());
    ScribbleContainer::SetOffset((float)self->dyn__initData()->dyn_noteJumpValue());
}

MAKE_AUTO_HOOK_MATCH(VariableBpmProcessor_ProcessBeatmapEventData, &GlobalNamespace::BpmController::HandleBpmChangeBeatmapEvent, void, GlobalNamespace::BpmController* self, GlobalNamespace::BPMChangeBeatmapEventData* bpmChangeBeatmapEventData)
{
    VariableBpmProcessor_ProcessBeatmapEventData(self, bpmChangeBeatmapEventData);

    if (bpmChangeBeatmapEventData)
    {
        ScribbleContainer::SetBPM(self->dyn__currentBpm());
    }
}

MAKE_AUTO_HOOK_MATCH(MenuTransitionsHelper_RestartGame, &GlobalNamespace::MenuTransitionsHelper::RestartGame, void, GlobalNamespace::MenuTransitionsHelper* self, System::Action_1<Zenject::DiContainer*>* finishCallback)
{
    ScribbleUI::Reset();
    ScribbleContainer::Reset();
    BrushTextures::Reset();
    MenuTransitionsHelper_RestartGame(self, finishCallback);
}

MAKE_AUTO_HOOK_MATCH(GamePause_Pause, &GlobalNamespace::GamePause::Pause, void, GlobalNamespace::GamePause* self)
{
    auto ui = UnityEngine::Resources::FindObjectsOfTypeAll<ScribbleUI*>();
    if (ui && ui->Length() > 0)
    {
        ScribbleUI::inPause = true;
        ui->values[0]->Show(true);
    }
    ScribbleContainer::get_instance()->Show();
    GamePause_Pause(self);
}

MAKE_AUTO_HOOK_MATCH(GamePause_Resume, &GlobalNamespace::GamePause::Resume, void, GlobalNamespace::GamePause* self)
{
    GamePause_Resume(self);
    auto ui = UnityEngine::Resources::FindObjectsOfTypeAll<ScribbleUI*>();
    if (ui && ui->Length() > 0)
    {
        auto scribbleUI = ui->values[0];
        if (scribbleUI->globalContainer->get_gameObject()->get_active())
        {
            scribbleUI->startButton.noTransitionsButton->get_onClick()->Invoke();
        }

        scribbleUI->Show(false);
        ScribbleUI::inPause = false;
    }

    if (config.visibleDuringPlay)
        ScribbleContainer::get_instance()->Show();
    else
        ScribbleContainer::get_instance()->Hide();
}

extern "C" void setup(ModInfo& info)
{
    info.id = MOD_ID;
    info.version = VERSION;
}

extern "C" void load()
{
    QuestUI::Init();
    if (!LoadConfig())
        SaveConfig();
    if (!direxists(brushTexturePath))
        mkpath(brushTexturePath);
    if (!direxists(drawingPath))
        mkpath(drawingPath);
    if (!direxists(modelsPath))
        mkpath(modelsPath);
    Brushes::Load();
    Hooks::InstallHooks(::Scribble::Logging::getLogger());
    custom_types::Register::AutoRegister();

    //QuestUI::Register::RegisterModSettingsViewController<ScribbleSettingsViewController*>({ID, VERSION});
}