#include "..\..\Core\core.hpp"
#include "hooks.hpp"

uintptr_t* AAOptionBase = nullptr;
uintptr_t CDeferredFxAntialiasRenderer = 0;
uintptr_t* CDeferredFxRendererContext = nullptr;
CSceneViewportPrivateData* m_viewportPrivateData = nullptr;

AAOptions GetAAOption()
{
   return *(AAOptions*)(*(uintptr_t*)(*AAOptionBase) + 0x3A4);
}

__int64 __fastcall Hooked_CDeferredFxAntialiasRendererPrepare(__int64 a1, uintptr_t* a2)
{
   if (a1)
   {
      CDeferredFxAntialiasRenderer = a1;
   }
   
   if (a2 && a2[13])
   {
      uintptr_t base = a2[13];
      m_viewportPrivateData = reinterpret_cast<CSceneViewportPrivateData*>(base);
   }

   return g_deferred_fx_antialias_renderer_hook
       .unsafe_call<__int64>(a1, a2);
}