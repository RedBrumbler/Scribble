#include "UI/UITools.hpp"

#include "UI/UIElements.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "TMPro/TMP_FontAsset.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"

#include <string_view>

namespace UITools
{
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::RectTransform* parent, std::string_view text, UnityEngine::Vector2 anchoredPosition)
    {
        auto textGo = UnityEngine::GameObject::New_ctor(il2cpp_utils::newcsstr("Text"));
        textGo->SetActive(false);
        auto textComp = textGo->AddComponent<TMPro::TextMeshProUGUI*>();
        auto font = QuestUI::ArrayUtil::First(UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_FontAsset*>(), [](auto t){ 
            static auto font_cs = il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("Teko-Medium SDF");
            return t && t->get_name() && font_cs->Equals(t->get_name());
        });
        textComp->set_font(font);
        textComp->get_rectTransform()->SetParent(parent, false);
        textComp->set_text(il2cpp_utils::newcsstr(text));
        textComp->set_fontSize(14.0f);
        textComp->set_overrideColorTags(true);
        textComp->get_rectTransform()->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
        textComp->get_rectTransform()->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
        textComp->get_rectTransform()->set_sizeDelta(UnityEngine::Vector2(30.0f, 15.0f));
        textComp->get_rectTransform()->set_anchoredPosition(anchoredPosition);
        textComp->get_rectTransform()->set_pivot(UnityEngine::Vector2(0, 0.5f));

        auto fitter = textGo->AddComponent<UnityEngine::UI::ContentSizeFitter*>();
        fitter->set_horizontalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);
        fitter->set_verticalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);

        textGo->SetActive(true);

        return textComp;
    }

    void SetRaycasterCamera(UnityEngine::GameObject* go, UnityEngine::Canvas* canvas)
    {
        auto raycaster = go->GetComponent<VRUIControls::VRGraphicRaycaster*>();
        if (!raycaster) raycaster = go->AddComponent<VRUIControls::VRGraphicRaycaster*>();
        // dont need to set, property not used and always null
        //raycaster->set_eventCamera(UnityEngine::Camera::get_main());
        if(canvas) raycaster->canvas = canvas;
    }

    UnityEngine::UI::Image* CreateImage(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta)
    {
        auto image_go = UnityEngine::GameObject::New_ctor(il2cpp_utils::newcsstr("Image"));
        auto imageComp = image_go->AddComponent<UnityEngine::UI::Image*>();
        imageComp->get_rectTransform()->SetParent(parent, false);
        imageComp->get_rectTransform()->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
        imageComp->get_rectTransform()->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
        imageComp->get_rectTransform()->set_sizeDelta(sizeDelta);
        imageComp->get_rectTransform()->set_anchoredPosition(anchoredPosition);

        return imageComp;
    }
    
    UnityEngine::GameObject* CreateFromInstance(UnityEngine::Transform* parent, std::string_view name, bool worldPositionStays)
    {
        auto nameCS = il2cpp_utils::newcsstr(name);
        auto go = QuestUI::ArrayUtil::First(UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::GameObject*>(), [nameCS](auto x){
            return x && nameCS->Equals(x->get_name());
        });
        return UnityEngine::Object::Instantiate(go, parent, worldPositionStays);
    }

    Scribble::ScribbleUISlider CreateSlider(UnityEngine::Transform* parent)
    {
        auto go = CreateFromInstance(parent, "PositionX");
        return Scribble::ScribbleUISlider(go);
    }

    Scribble::ScribbleUISimpleButton CreateSimpleButton(UnityEngine::Transform* parent, std::string_view text)
    {
        auto go = CreateFromInstance(parent, "ApplyButton");
        return Scribble::ScribbleUISimpleButton(go, text); 
    }

    Scribble::ScribbleUICheckbox CreateCheckbox(UnityEngine::Transform* parent)
    {
        auto go = CreateFromInstance(parent, "Toggle");
        return Scribble::ScribbleUICheckbox(go);
    }

    UnityEngine::GameObject* GetChild(UnityEngine::GameObject* go, std::string_view path)
    {
        return go->get_transform()->Find(il2cpp_utils::newcsstr(path))->get_gameObject();
    }
}