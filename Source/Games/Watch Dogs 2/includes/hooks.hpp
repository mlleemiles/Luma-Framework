#pragma once
#include <cstddef>
#include <cstdint>

#include "safetyhook.hpp"

template<typename T>
static inline T* ResolveRipRelative(void* instr, std::ptrdiff_t dispOffset, std::size_t instrSize)
{
   auto base = reinterpret_cast<uintptr_t>(instr);
   auto rel = *reinterpret_cast<int32_t*>(base + dispOffset);
   auto dest = base + instrSize + rel;
   return reinterpret_cast<T*>(dest);
}

struct CCameraMatrices {
   Math::Matrix44F m_viewMatrix; // 0x0  (size 0x40)
   Math::Matrix44F m_viewMatrixInverse; // 0x40  (size 0x40)
   Math::Matrix44F m_viewMatrixPure; // 0x80  (size 0x40)
   Math::Matrix44F m_projectionMatrix; // 0xC0  (size 0x40)
   Math::Matrix44F m_projectionMatrixPure; // 0x100  (size 0x40)
   Math::Matrix44F m_projectionMatrixInverse; // 0x140  (size 0x40)
   Math::Matrix44F m_viewProjectionMatrix; // 0x180  (size 0x40)
   Math::Matrix44F m_viewProjectionMatrixInverse; // 0x1C0  (size 0x40)
};

struct CCamera
{
   CCameraMatrices m_cameraMatrix; // 0x0  (size 0x200)
   bool m_useAngles; // 0x200  (size 0x1)
   uint8_t _pad_201[0x3]; // padding
   float m_Angles[3]; // 0x204  (size 0xC)
   float m_ProjectionRatio; // 0x210  (size 0x4)
   float m_projectedSizeFactor; // 0x214  (size 0x4)
   float m_orthoProjectionSizeFactor; // 0x218  (size 0x4)
   int32_t m_ProjType; // 0x21C  (size 0x4)
   float m_position[3]; // 0x220  (size 0xC)
   float  m_frontVector[3]; // 0x22C  (size 0xC)
   float m_upVector[3]; // 0x238  (size 0xC)
   float m_leftVector[3]; // 0x244  (size 0xC)
   float m_nearClipDistance; // 0x250  (size 0x4)
   float m_farClipDistance; // 0x254  (size 0x4)
   float m_FOV; // 0x258  (size 0x4)
   uint8_t m_cullingFrustum[0x180];// 0x25C  (size 0x180)
};

struct CControlCamera
{
   CCamera m_camera; // 0x0  (size 0x3E0)
   float m_ViewSurfaceX; // 0x3E0  (size 0x4)
   float m_ViewSurfaceZ; // 0x3E4  (size 0x4)
   float m_userDirection[3]; // 0x3E8  (size 0xC)
   float m_userUp[3]; // 0x3F4  (size 0xC)
   float m_verticalFOV; // 0x400  (size 0x4)
   float m_jitter[2]; // 0x404  (size 0x8)
   uint8_t _pad_end[0x14]; // final padding (total size 0x420)
};

struct CSceneViewportPrivateDataMotionBlur
{
   CControlCamera m_lastPreviousCamera;
   CControlCamera m_lastCurrentCamera;
   float m_lastGameDeltaTime;
   uint8_t __pad[0xC];
};

struct CSceneViewportPrivateData
{
   uint64_t field_0;
   int m_viewportPosition[2];
   int m_viewportSize[2];
   float m_cullingCameraPositionForAnimSafeLOD[3];
   uint8_t _pad_24[0xC];
   CControlCamera m_cullingCamera;
   CControlCamera m_renderCamera;
   CControlCamera m_renderCameraFull;
   CSceneViewportPrivateDataMotionBlur m_motionBlur;
   uint8_t unknown_field[0x88];
   unsigned int m_renderOnceFrameCount;
   unsigned int m_renderOnceMaxNumFrames;
   unsigned int m_renderCounter;
};

enum AAOptions {
   OPTION_NO_AA,
   OPTION_FXAA,
   OPTION_SMAA = 5,
   OPTION_SMAA_T2X
};

inline SafetyHookInline g_deferred_fx_antialias_renderer_hook;

extern uintptr_t* AAOptionBase;
extern uintptr_t CDeferredFxAntialiasRenderer;
extern uintptr_t* CDeferredFxRendererContext;
extern CSceneViewportPrivateData* m_viewportPrivateData;

AAOptions GetAAOption();
//float GetGameDeltaTime();
//void GetViewportSize();

__int64 __fastcall Hooked_CDeferredFxAntialiasRendererPrepare(__int64 a1, uintptr_t* a2);