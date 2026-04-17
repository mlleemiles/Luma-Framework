#pragma once
#include <cstddef>
#include <cstdint>

template<typename T>
static inline T* ResolveRipRelative(void* instr, std::ptrdiff_t dispOffset, std::size_t instrSize)
{
   auto base = reinterpret_cast<uintptr_t>(instr);
   auto rel = *reinterpret_cast<int32_t*>(base + dispOffset);
   auto dest = base + instrSize + rel;
   return reinterpret_cast<T*>(dest);
}

enum AAOptions {
   OPTION_NO_AA,
   OPTION_FXAA,
   OPTION_SMAA = 5,
   OPTION_SMAA_T2X
};

extern uintptr_t* AAOptionBase;

AAOptions GetAAOption();