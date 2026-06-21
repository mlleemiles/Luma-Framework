#include "Includes/Common.hlsl"

Texture2D<float4> smplVelocity_Tex : register(t0);
Texture2D<float4> smplDepth_Tex : register(t1);
RWTexture2D<float2> g_updatedVelocityTex : register(u0);

// 3Dmigoto declarations
#define cmp -


[numthreads(8, 8, 1)]
void main(uint2 tid : SV_DispatchThreadID, uint3 gid : SV_GroupId, uint gix : SV_GroupIndex)
{
	if(any(tid >= uint2(LumaData.GameData.ViewportSize.xy)))
	{
		return;
	}
	
	float2 pixelUV = ((float2)tid + 0.5f) * LumaData.GameData.ViewportSize.zw;
	
	float4 r0,r1,r2,r3;
	uint4 bitmask, uiDest;
	float4 fDest;

	r0.x = smplDepth_Tex.Load(int3((int2)tid.xy, 0)).x;
	r0.y = pixelUV.x * 2 + -1;
	r0.z = 1 + -pixelUV.y;
	r0.z = r0.z * 2 + -1;

	float4 clip_pos = float4(r0.yzx, 1.0f);
	clip_pos = mul(clip_pos, LumaData.GameData.CurrentProjectionInverseMatrix);
	clip_pos /= clip_pos.w;
	float4 world_pos = mul(clip_pos, LumaData.GameData.CurrentViewInverseMatrix);
	r0 = world_pos;

	r1.xyz = LumaData.GameData.PreviousViewProjectionMatrix._m10_m11_m13 * r0.yyy;
	r1.xyz = r0.xxx * LumaData.GameData.PreviousViewProjectionMatrix._m00_m01_m03 + r1.xyz;
	r1.xyz = r0.zzz * LumaData.GameData.PreviousViewProjectionMatrix._m20_m21_m23 + r1.xyz;
	r1.xyz = r0.www * LumaData.GameData.PreviousViewProjectionMatrix._m30_m31_m33 + r1.xyz;
	r1.xy = r1.xy / r1.zz;
	r1.zw = pixelUV.xy * float2(2,-2) + float2(-1,1);
	r1.xy = r1.zw + -r1.xy;

	float2 uvVelocity;
	uvVelocity.x =  r1.x * 0.5;
	uvVelocity.y = -r1.y * 0.5;

	float4 velocity = smplVelocity_Tex.Load(int3((int2)tid.xy, 0));
	if (velocity.y == -1.f)
	{
		velocity.xy = uvVelocity.xy;
	}

	float2 jitter_delta = LumaData.GameData.PrevJitters - LumaData.GameData.CurrJitters;
	velocity.xy -= jitter_delta;
  
	g_updatedVelocityTex[tid] = 100.0;
}