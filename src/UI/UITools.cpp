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
#include "HMUI/ImageView.hpp"
#include "System/Convert.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include <string_view>

namespace UITools
{
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::RectTransform* parent, std::string_view text, UnityEngine::Vector2 anchoredPosition)
    {
        auto textGo = UnityEngine::GameObject::New_ctor("Text");
        textGo->SetActive(false);
        auto textComp = textGo->AddComponent<TMPro::TextMeshProUGUI*>();
        auto font = QuestUI::ArrayUtil::First(UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_FontAsset*>(), [](auto t){ 
            static StringW font_cs = "Teko-Medium SDF";
            return t && t->get_name() && font_cs->Equals(t->get_name());
        });
        textComp->set_font(font);
        textComp->get_rectTransform()->SetParent(parent, false);
        textComp->set_text(text);
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
        if(canvas) raycaster->dyn__canvas() = canvas;
    }

    UnityEngine::UI::Image* CreateImage(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta)
    {
        auto image_go = UnityEngine::GameObject::New_ctor("Image");
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
        auto go = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::GameObject*>().FirstOrDefault([name](auto x){
            return x && x->get_name() == name;
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
        return go->get_transform()->Find(path)->get_gameObject();
    }

    void SetSkewForChildren(UnityEngine::GameObject* root, float skew)
    {
        auto imageViews = root->GetComponentsInChildren<HMUI::ImageView*>(true);
        int length = imageViews->Length();
        for (int i = 0; i < length; i++)
        {
            imageViews->values[i]->dyn__skew() = skew;
        }

    }

    UnityEngine::Sprite* Base64ToSprite(std::string_view base64)
    {
        ArrayW<uint8_t> bytes = System::Convert::FromBase64String(base64);
        return QuestUI::BeatSaberUI::ArrayToSprite(bytes);
    }
}