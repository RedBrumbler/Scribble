#include "Utils/ThumbnailHelper.hpp"
#include "logging.hpp"

#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/RenderTexture.hpp"
#include "UnityEngine/ImageConversion.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RenderTextureFormat.hpp"
#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/RenderTextureReadWrite.hpp"
#include "UnityEngine/AudioListener.hpp"
#include "UnityEngine/MeshCollider.hpp"
#include "UnityEngine/StereoTargetEyeMask.hpp"
#include "LIV/SDK/Unity/LIV.hpp"

using namespace UnityEngine;

namespace Scribble::ThumbnailHelper
{
    Texture2D* GetThumbnail(Camera* camera, int width, int height)
    {
        if (!camera) return nullptr;
        auto rt = RenderTexture::GetTemporary(width, height, 24, RenderTextureFormat::Default, RenderTextureReadWrite::Default);
        camera->set_targetTexture(rt);
        auto screenShot = Texture2D::New_ctor(width, height, TextureFormat::RGB24, false);
        camera->Render();
        // save prev RT
        auto prev = RenderTexture::get_active();
        RenderTexture::set_active(rt);
        screenShot->ReadPixels(Rect(0, 0, width, height), 0, 0);
        camera->set_targetTexture(nullptr);
        // set prev RT
        RenderTexture::set_active(prev);
        RenderTexture::ReleaseTemporary(rt);
        return screenShot;
    }

    template<typename T, typename W = ::ArrayW<T>, class Ptr = ::Array<T>*>
    Ptr get_ptr(W&& wrapper) {
        return (Ptr)wrapper;
    }

    void WriteThumbnail(std::ofstream& writer, Camera* camera, int width, int height)
    {
        auto screenshot = GetThumbnail(camera, width, height);
        auto data = ImageConversion::EncodeToPNG(screenshot);
        writer.write(reinterpret_cast<const char*>(get_ptr<uint8_t>(data)->values), data.Length());
        Object::Destroy(screenshot);
    }

    Camera* CreateCamera(Sombrero::FastVector3&& position, Sombrero::FastVector3&& rotation)
    {
        //GameObject go = new GameObject("ThumbnailCam");
        auto go = GameObject::Instantiate(Camera::get_main()->get_gameObject());
        go->SetActive(false);
        while (go->get_transform()->get_childCount() > 0)
            Object::DestroyImmediate(go->get_transform()->GetChild(0)->get_gameObject());

        Object::DestroyImmediate(go->GetComponent<AudioListener*>());
        Object::DestroyImmediate(go->GetComponent<MeshCollider*>());
        auto camera = go->GetComponent<Camera*>();
        camera->set_stereoTargetEye(StereoTargetEyeMask::None);
        static constexpr const int cullingMask = 1 << 30;
        camera->set_cullingMask(cullingMask);
        camera->set_fieldOfView(60.0f);
        camera->set_enabled(true);
        auto liv = camera->GetComponent<LIV::SDK::Unity::LIV*>();
        if (liv) Object::Destroy(liv);
        go->SetActive(true);
        go->get_transform()->set_position(position);
        go->get_transform()->set_rotation(Quaternion::Euler(rotation));
        return camera;
    }
    /*
    // reverses the order of 4 bytes
    void reverse(uint8_t* half)
    {
        uint8_t temp = half[0];
        half[0] = half[3];
        half[3] = temp;
        temp = half[1];
        half[1] = half[2];
        half[2] = temp;
    }
*/
    bool CheckPngData(std::ifstream& reader, long& size, bool skip)
    {
        // using std::array because, and I quote:
        // just believe me when i tell you that your life will be much nicer if you use std::array and avoid C arrays like the plague
        // ~Danike
        std::array<uint8_t, 8> pngHeader {{137,80,78,71,13,10,26,10}};
        std::array<uint8_t, 8> foundHeader;

        // save original position
        long pos = reader.tellg();

        // get stream total size
        reader.seekg(0, reader.end);
        long streamsize = reader.tellg();
        reader.seekg(pos, reader.beg);
        INFO("Stream length: %lu", streamsize);
        // read header data
        reader.read(reinterpret_cast<char*>(&foundHeader[0]), foundHeader.size());
        
        if (foundHeader != pngHeader)
        {
            ERROR("Invalid png header");
            reader.seekg(pos, reader.beg);
            return false;
        }

        int num1;
        int num2;
        
        std::array<uint8_t, 4> half1;

        bool flag = true;
        while (flag)
        {
            reader.read(reinterpret_cast<char*>(&half1[0]), half1.size());
            std::reverse(half1.begin(), half1.end());
            num1 = (*(int*)&half1[0]);
            reader.read(reinterpret_cast<char*>(&num2), sizeof(int));
            if (num2 == 1145980233)
            {
                flag = false;
            }
            if (num1 + 4 > streamsize - reader.tellg())
            {
                reader.seekg(pos, reader.beg);
                return false;
            }
            reader.seekg(num1 + 4, reader.cur);
        }
        size = (long)reader.tellg() - pos;
        INFO("size %ld found", size);
        if (!skip)
        {
            reader.seekg(pos, reader.beg);
        }
        return true;
    }

    UnityEngine::Texture2D* ReadPNG(std::ifstream& reader, const long& size)
    {
        Array<uint8_t>* bytes = Array<uint8_t>::NewLength(size);
        reader.read(reinterpret_cast<char*>(bytes->values), size);
        auto texture = Texture2D::New_ctor(0, 0, TextureFormat::RGBA32, false, false);
        ImageConversion::LoadImage(texture, bytes, false);
        texture->set_wrapMode(TextureWrapMode::Clamp);
        return texture;
    }
}