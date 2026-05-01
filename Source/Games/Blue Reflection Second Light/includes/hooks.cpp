#include "..\..\Core\core.hpp"
#include "hooks.hpp"

__int64 __fastcall Hooked_ComputeProjectionMatrix(__int64 a1, float a2, float a3, float a4, float a5, char a6, float a7)
{
    auto original_result = g_compute_projection_matrix_hook
    .unsafe_call<__int64>(a1, a2, a3, a4, a5, a6, a7);
    
    if (a3 != 1.0)
    {
        // the matrix is transposed
        *(float*)(a1 + 0x20) = projection_jitters.x * 2.0 / output_resolution.x;
        *(float*)(a1 + 0x24) = projection_jitters.y * -2.0 / output_resolution.y;
    }
    
    return original_result;
}