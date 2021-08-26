#pragma once

#include "sombrero/shared/Vector3Utils.hpp"

#include "UnityEngine/LineRenderer.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
namespace Scribble
{
    // sc2ad will fucking strangle me if he sees this
    // better not show him then
    class LineRenderer : public UnityEngine::LineRenderer
    {
        public:
            enum LineTextureMode {
                Stretch,
                Tile,
                DistributePerSegment,
                RepeatPerSegment
            };
            
            int get_positionCount()
            {
                using GetPositionCount = function_ptr_t<int, UnityEngine::LineRenderer*>;
                static auto get_positionCount = reinterpret_cast<GetPositionCount>(il2cpp_functions::resolve_icall("UnityEngine.LineRenderer::get_positionCount"));
                return get_positionCount(this);
            }

            Sombrero::FastVector3 GetPosition(int index)
            {
                using GetPosition = function_ptr_t<void, LineRenderer*, int, ByRef<Sombrero::FastVector3>>;
                static auto GetPosition_Injected = reinterpret_cast<GetPosition>(il2cpp_functions::resolve_icall("UnityEngine.LineRenderer::GetPosition_Injected"));
                Sombrero::FastVector3 pos;
                GetPosition_Injected(this, index, byref(pos));
                return pos;
            }

            std::vector<Sombrero::FastVector3> GetPositions()
            {
                int length = get_positionCount();
                std::vector<Sombrero::FastVector3> positions;
                for (int i = 0; i < length; i++){
                    positions.push_back(GetPosition(i));
                }
                return positions;
            }

            void GetPositions(Array<Sombrero::FastVector3>* out)
            {
                int length = out->Length();
                for (int i = 0; i < length; i++){
                    out->values[i] = GetPosition(i);
                }
            }

            void set_widthMultiplier(float value)
            {
                using SetMultiplier = function_ptr_t<void, UnityEngine::LineRenderer*, float>;
                static auto SetMultiplierIcall = reinterpret_cast<SetMultiplier>(il2cpp_functions::resolve_icall("UnityEngine.LineRenderer::set_widthMultiplier"));
                SetMultiplierIcall(this, value);
            }

            void set_numCornerVertices(int num)
            {
                using SetCornerNum = function_ptr_t<void, UnityEngine::LineRenderer*, int>;
                static auto SetCornerIcall = reinterpret_cast<SetCornerNum>(il2cpp_functions::resolve_icall("UnityEngine.LineRenderer::set_numCornerVertices"));
                SetCornerIcall(this, num);
            }

            void  set_numCapVertices(int num)
            {
                using SetCapNum = function_ptr_t<void, UnityEngine::LineRenderer*, int>;
                static auto SetCapIcall = reinterpret_cast<SetCapNum>(il2cpp_functions::resolve_icall("UnityEngine.LineRenderer::set_numCapVertices"));
                SetCapIcall(this, num);
            }

            void set_textureMode(const LineTextureMode& mode)
            {
                using SetLineTextureMode = function_ptr_t<void, UnityEngine::LineRenderer*, int>;
                static auto SetModeIcall = reinterpret_cast<SetLineTextureMode>(il2cpp_functions::resolve_icall("UnityEngine.LineRenderer::set_textureMode"));
                SetModeIcall(this, (int)mode);
            }
    };
}
// now it's also a linerenderer pog
DEFINE_IL2CPP_ARG_TYPE(Scribble::LineRenderer*, "UnityEngine", "LineRenderer");