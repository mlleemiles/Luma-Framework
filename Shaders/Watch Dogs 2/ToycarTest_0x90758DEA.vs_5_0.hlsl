#include "Includes/Common.hlsl"

cbuffer Viewport : register(b0)
{
  float4 CameraNearPlaneSize : packoffset(c0);
  float4x4 DepthTextureTransform : packoffset(c1);
  float4 FSMClipPlanes : packoffset(c5);
  float4 FacettedShadowCastParams : packoffset(c6);
  float4 FogValues0 : packoffset(c7);
  float4 FogValues1 : packoffset(c8);
  float4x4 InvProjectionMatrix : packoffset(c9);
  float4x4 InvProjectionMatrixDepth : packoffset(c13);
  float4x3 InvViewMatrix : packoffset(c17);
  float4x4 PreviousViewProjectionMatrix : packoffset(c20);
  float4x4 ProjectionMatrix : packoffset(c24);
  float4 RainOcclusionFadeParams : packoffset(c28);
  float4x4 RainOcclusionProjectionMatrix : packoffset(c29);
  float4 RainOcclusionShadowMapSize : packoffset(c33);
  float4 ReflectionVolumeDebugColors[15] : packoffset(c34);
  float4 VPosOffset : packoffset(c49);
  float4 VPosScale : packoffset(c50);
  float4x3 ViewMatrix : packoffset(c51);
  float4x4 ViewProjectionMatrix : packoffset(c54);
  float4x4 ViewRotProjectionMatrix : packoffset(c58);
  float4x4 ViewRotProjectionMatrixPure : packoffset(c62);
  float4 ViewportSize : packoffset(c66);

  struct
  {
    float near;
    float far;
    float view;
    float oneOverView;
  } CameraDistances : packoffset(c67);


  struct
  {
    float4x4 inverseTransform;
    float3 rcpFadeRangePositive;
    float textureArrayIndexAsFloat;
    float3 rcpFadeRangeNegative;
    float fadeFactor;
    float2 multipliers;
    uint parallaxCorrection;
    float padding0;
  } ReflectionVolumes[15] : packoffset(c68);

  float3 CameraDirection : packoffset(c173);
  float DefaultReflectionTextureArrayIndexAsFloat : packoffset(c173.w);
  float3 CameraPosition : packoffset(c174);
  float DynamicCubeMapReflectionTextureMaxMipIndex : packoffset(c174.w);
  float3 CullingCameraPosition : packoffset(c175);
  float ExposedWhitePointOverExposureScale : packoffset(c175.w);
  float3 FogColorVector : packoffset(c176);
  float ExposureScale : packoffset(c176.w);
  float3 OppositeFogColorDelta : packoffset(c177);
  float MaxParaboloidReflectionMipIndex : packoffset(c177.w);
  float3 SideFogColor : packoffset(c178);
  float MaxStaticReflectionMipIndex : packoffset(c178.w);
  float3 SunFogColorDelta : packoffset(c179);
  float MeasuredExposureScale : packoffset(c179.w);
  float3 TemporalFilteringParams : packoffset(c180);
  float RaindropRippleScale : packoffset(c180.w);
  float3 UncompressDepthWeights : packoffset(c181);
  float ReflectionScaleDistanceMul : packoffset(c181.w);
  float3 UncompressDepthWeightsWS : packoffset(c182);
  float ReflectionScaleStrength : packoffset(c182.w);
  float3 ViewPoint : packoffset(c183);
  float SkyParaboloidTextureMaxMipIndex : packoffset(c183.w);
  float2 DefaultReflectionMultipliers : packoffset(c184);
  bool UseOnlySkyReflection : packoffset(c184.z);
  float2 ReflectionGIControl : packoffset(c185);
  uint2 SelectedPixel : packoffset(c185.z);
}

cbuffer SceneGraphicObjectInstancePart : register(b1)
{
  float4x3 WorldMatrix : packoffset(c0);
}

cbuffer MeshLightsModifier : register(b2)
{
  float4 MeshLightsColors[26] : packoffset(c0);
}

cbuffer SceneGeometry : register(b3)
{
  float4 GeometryPickingID : packoffset(c0);
  float4 GeometryUserData : packoffset(c1);
  float4 UVDecompression : packoffset(c2);

  struct
  {
    float positionMin;
    float positionRange;
    float meshLocalHeight;
    float isBuildingFacadeInterior;
  } MeshDecompression : packoffset(c3);

  float3 GeometryBBoxMax : packoffset(c4);
  float3 GeometryBBoxMin : packoffset(c5);
}

