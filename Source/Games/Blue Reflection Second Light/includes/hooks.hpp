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
extern float2 projection_jitters;
extern float2 output_resolution;
uintptr_t* CameraData = nullptr;

inline SafetyHookInline g_compute_projection_matrix_hook;

__int64 __fastcall Hooked_ComputeProjectionMatrix(__int64 a1, float a2, float a3, float a4, float a5, char a6, float a7);