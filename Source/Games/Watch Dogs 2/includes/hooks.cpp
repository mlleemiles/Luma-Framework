#include "hooks.hpp"

uintptr_t* AAOptionBase = nullptr;

AAOptions GetAAOption()
{
   return *(AAOptions*)(*(uintptr_t*)(*AAOptionBase) + 0x3A4);
}