cbuffer Skinning : register(b4)
{
  float4x3 BlendMatrices[511] : packoffset(c0);
  float4x3 PrevBlendMatrices[511] : packoffset(c1533);
}

cbuffer MaterialWD2Generic : register(b5)
{
  float4 MaterialPickingID : packoffset(c0);
  float4 AlphaUVTiling1 : packoffset(c1);
  float4 ColorizeParams : packoffset(c2);
  float4 DiffuseUVTiling1 : packoffset(c3);
  float4 EmissiveIntensity : packoffset(c4);
  float4 EmissiveUVTiling : packoffset(c5);
  float4 GenericParameters : packoffset(c6);
  float4 MaskTextureUVTiling1 : packoffset(c7);
  float4 NormalUVTiling1 : packoffset(c8);
  float4 PatternTexture1Size : packoffset(c9);
  float3 DiffuseColor1 : packoffset(c10);
  float ContrastTopSand : packoffset(c10.w);
  float3 DiffuseColor2 : packoffset(c11);
  float Duration : packoffset(c11.w);
  float3 DiffuseColorTopSand : packoffset(c12);
  float FramesPerSecond : packoffset(c12.w);
  float3 EmissiveColor : packoffset(c13);
  float MaskChannelUsageAlpha : packoffset(c13.w);
  float3 SeparateOpacity : packoffset(c14);
  float MaskChannelUsageBlue : packoffset(c14.w);
  float2 AlphaTextureTiling1 : packoffset(c15);
  float MaskChannelUsageGreen : packoffset(c15.z);
  float MaskChannelUsageRed : packoffset(c15.w);
  float2 AnimTexture1Size : packoffset(c16);
  float Metalness : packoffset(c16.z);
  float NormalIntensity : packoffset(c16.w);
  float2 EmissiveFadeoutParams : packoffset(c17);
  float NormalIntensityTopSand : packoffset(c17.z);
  float NumberOfFrames : packoffset(c17.w);
  float2 PatternTextureTiling1 : packoffset(c18);
  float Opacity : packoffset(c18.z);
  float Reflectance : packoffset(c18.w);
  float ReflectionRoughnessBias : packoffset(c19);
  float Roughness : packoffset(c19.y);
  float ThresholdTopSand : packoffset(c19.z);
  float Translucency : packoffset(c19.w);
  float VertexAnimationFreq1 : packoffset(c20);
  float VertexAnimationFreq2 : packoffset(c20.y);
  float VertexAnimationFreq3 : packoffset(c20.z);
  float VertexAnimationRadius1 : packoffset(c20.w);
  float VertexAnimationRadius2 : packoffset(c21);
  float VertexAnimationRadius3 : packoffset(c21.y);
  float VertexAnimationSpeed1 : packoffset(c21.z);
  float VertexAnimationSpeed2 : packoffset(c21.w);
  float VertexAnimationSpeed3 : packoffset(c22);
  int EmissiveType : packoffset(c22.y);
  int SpecialMode : packoffset(c22.z);
  int TextureArraySize : packoffset(c22.w);
  int TextureMappingType : packoffset(c23);
  int UseAsMaskChannel : packoffset(c23.y);
  int VertexAnimationAxis : packoffset(c23.z);
  int VertexAnimationType : packoffset(c23.w);
  int WetSurfaceTypeIndex : packoffset(c24);
  bool Colorize : packoffset(c24.y);
  bool EmissiveAsMask : packoffset(c24.z);
  bool RandomUVOffset : packoffset(c24.w);
  bool TextureMappingFlipWorld : packoffset(c25);
  bool UseAsMask : packoffset(c25.y);
}

cbuffer PreviousWorldTransform : register(b6)
{
  float4x3 PreviousWorldMatrix : packoffset(c0);
}

Buffer<uint4> SkinningConfig : register(t0);


// 3Dmigoto declarations
#define cmp -


