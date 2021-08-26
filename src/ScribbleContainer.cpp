#include "ScribbleContainer.hpp"
#include "logging.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Shader.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"

DEFINE_TYPE(Scribble, LinerendererData);
DEFINE_TYPE(Scribble, ScribbleContainer);

using namespace UnityEngine;

namespace Scribble
{
    ScribbleContainer* ScribbleContainer::instance = nullptr;

    void ScribbleContainer::Start()
    {
        instance = this;
        lineRenderers = List<LinerendererData*>::New_ctor();
        INFO("Scribble Container Initialized");

        SetBPM(60);
    }

    ScribbleContainer* ScribbleContainer::get_instance()
    {
        return instance;
    }

    bool ScribbleContainer::get_IsInAnimation()
    {
        return animatedLoadRoutine != nullptr;
    }

    void ScribbleContainer::Create()
    {
        if (instance) return;
        auto go = GameObject::New_ctor(il2cpp_utils::newcsstr("ScribbleContainer"))->AddComponent<ScribbleContainer*>();
        Object::DontDestroyOnLoad(go);
    }

    Scribble::LineRenderer* ScribbleContainer::InitLineRenderer(const CustomBrush& brush, bool disableOnStart)
    {
        // create GO with numbered name
        INFO("Creating go");
        auto go = GameObject::New_ctor(il2cpp_utils::newcsstr(string_format("LineRenderer-%d", lineRenderers->get_Count())));
        INFO("set parent");
        go->get_transform()->SetParent(get_transform());
        INFO("add linerenderer");
        auto lineRenderer = go->AddComponent<Scribble::LineRenderer*>();
        INFO("set data");
        lineRenderer->set_enabled(!disableOnStart);
        lineRenderer->set_widthMultiplier(brush.size * lineWidth);
        lineRenderer->set_numCornerVertices(5);
        lineRenderer->set_numCapVertices(5);

        INFO("set modes");
        if (brush.textureMode == CustomBrush::TextureMode::Stretch)
            lineRenderer->set_textureMode(Scribble::LineRenderer::LineTextureMode::Stretch);
        else if (brush.textureMode == CustomBrush::TextureMode::Tile)
            lineRenderer->set_textureMode(Scribble::LineRenderer::LineTextureMode::Tile);
        lineRenderer->set_material(brush.CreateMaterial());

        INFO("add line");
        lineRenderers->Add(LinerendererData::Create(lineRenderer, brush));
        INFO("return");
        return lineRenderer;
    }

    void ScribbleContainer::SetBPM(float BPM)
    {
        static auto _BPM = il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("_BPM");
        // bpm / 60 means per second, 60 bpm is once per second
        Shader::SetGlobalFloat(_BPM, BPM / 60);
    }

    void ScribbleContainer::SetOffset(float offset)
    {
        static auto _StartOffset = il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("_StartOffset");
        Shader::SetGlobalFloat(_StartOffset, offset);
    }

    void ScribbleContainer::InitPoint(Sombrero::FastVector3 point, GlobalNamespace::SaberType saberType, CustomBrush& brush)
    {
        INFO("initpoint");
        auto lineRenderer = InitLineRenderer(brush);
        INFO("set stuff");
        lineRenderer->set_positionCount(1);
        lineRenderer->SetPosition(0, point);
        lineRenderer->set_enabled(true);
        INFO("currentLinerenderer setting");
        if (saberType == GlobalNamespace::SaberType::SaberA) currentLineRendererLeft = lineRenderer;
        else currentLineRendererRight = lineRenderer;
    }

    void ScribbleContainer::AddPoint(Sombrero::FastVector3 point, GlobalNamespace::SaberType saberType)
    {
        auto lineRenderer = saberType==GlobalNamespace::SaberType::SaberA ? currentLineRendererLeft : currentLineRendererRight;
        int posCount = lineRenderer->get_positionCount();
        lineRenderer->set_positionCount(posCount + 1);
        lineRenderer->SetPosition(posCount, point);
    }

    std::vector<Sombrero::FastVector3> ScribbleContainer::GetAllPoints()
    {
        std::vector<Sombrero::FastVector3> result = {};
        int length = lineRenderers->get_Count();
        for (int i = 0; i < length; i++)
        {
            auto lineRendererData = lineRenderers->items->values[i];
            auto positions = lineRendererData->lineRenderer->GetPositions();
            for (auto pos : positions)
                result.push_back(pos);
        }
        return result;
    }

    void ScribbleContainer::Erase(Sombrero::FastVector3 position, float size)
    {
        int length = lineRenderers->get_Count();
        float sizeSqr = size * size;
        // cache what to delete
        std::vector<LinerendererData*> toDelete = {};
        for (int i = 0; i < length; i++)
        {
            auto lineRendererData = lineRenderers->items->values[i];
            auto positions = lineRendererData->lineRenderer->GetPositions();
            for (auto& point : positions)
            {
                float dist = point.sqrDistance(position);
                if (dist < sizeSqr)
                {
                    toDelete.push_back(lineRendererData);
                    break;
                }
            }
        }

        // delete them
        for (auto l : toDelete) Delete(l);
    }

    void ScribbleContainer::Clear()
    {
        int length = lineRenderers->get_Count();
        for (int i = 0; i < length; i++)
        {
            Object::Destroy(lineRenderers->items->values[i]->lineRenderer->get_gameObject());
        }

        lineRenderers->Clear();
    }

    void ScribbleContainer::Undo()
    {
        INFO("Undo");
        Delete(lineRenderers->get_Count() - 1);
    }

    void ScribbleContainer::Delete(int index)
    {
        if (index < 0 || index > lineRenderers->get_Count() - 1) return;
        INFO("Deleting index %d", index);
        auto lr = lineRenderers->items->values[index]->lineRenderer;
        lineRenderers->RemoveAt(index);
        Object::Destroy(lr->get_gameObject());
    }

    void ScribbleContainer::Delete(LinerendererData* data)
    {
        INFO("Deleting data");
        auto lr = data->lineRenderer;
        auto b = lineRenderers->Remove(data);
        Object::Destroy(lr);
    }

    void ScribbleContainer::SetLayer(int layer)
    {
        int length = lineRenderers->get_Count();
        for (int i = 0; i < length; i++)
            lineRenderers->items->values[i]->lineRenderer->get_gameObject()->set_layer(layer);
    }

    void ScribbleContainer::Show()
    {
        get_gameObject()->SetActive(true);
    }

    void ScribbleContainer::Hide()
    {
        get_gameObject()->SetActive(false);
    }

    void ScribbleContainer::CheckLine(GlobalNamespace::SaberType saberType)
    {
        auto lineRenderer = saberType == GlobalNamespace::SaberType::SaberA ? currentLineRendererLeft : currentLineRendererRight;
        if (!lineRenderer) return;

        for (int i = 0; i < lineRenderers->get_Count(); i++)
        {
            auto data = lineRenderers->items->values[i];
            if (data->lineRenderer == lineRenderer && lineRenderer->get_positionCount() <= minPositionCount) Delete(data);
        }
    }

    LinerendererData* LinerendererData::Create(Scribble::LineRenderer* linerenderer, const CustomBrush& brush)
    {
        auto data = *il2cpp_utils::New<LinerendererData*>();
        data->lineRenderer = linerenderer;
        data->brush.copy(brush);
        return data;
    }
}