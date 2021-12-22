#include "ScribbleContainer.hpp"
#include "GlobalBrushManager.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/WaitForSeconds.hpp"
#include "config.hpp"
#include "logging.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BoolSO.hpp"
#include "GlobalNamespace/MainEffectContainerSO.hpp"
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
        drawingEnabled = false;
        lineRenderers = List<LinerendererData*>::New_ctor();
        INFO("Scribble Container Initialized");

        SetBPM(60);

        // turn on / off glow capability in scribble
        auto effectContainers = Resources::FindObjectsOfTypeAll<
            GlobalNamespace::MainEffectContainerSO*>();
        if (effectContainers && effectContainers.Length() > 0)
        {
            INFO("Found Effect Container");
            auto container = effectContainers[0];
            SetRealGlow(container->postProcessEnabled->get_value() &&
                        config.useRealGlow);
        }
        else
            SetRealGlow(config.useRealGlow);

        if (config.firstTimeLaunch)
        {
            Load(string_format("%s/%s", drawingPath, "first.png"), true, true);
        }
    }

    ScribbleContainer* ScribbleContainer::get_instance() { return instance; }

    bool ScribbleContainer::get_IsInAnimation()
    {
        return animatedLoadRoutine != nullptr;
    }

    void ScribbleContainer::Create()
    {
        if (instance)
            return;
        auto go = GameObject::New_ctor(il2cpp_utils::newcsstr("ScribbleContainer"))
                      ->AddComponent<ScribbleContainer*>();
        Object::DontDestroyOnLoad(go);
        go->get_transform()->set_position(Vector3::get_up());
    }

    Scribble::LineRenderer*
    ScribbleContainer::InitLineRenderer(const CustomBrush& brush,
                                        bool disableOnStart)
    {
        // create GO with numbered name
        auto go = GameObject::New_ctor(il2cpp_utils::newcsstr(
            string_format("LineRenderer-%d", lineRenderers->get_Count())));
        go->get_transform()->SetParent(get_transform());
        auto lineRenderer = go->AddComponent<Scribble::LineRenderer*>();
        lineRenderer->set_enabled(!disableOnStart);
        lineRenderer->set_widthMultiplier(brush.size * lineWidth);
        lineRenderer->set_numCornerVertices(3);
        lineRenderer->set_numCapVertices(3);
        lineRenderer->set_useWorldSpace(false);
        if (brush.textureMode == CustomBrush::TextureMode::Stretch)
            lineRenderer->set_textureMode(
                Scribble::LineRenderer::LineTextureMode::Stretch);
        else if (brush.textureMode == CustomBrush::TextureMode::Tile)
            lineRenderer->set_textureMode(
                Scribble::LineRenderer::LineTextureMode::Tile);
        lineRenderer->set_material(brush.CreateMaterial());

        lineRenderers->Add(LinerendererData::Create(lineRenderer, brush));
        return lineRenderer;
    }

#pragma region set_data
    void ScribbleContainer::SetRealGlow(bool real)
    {
        INFO("Setting real glow usage to: %d", real);
        static auto _ScribbleRealGlow =
            il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(
                "_ScribbleRealGlow");
        UnityEngine::Shader::SetGlobalFloat(_ScribbleRealGlow, real);
    }

    void ScribbleContainer::SetBPM(float BPM)
    {
        static auto _BPM =
            il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("_BPM");
        // bpm / 60 means per second, 60 bpm is once per second
        Shader::SetGlobalFloat(_BPM, BPM / 60);
    }

    void ScribbleContainer::SetOffset(float offset)
    {
        static auto _StartOffset =
            il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(
                "_StartOffset");
        Shader::SetGlobalFloat(_StartOffset, offset);
    }
#pragma endregion

#pragma region point_mutations
    void ScribbleContainer::InitPoint(Sombrero::FastVector3 point,
                                      GlobalNamespace::SaberType saberType,
                                      const CustomBrush& brush)
    {
        auto lineRenderer = InitLineRenderer(brush);
        lineRenderer->set_positionCount(1);
        lineRenderer->SetPosition(0, point);
        lineRenderer->set_enabled(true);
        if (saberType == GlobalNamespace::SaberType::SaberA)
            currentLineRendererLeft = lineRenderer;
        else
            currentLineRendererRight = lineRenderer;
    }

    void ScribbleContainer::AddPoint(Sombrero::FastVector3 point,
                                     GlobalNamespace::SaberType saberType)
    {
        auto lineRenderer = saberType == GlobalNamespace::SaberType::SaberA
                                ? currentLineRendererLeft
                                : currentLineRendererRight;
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
            auto lineRendererData = lineRenderers->items[i];
            auto positions = lineRendererData->lineRenderer->GetPositions();
            for (auto pos : positions)
                result.push_back(pos);
        }
        return result;
    }
