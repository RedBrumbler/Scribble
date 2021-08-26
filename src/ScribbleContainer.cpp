#include "ScribbleContainer.hpp"
#include "logging.hpp"
#include "config.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Shader.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "static-defines.hpp"

#include "Utils/ThumbnailHelper.hpp"

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
        Object::Destroy(lr->get_gameObject());
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

    void ScribbleContainer::Save(std::string_view path, bool clear)
    {
        if (fileexists(path)) deletefile(path);
        
        std::ofstream outStream(path, std::ios::out | std::ios::binary);

        SetLayer(30);
        // make camera
        auto cam = ThumbnailHelper::CreateCamera({1.3676883f, 1.674314f, -2.518276f}, {12.04695f, 341.8893f, 3.806141f});
        // write thumbnail to file
        ThumbnailHelper::WriteThumbnail(outStream, cam, config.thumbnailSize, config.thumbnailSize);
        // destroy camera
        Object::Destroy(cam->get_gameObject());

        outStream.write(reinterpret_cast<const char*>(&imageVersion), sizeof(int));
        int lineCount = lineRenderers->get_Count();
        outStream.write(reinterpret_cast<const char*>(&lineCount), sizeof(int));
        for (int i = 0; i < lineCount; i++)
        {
            lineRenderers->get_Item(i)->Serialize(outStream);
        }

        outStream.flush();
        if (clear) Clear();
        SetLayer(0);
    }   

    void ScribbleContainer::Load(std::string_view path, bool clear)
    {
        if (!fileexists(path)) return;
        std::ifstream inStream(path, std::ios::out | std::ios::binary);
        
        long size;
        ThumbnailHelper::CheckPngData(inStream, size, true);
        // regardless of if it had valid png data, we should now be at the point after IEND -> where our data lives
        int foundVer;
        inStream.read(reinterpret_cast<char*>(&foundVer), sizeof(int));

        if (foundVer != imageVersion)
        {
            ERROR("Image version number was not right!");
            return;
        }
        
        if (clear) Clear();
        int lineCount;
        inStream.read(reinterpret_cast<char*>(&lineCount), sizeof(int));

        for (int i = 0; i < lineCount; i++)
        {
            LinerendererData::Deserialize(inStream);
        }
    }

    void LinerendererData::Deserialize(std::ifstream& reader)
    {
        auto brush = CustomBrush::Deserialize(reader);
        int posCount;
        reader.read(reinterpret_cast<char*>(&posCount), sizeof(int));
        auto lineRenderer = ScribbleContainer::get_instance()->InitLineRenderer(brush);
        lineRenderer->set_positionCount(posCount);
        Sombrero::FastVector3 val;
        for (int i = 0; i < posCount; i++)
        {
            reader.read(reinterpret_cast<char*>(&val), sizeof(Sombrero::FastVector3));
            lineRenderer->SetPosition(i, val);
        }
        
        lineRenderer->set_enabled(true);
    }

    void LinerendererData::Serialize(std::ofstream& writer)
    {
        // write out brush to out stream
        brush.Serialize(writer);

        auto positions = lineRenderer->GetPositions();
        // write the amount of positions out
        int posCount = positions.size();
        writer.write(reinterpret_cast<const char*>(&posCount), sizeof(int));
        // write out all the positions, a pos is basically 3 floats
        for (auto& pos : positions)
        {
            writer.write(reinterpret_cast<const char*>(&pos), sizeof(Sombrero::FastVector3));
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