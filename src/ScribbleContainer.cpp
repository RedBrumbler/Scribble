#include "ScribbleContainer.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Shader.hpp"

DEFINE_TYPE(Scribble, LinerendererData);
DEFINE_TYPE(Scribble, ScribbleContainer);

using namespace UnityEngine;

namespace Scribble
{
    ScribbleContainer* ScribbleContainer::instance = nullptr;

    void ScribbleContainer::Start()
    {

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
        GameObject::New_ctor(il2cpp_utils::newcsstr("ScribbleContianer"))->AddComponent<ScribbleContainer*>();
    }

    Scribble::LineRenderer* ScribbleContainer::InitLineRenderer(const CustomBrush& brush, bool disableOnStart)
    {
        // create GO with numbered name
        auto go = GameObject::New_ctor(il2cpp_utils::newcsstr(string_format("LineRenderer-%d", lineRenderers->get_Count())));
        go->get_transform()->SetParent(get_transform());
        auto lineRenderer = go->AddComponent<Scribble::LineRenderer*>();
        lineRenderer->set_enabled(!disableOnStart);
        lineRenderer->set_widthMultiplier(brush.size * lineWidth);
        lineRenderer->set_numCornerVertices(5);
        lineRenderer->set_numCapVertices(5);

        if (brush.textureMode == CustomBrush::TextureMode::Stretch)
            lineRenderer->set_textureMode(Scribble::LineRenderer::LineTextureMode::Stretch);
        else if (brush.textureMode == CustomBrush::TextureMode::Tile)
            lineRenderer->set_textureMode(Scribble::LineRenderer::LineTextureMode::Tile);
        lineRenderer->set_material(brush.CreateMaterial());

        lineRenderers->Add(LinerendererData::Create(lineRenderer, brush));
        return lineRenderer;
    }

    void ScribbleContainer::UpdateMaterials(float BPM)
    {
        static auto _BPM = il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("_BPM");
        Shader::SetGlobalFloat(_BPM, BPM);
    }

    void ScribbleContainer::InitPoint(Sombrero::FastVector3 point, GlobalNamespace::SaberType saberType, CustomBrush& brush)
    {
        auto lineRenderer = InitLineRenderer(brush);
        lineRenderer->set_positionCount(1);
        lineRenderer->SetPosition(0, point);
        lineRenderer->set_enabled(true);
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
        for (int index = 0; index < length; index++)
        {
            auto lineRendererData = lineRenderers->items->values[index];
            auto positions = lineRendererData->lineRenderer->GetPositions();
            for (auto& point : positions)
            {
                if (point.sqrDistance(position) < sizeSqr)
                {
                    Delete(lineRendererData);
                    break;
                }
            }
        }
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
        Delete(lineRenderers->get_Count() - 1);
    }

    void ScribbleContainer::Delete(int index)
    {
        if (index < 0 || index > lineRenderers->get_Count() - 1) return;
        auto lr = lineRenderers->items->values[index]->lineRenderer;
        lineRenderers->RemoveAt(index);
        Object::Destroy(lr->get_gameObject());
    }

    void ScribbleContainer::Delete(LinerendererData* data)
    {
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
        if (lineRenderer->get_positionCount() <= minPositionCount)
            Undo();
    }

    LinerendererData* LinerendererData::Create(Scribble::LineRenderer* linerenderer, const CustomBrush& brush)
    {
        auto data = *il2cpp_utils::New<LinerendererData*>();
        data->lineRenderer = linerenderer;
        data->brush.copy(brush);
        return data;
    }
}