#pragma endregion

#pragma region tools
    void ScribbleContainer::Erase(Sombrero::FastVector3 position, float size)
    {
        int length = lineRenderers->get_Count();
        float sizeSqr = size * size;
        // cache what to delete
        std::vector<LinerendererData*> toDelete = {};
        for (int i = 0; i < length; i++)
        {
            auto lineRendererData = lineRenderers->items[i];
            auto positions = lineRendererData->lineRenderer->GetPositions();
            Sombrero::FastVector3 offset = lineRendererData->lineRenderer->get_transform()->get_position();

            for (auto& point : positions)
            {
                float dist = (offset + point).sqrDistance(position);
                if (dist < sizeSqr)
                {
                    toDelete.push_back(lineRendererData);
                    break;
                }
            }
        }

        // delete them
        for (auto l : toDelete)
            Delete(l);
    }

    void ScribbleContainer::Bucket(Sombrero::FastVector3 position, float size,
                                   const CustomBrush& brush)
    {
        int length = lineRenderers->get_Count();
        float sizeSqr = size * size;
        for (int i = 0; i < length; i++)
        {
            auto lineRendererData = lineRenderers->items[i];
            if (lineRendererData->brush == brush)
                continue;
            auto positions = lineRendererData->lineRenderer->GetPositions();
            Sombrero::FastVector3 offset = lineRendererData->lineRenderer->get_transform()->get_position();

            for (auto& point : positions)
            {
                float dist = (offset + point).sqrDistance(position);
                if (dist < sizeSqr)
                {
                    lineRendererData->brush = brush;
                    lineRendererData->lineRenderer->set_material(brush.CreateMaterial());
                    break;
                }
            }
        }
    }

    void ScribbleContainer::Move(Sombrero::FastVector3 delta)
    {
        // dont move stuff while animating
        if (get_IsInAnimation())
            return;

        // move self the given delta
        get_transform()->Translate(delta);

        /*
        int length = lineRenderers->get_Count();
        for (int i = 0; i < length; i++)
        {
            auto line = lineRenderers->items[i]->lineRenderer;
            int j = 0;
            for (auto& pos : line->GetPositions())
            {
                line->SetPosition(j, pos + delta);
                j++;
            }
        }
        */
    }

    void ScribbleContainer::StartRuler(Sombrero::FastVector3 position,
                                       GlobalNamespace::SaberType type,
                                       const CustomBrush& brush)
    {
        InitPoint(position, type, brush);
        AddPoint(position, type);
    }

    void ScribbleContainer::UpdateRuler(Sombrero::FastVector3 position,
                                        GlobalNamespace::SaberType type)
    {
        auto lineRenderer = type == GlobalNamespace::SaberType::SaberA
                                ? currentLineRendererLeft
                                : currentLineRendererRight;
        lineRenderer->SetPosition(1, position);
    }

    void ScribbleContainer::FinishRuler(Sombrero::FastVector3 position,
                                        GlobalNamespace::SaberType type)
    {
        auto lineRenderer = type == GlobalNamespace::SaberType::SaberA
                                ? currentLineRendererLeft
                                : currentLineRendererRight;
        auto firstPos = lineRenderer->GetPosition(0);
        float dist = firstPos.Distance(position);

        int posCount = dist > 0.1f ? dist * 20 : 2;

        lineRenderer->set_positionCount(posCount + 1);
        for (int i = 0; i < posCount; i++)
        {
            lineRenderer->SetPosition(
                i, Sombrero::FastVector3::Lerp(firstPos, position,
                                               (float)i / (float)posCount));
        }

        lineRenderer->SetPosition(posCount, position);
        CheckLine(type);
    }