void main(
  int4 v0 : position0,
  int4 v1 : texcoord2,
  float4 v2 : blendweight0,
  float4 v3 : normal0,
  float4 v4 : color1,
  float4 v5 : texcoord1,
  float4 v6 : texcoord0,
  out float4 o0 : TEXCOORD0,
  out float4 o1 : TEXCOORD1,
  out float4 o2 : TEXCOORD2,
  out float4 o3 : TEXCOORD3,
  out float4 o4 : TEXCOORD4,
  out float4 o5 : TEXCOORD5,
  out float4 o6 : TEXCOORD6,
  out float4 o7 : TEXCOORD7,
  out float2 o8 : TEXCOORD8,
  out float4 o9 : SV_Position0)
{
  const float4 icb[] = { { 1.000000, 0, 0, 0},
                              { 0, 1.000000, 0, 0},
                              { 0, 0, 1.000000, 0},
                              { 0, 0, 0, 1.000000} };
  float4 r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13;
  uint4 bitmask, uiDest;
  float4 fDest;

  r0.xyz = (int3)v0.xyz;
  r1.xyzw = (int4)v1.xyzw;
  if (3 == 0) r0.w = 0; else if (3+13 < 32) {   r0.w = (uint)v0.w << (32-(3 + 13)); r0.w = (uint)r0.w >> (32-3);  } else r0.w = (uint)v0.w >> 13;
  r0.w = (int)r0.w + 1;
  r2.x = cmp((int)r0.w == 1);
  if (r2.x != 0) {
    r2.x = (int)v0.w & 8191;
    r3.xyzw = float4(1,0,0,0);
    r2.yzw = float3(0,0,0);
  } else {
    r4.x = (int)v0.w & 8191;
    r4.xyzw = SkinningConfig.Load(r4.x).xyzw;
    r2.xyzw = (int4)r4.xyzw & int4(0xffff,0xffff,0xffff,0xffff);
    r3.xyzw = v2.zyxw;
  }
  r4.xyz = r0.xyz * MeshDecompression.positionRange + MeshDecompression.positionMin;
  r1.xyzw = r1.xyzw * UVDecompression.zwzw + UVDecompression.xyxy;
  r0.xyz = v3.zyx * float3(2,2,2) + float3(-1,-1,-1);
  r5.xyz = v5.zyx * float3(2,2,2) + float3(-1,-1,-1);
  r6.xyz = v6.zyx * float3(2,2,2) + float3(-1,-1,-1);
  r7.xyzw = float4(0,0,0,0);
  r8.xyzw = float4(0,0,0,0);
  r9.xyzw = float4(0,0,0,0);
  r10.x = 0;
  while (true) {
    r5.w = cmp((uint)r10.x < (uint)r0.w);
    r6.w = cmp((uint)r10.x < 4);
    r5.w = r5.w ? r6.w : 0;
    if (r5.w == 0) break;
    r5.w = dot(r3.xyzw, icb[r10.x+0].xyzw);
    r6.w = -(int)r10.x;
    r11.xyz = cmp((uint3)r10.xxx < int3(1,2,3));
    r12.y = r11.y ? r6.w : 0;
    r10.xy = (int2)r10.xx + int2(1,-3);
    r12.z = r11.y ? 0 : r10.y;
    r12.w = cmp((int)r11.z == 0);
    r12.x = r11.x;
    r11.xyzw = r2.xyzw ? r12.xyzw : 0;
    r10.yz = (int2)r11.yw | (int2)r11.xz;
    r6.w = (int)r10.z | (int)r10.y;
    r6.w = (int)r6.w * 3;
    r11.x = BlendMatrices[r6.w]._m30 * r5.w;
    r11.y = BlendMatrices[r6.w]._m31 * r5.w;
    r11.z = BlendMatrices[r6.w]._m32 * r5.w;
    r7.xyz = r5.www * BlendMatrices[r6.w]._m00_m10_m20 + r7.xyz;
    r8.xyz = r5.www * BlendMatrices[r6.w]._m01_m11_m21 + r8.xyz;
    r9.xyz = r5.www * BlendMatrices[r6.w]._m02_m12_m22 + r9.xyz;
    r12.x = r7.w;
    r12.y = r8.w;
    r12.z = r9.w;
    r10.yzw = r12.xyz + r11.xyz;
    r7.w = r10.y;
    r8.w = r10.z;
    r9.w = r10.w;
  }
  r4.w = 1;
  r10.x = dot(r4.xyzw, r7.xyzw);
  r10.y = dot(r4.xyzw, r8.xyzw);
  r10.z = dot(r4.xyzw, r9.xyzw);
  r11.x = dot(r0.xyz, r7.xyz);
  r11.y = dot(r0.xyz, r8.xyz);
  r11.z = dot(r0.xyz, r9.xyz);
  r0.x = dot(r5.xyz, r7.xyz);
  r0.y = dot(r5.xyz, r8.xyz);
  r0.z = dot(r5.xyz, r9.xyz);
  r5.x = dot(r6.xyz, r7.xyz);
  r5.y = dot(r6.xyz, r8.xyz);
  r5.z = dot(r6.xyz, r9.xyz);
  r6.xyzw = float4(0,0,0,0);
  r7.xyzw = float4(0,0,0,0);
  r8.xyzw = float4(0,0,0,0);
  r9.x = 0;
  while (true) {
    r5.w = cmp((uint)r9.x < (uint)r0.w);
    r9.z = cmp((uint)r9.x < 4);
    r5.w = r5.w ? r9.z : 0;
    if (r5.w == 0) break;
    r5.w = dot(r3.xyzw, icb[r9.x+0].xyzw);
    r9.z = -(int)r9.x;
    r12.xyz = cmp((uint3)r9.xxx < int3(1,2,3));
    r13.y = r12.y ? r9.z : 0;
    r9.xy = (int2)r9.xx + int2(1,-3);
    r13.z = r12.y ? 0 : r9.y;
    r13.w = cmp((int)r12.z == 0);
    r13.x = r12.x;
    r12.xyzw = r2.xyzw ? r13.xyzw : 0;
    r9.yz = (int2)r12.yw | (int2)r12.xz;
    r9.y = (int)r9.z | (int)r9.y;
    r9.y = (int)r9.y * 3;
    r6.xyzw = r5.wwww * PrevBlendMatrices[r9.y]._m00_m10_m20_m30 + r6.xyzw;
    r7.xyzw = r5.wwww * PrevBlendMatrices[r9.y]._m01_m11_m21_m31 + r7.xyzw;
    r8.xyzw = r5.wwww * PrevBlendMatrices[r9.y]._m02_m12_m22_m32 + r8.xyzw;
  }
  r2.x = dot(r4.xyzw, r6.xyzw);
  r2.y = dot(r4.xyzw, r7.xyzw);
  r2.z = dot(r4.xyzw, r8.xyzw);
  r3.x = dot(r11.xyz, WorldMatrix._m00_m10_m20);
  r3.y = dot(r11.xyz, WorldMatrix._m01_m11_m21);
  r3.z = dot(r11.xyz, WorldMatrix._m02_m12_m22);
  r4.x = dot(r0.xyz, WorldMatrix._m00_m10_m20);
  r4.y = dot(r0.xyz, WorldMatrix._m01_m11_m21);
  r4.z = dot(r0.xyz, WorldMatrix._m02_m12_m22);
  r0.x = dot(r5.xyz, WorldMatrix._m00_m10_m20);
  r0.y = dot(r5.xyz, WorldMatrix._m01_m11_m21);
  r0.z = dot(r5.xyz, WorldMatrix._m02_m12_m22);
  r10.w = 1;
  r5.x = dot(r10.xyzw, WorldMatrix._m00_m10_m20_m30);
  r5.y = dot(r10.xyzw, WorldMatrix._m01_m11_m21_m31);
  r5.z = dot(r10.xyzw, WorldMatrix._m02_m12_m22_m32);
  r6.xyz = -CameraPosition.xyz + r5.xyz;
  r7.x = ViewRotProjectionMatrix._m00 * r6.x;
  r7.y = ViewRotProjectionMatrix._m01 * r6.x;
  r7.z = ViewRotProjectionMatrix._m02 * r6.x;
  r7.w = ViewRotProjectionMatrix._m03 * r6.x;
  r8.x = ViewRotProjectionMatrix._m10 * r6.y;
  r8.y = ViewRotProjectionMatrix._m11 * r6.y;
  r8.z = ViewRotProjectionMatrix._m12 * r6.y;
  r8.w = ViewRotProjectionMatrix._m13 * r6.y;
  r9.x = r6.z * ViewRotProjectionMatrix._m20 + ViewRotProjectionMatrix._m30;
  r9.y = r6.z * ViewRotProjectionMatrix._m21 + ViewRotProjectionMatrix._m31;
  r9.z = r6.z * ViewRotProjectionMatrix._m22 + ViewRotProjectionMatrix._m32;
  r9.w = r6.z * ViewRotProjectionMatrix._m23 + ViewRotProjectionMatrix._m33;
  r6.xyzw = r9.xyzw + r8.xyzw;
  r6.xyzw = r7.xyzw + r6.xyzw;
  r0.w = cmp(0 < r3.z);
  r3.w = cmp(r3.z < 0);
  r0.w = (int)-r0.w + (int)r3.w;
  r7.z = (int)r0.w;
  r7.xy = float2(0,0);
  r8.xyz = r7.xyz * r3.zxy;
  r7.xyz = r3.yzx * r7.yzy + -r8.xyz;
  r0.w = dot(r7.yz, r7.yz);
  r0.w = rsqrt(r0.w);
  r7.xyz = r7.xyz * r0.www;
  r5.w = -r5.y;
  r8.xyzw = TextureMappingFlipWorld ? r5.wxwx : r5.xwxw;
  r7.w = -r7.y;
  r9.xy = TextureMappingFlipWorld ? r7.wx : r7.xy;
  r0.w = cmp(TextureMappingType == 2);
  r10.x = dot(r8.zw, r1.xy);
  r9.zw = float2(-1,1) * r1.yx;
  r10.y = dot(r8.zw, r9.zw);
  r11.x = dot(r9.xy, r1.xy);
  r11.y = dot(r9.xy, r9.zw);
  r8.xyzw = r0.wwww ? r10.xyxy : r8.xyzw;
  r7.xy = r0.ww ? r11.xy : r9.xy;
  r9.xyz = r7.zxy * r3.yzx;
  r9.xyz = r7.yzx * r3.zxy + -r9.xyz;
  r0.w = dot(r9.xyz, r9.xyz);
  r0.w = rsqrt(r0.w);
  r9.xyz = r9.xyz * r0.www;
  r1.xyzw = TextureMappingType ? r8.xyzw : r1.xyzw;
  o3.xyz = TextureMappingType ? r9.xyz : r4.xyz;
  o2.xyz = TextureMappingType ? r7.xyz : r0.xyz;
  r0.xyzw = DiffuseUVTiling1.xyzw * r1.xyzw;
  o7.xy = r0.xy + r0.zw;
  r0.x = cmp(0.00196078443 < v5.w);
  if (r0.x != 0) {
    r0.x = v5.w * 255 + -0.5;
    r0.x = (int)r0.x;
    r0.xyz = MeshLightsColors[r0.x].xyz / MeasuredExposureScale;
  } else {
    r0.xyz = float3(0,0,0);
  }
  r0.xyz = EmissiveIntensity.www * r0.xyz;
  r4.x = WorldMatrix._m30;
  r4.y = WorldMatrix._m31;
  r4.z = WorldMatrix._m32;
  r4.xyz = CameraPosition.xyz + -r4.xyz;
  r0.w = dot(r4.xyz, r4.xyz);
  r0.w = sqrt(r0.w);
  r0.w = -EmissiveFadeoutParams.x + r0.w;
  r3.w = EmissiveFadeoutParams.y + -EmissiveFadeoutParams.x;
  r3.w = max(9.99999975e-006, r3.w);
  r0.w = saturate(r0.w / r3.w);
  r0.w = 1 + -r0.w;
  r0.w = r0.w * r0.w;
  r0.w = r0.w * r0.w;
  o0.xyz = r0.xyz * r0.www;
  r0.xyzw = NormalUVTiling1.xyzw * r1.xyzw;
  o8.xy = r0.xy + r0.zw;
  r0.xyzw = MaskTextureUVTiling1.xyzw * r1.xyzw;
  o7.zw = r0.xy + r0.zw;
  r2.w = 1;
  r0.x = dot(r2.xyzw, PreviousWorldMatrix._m00_m10_m20_m30);
  r0.y = dot(r2.xyzw, PreviousWorldMatrix._m01_m11_m21_m31);
  r0.z = dot(r2.xyzw, PreviousWorldMatrix._m02_m12_m22_m32);
  r0.w = 1;
  
  r0.xyz -= LumaData.GameData.PreviousCameraPosition.xyz;
  
  r1.x = dot(r0.xyzw, LumaData.GameData.PreviousViewRotProjectionMatrix._m00_m10_m20_m30);
  r1.y = dot(r0.xyzw, LumaData.GameData.PreviousViewRotProjectionMatrix._m01_m11_m21_m31);
  o6.z = dot(r0.xyzw, LumaData.GameData.PreviousViewRotProjectionMatrix._m03_m13_m23_m33);
  
  o6.xy = float2(0.5,-0.5) * r1.xy;
  o9.xyzw = r6.xyzw;
  o1.xyz = r3.xyz;
  o4.xyz = r5.xyz;
  o5.xyz = float3(0.5,-0.5,1) * r6.xyw;
  return;
}