#pragma endregion

    void ScribbleContainer::Clear()
    {
        int length = lineRenderers->get_Count();
        for (int i = 0; i < length; i++)
        {
            Object::Destroy(
                lineRenderers->items[i]->lineRenderer->get_gameObject());
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
        if (index < 0 || index > lineRenderers->get_Count() - 1)
            return;
        INFO("Deleting index %d", index);
        auto lr = lineRenderers->items[index]->lineRenderer;
        if (lr == currentLineRendererLeft || lr == currentLineRendererRight)
            return;
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
            lineRenderers->items[i]->lineRenderer->get_gameObject()->set_layer(
                layer);
    }

    void ScribbleContainer::Show() { get_gameObject()->SetActive(true); }

    void ScribbleContainer::Hide() { get_gameObject()->SetActive(false); }

    void ScribbleContainer::CheckLine(GlobalNamespace::SaberType saberType)
    {
        auto lineRenderer = saberType == GlobalNamespace::SaberType::SaberA
                                ? currentLineRendererLeft
                                : currentLineRendererRight;
        if (!lineRenderer)
            return;

        for (int i = 0; i < lineRenderers->get_Count(); i++)
        {
            auto data = lineRenderers->items[i];
            if (data->lineRenderer == lineRenderer &&
                lineRenderer->get_positionCount() < minPositionCount)
                Delete(data);
        }

        switch (saberType)
        {
        case GlobalNamespace::SaberType::SaberA:
            currentLineRendererLeft = nullptr;
            break;
        case GlobalNamespace::SaberType::SaberB:
            currentLineRendererRight = nullptr;
            break;
        }
    }

    void ScribbleContainer::Save(std::string_view path, bool clear)
    {
        if (fileexists(path))
            deletefile(path);

        std::ofstream outStream(path, std::ios::out | std::ios::binary);

        SetLayer(30);
        // make camera
        auto cam = ThumbnailHelper::CreateCamera({1.3676883f, 1.674314f, -2.518276f},
                                                 {12.04695f, 341.8893f, 3.806141f});
        // write thumbnail to file
        ThumbnailHelper::WriteThumbnail(outStream, cam, config.thumbnailSize,
                                        config.thumbnailSize);
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
        if (clear)
            Clear();
        SetLayer(0);
    }

    void ScribbleContainer::Load(std::string_view path, bool clear, bool animated)
    {
        if (!fileexists(path))
            return;
        if (get_IsInAnimation())
            return;

        if (path.ends_with(".obj"))
        {
            // this is a 3d .obj file, we should load that instead
            LoadObj(path, clear, animated);
            return;
        }
        std::shared_ptr<std::ifstream> inStream =
            std::make_shared<std::ifstream>(path, std::ios::out | std::ios::binary);

        long size;
        ThumbnailHelper::CheckPngData(*inStream, size, true);
        // regardless of if it had valid png data, we should now be at the point after
        // IEND -> where our data lives
        int foundVer;
        inStream->read(reinterpret_cast<char*>(&foundVer), sizeof(int));

        if (foundVer != imageVersion)
        {
            ERROR("Image version number was not right!");
            return;
        }

        if (clear)
            Clear();
        int lineCount;
        inStream->read(reinterpret_cast<char*>(&lineCount), sizeof(int));

        if (animated)
        {
            animatedLoadRoutine =
                StartCoroutine(reinterpret_cast<System::Collections::IEnumerator*>(
                    custom_types::Helpers::CoroutineHelper::New(
                        LoadAnimated(inStream, lineCount))));
            return;
        }

        for (int i = 0; i < lineCount; i++)
        {
            LinerendererData::Deserialize(*inStream);
        }
    }

    custom_types::Helpers::Coroutine
    ScribbleContainer::LoadAnimated(std::shared_ptr<std::ifstream> reader,
                                    int lineCount)
    {
        INFO("Loading animated: %d", lineCount);

        float delay = 0.004f;
        auto brush = CustomBrush::Deserialize(*reader);

        for (int i = 0; i < lineCount; i++)
        {
            INFO("line: %d", i);
            
            int posCount;
            reader->read(reinterpret_cast<char*>(&posCount), sizeof(int));
            auto lineRenderer =
                ScribbleContainer::get_instance()->InitLineRenderer(brush);
            lineRenderer->set_enabled(true);
            Sombrero::FastVector3 val;
            lineRenderer->set_positionCount(2);
            reader->read(reinterpret_cast<char*>(&val), sizeof(Sombrero::FastVector3));
            lineRenderer->SetPosition(0, val);
            reader->read(reinterpret_cast<char*>(&val), sizeof(Sombrero::FastVector3));
            lineRenderer->SetPosition(1, val);

            for (int i = 2; i < posCount; i++)
            {
                reader->read(reinterpret_cast<char*>(&val),
                             sizeof(Sombrero::FastVector3));
                lineRenderer->set_positionCount(i + 1);
                lineRenderer->SetPosition(i, val);
                float actualDelay = ((float)i / (float)posCount) * delay;
                co_yield reinterpret_cast<System::Collections::IEnumerator*>(
                    UnityEngine::WaitForSeconds::New_ctor(actualDelay));
            }
        }

        animatedLoadRoutine = nullptr;
        co_return;
    }

    void ScribbleContainer::LoadObj(std::string_view path, bool clear, bool animated)
    {
        if (clear)
            Clear();
        std::shared_ptr<std::ifstream> inStream =
            std::make_shared<std::ifstream>(path, std::ios::out | std::ios::binary);

        auto obj = SimpleObj::Deserialize(*inStream);
        if (animated)
        {
            animatedLoadRoutine =
                StartCoroutine(reinterpret_cast<System::Collections::IEnumerator*>(
                    custom_types::Helpers::CoroutineHelper::New(
                        LoadObjAnimated(obj))));
            return;
        }

        auto brush = GlobalBrushManager::get_activeBrush()->currentBrush;
        int i = 0;
        int lineCount = obj.edges.size();
        for (auto& pair : obj.edges)
        {
            Sombrero::FastVector3& pos1 = obj.vertices[pair.first];
            Sombrero::FastVector3& pos2 = obj.vertices[pair.second];

            auto lineRenderer =
                ScribbleContainer::get_instance()->InitLineRenderer(brush);

            lineRenderer->set_enabled(true);

            // increase position count so that it can be erased properly
            float dist = pos1.Distance(pos2);

            int posCount = dist > 0.1f ? dist * 20 : 2;

            lineRenderer->set_positionCount(posCount + 1);
            for (int i = 0; i < posCount; i++)
            {
                lineRenderer->SetPosition(
                    i, Sombrero::FastVector3::Lerp(pos1, pos2,
                                                   (float)i / (float)posCount));
            }

            lineRenderer->SetPosition(posCount, pos2);
        }
    }

    custom_types::Helpers::Coroutine
    ScribbleContainer::LoadObjAnimated(SimpleObj obj)
    {
        float delay = 0.004f;
        auto brush = GlobalBrushManager::get_activeBrush()->currentBrush;
        int i = 0;
        int lineCount = obj.edges.size();
        for (auto& edge : obj.edges)
        {
            Sombrero::FastVector3& pos1 = obj.vertices[edge.first];
            Sombrero::FastVector3& pos2 = obj.vertices[edge.second];

            auto lineRenderer =
                ScribbleContainer::get_instance()->InitLineRenderer(brush);

            lineRenderer->set_enabled(true);
            lineRenderer->set_positionCount(3);
            lineRenderer->SetPosition(0, pos1);
            lineRenderer->SetPosition(1, (pos1 + pos2) / 2);
            lineRenderer->SetPosition(2, pos2);

            co_yield reinterpret_cast<System::Collections::IEnumerator*>(
                UnityEngine::WaitForSeconds::New_ctor(((float)i / (float)lineCount) *
                                                      delay));
        }

        animatedLoadRoutine = nullptr;
        co_return;
    }

    void ScribbleContainer::Reset()
    {
        auto scribbles =
            UnityEngine::Resources::FindObjectsOfTypeAll<ScribbleContainer*>();
        if (scribbles && scribbles.Length() > 0)
            UnityEngine::Object::DestroyImmediate(
                scribbles[0]->get_gameObject());
        instance = nullptr;
    }

    void LinerendererData::Deserialize(std::ifstream& reader)
    {
        auto brush = CustomBrush::Deserialize(reader);
        int posCount;
        reader.read(reinterpret_cast<char*>(&posCount), sizeof(int));
        auto lineRenderer =
            ScribbleContainer::get_instance()->InitLineRenderer(brush);
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

        auto base = lineRenderer->get_transform()->get_position();
        auto positions = lineRenderer->GetPositions();
        // write the amount of positions out
        int posCount = positions.size();
        writer.write(reinterpret_cast<const char*>(&posCount), sizeof(int));
        // write out all the positions, a pos is basically 3 floats
        for (auto& pos : positions)
        {
            auto actual = pos + base;
            writer.write(reinterpret_cast<const char*>(&actual),
                         sizeof(Sombrero::FastVector3));
        }
    }

    LinerendererData* LinerendererData::Create(Scribble::LineRenderer* linerenderer,
                                               const CustomBrush& brush)
    {
        auto data = *il2cpp_utils::New<LinerendererData*>();
        data->lineRenderer = linerenderer;
        data->brush.copy(brush);
        return data;
    }

} // namespace Scribble