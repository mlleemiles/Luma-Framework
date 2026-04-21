#define GAME_WATCH_DOGS_2 1

#define ENABLE_NGX 1
// Hooking a debugger is forbidden
#define DISABLE_AUTO_DEBUGGER 1
#define DEBUG_LOG 0

#define ENABLE_ORIGINAL_SHADERS_MEMORY_EDITS 1

#include "..\..\Core\core.hpp"
#include "..\..\Core\includes\shader_patching.h"
#include "includes\hooks.hpp"
#include "includes\safetyhook.hpp"
#include "includes\hooks.cpp"

namespace
{
   inline bool IsWine()
   {
      static void* pwine_get_version;
      HMODULE hntdll = GetModuleHandle(TEXT("ntdll.dll"));
      if (!hntdll)
      {
         return false;
      }

      pwine_get_version = (void*)GetProcAddress(hntdll, "wine_get_version");

      if (!pwine_get_version)
      {
         return false;
      }

      return true;
   }

   void PatchVMProtect()
   {
      DWORD oldProtect = 0;
      auto ntdll = GetModuleHandleA("ntdll.dll");
      if (IsWine())
      {
         auto nt_vp = (BYTE*)GetProcAddress(ntdll, "NtProtectVirtualMemory");
         auto nt_vp_offset = (uintptr_t)nt_vp - (uintptr_t)ntdll + 4;
         char nt_vp_syscall;

         std::ifstream infile("C:\\Windows\\System32\\ntdll.dll", std::ios::binary);
         infile.seekg(nt_vp_offset);
         infile.get(nt_vp_syscall);

         BYTE restore[] = {0x4C, 0x8B, 0xD1, 0xB8, static_cast<BYTE>(nt_vp_syscall)};
         VirtualProtect(nt_vp, sizeof(restore), PAGE_EXECUTE_READWRITE, &oldProtect);
         memcpy(nt_vp, restore, sizeof(restore));
         VirtualProtect(nt_vp, sizeof(restore), oldProtect, &oldProtect);
      }
      else
      {
         BYTE callcode = ((BYTE*)GetProcAddress(ntdll, "NtQuerySection"))[4] - 1;
         BYTE restore[] = {0x4C, 0x8B, 0xD1, 0xB8, callcode};
         auto nt_vp = (BYTE*)GetProcAddress(ntdll, "NtProtectVirtualMemory");
         VirtualProtect(nt_vp, sizeof(restore), PAGE_EXECUTE_READWRITE, &oldProtect);
         memcpy(nt_vp, restore, sizeof(restore));
         VirtualProtect(nt_vp, sizeof(restore), oldProtect, &oldProtect);
      }
   }

   float2 projection_jitters = {0, 0};
   
   static float2 jit[2] = {
      {0, 0}, {0, 0}
   };
   
   void JitterUpdate()
   {
      auto index = cb_luma_global_settings.FrameIndex % 8;
      jit[0].x = SR::HaltonSequence(index, 2);
      jit[0].y = SR::HaltonSequence(index, 3);
      jit[1].x = jit[0].x;
      jit[1].y = jit[0].y;
      
      std::memcpy((void*)JitterTableOffset, &jit, sizeof(jit));
   }
   
   DirectX::XMMATRIX ComputeCameraSpaceToPreviousProjectedSpaceMatrix()
   {
      using namespace DirectX;
	   
      XMMATRIX inv_view_matrix_current = XMLoadFloat4x4(
         reinterpret_cast<const XMFLOAT4X4*>(&m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_camera.m_cameraMatrix.m_viewMatrixInverse));
      XMMATRIX inv_view_matrix_prev = XMLoadFloat4x4(
         reinterpret_cast<const XMFLOAT4X4*>(&m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_camera.m_cameraMatrix.m_viewMatrixInverse));
      XMMATRIX proj_prev = XMLoadFloat4x4(
         reinterpret_cast<const XMFLOAT4X4*>(&m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_camera.m_cameraMatrix.m_projectionMatrix));
		   
      proj_prev = XMMatrixTranspose(proj_prev);
      
      float4 position_delta = {0.0,0.0,0.0,1.0};
      position_delta.x = inv_view_matrix_current.r[3].m128_f32[0] - inv_view_matrix_prev.r[3].m128_f32[0];
      position_delta.y = inv_view_matrix_current.r[3].m128_f32[1] - inv_view_matrix_prev.r[3].m128_f32[1];
      position_delta.z = inv_view_matrix_current.r[3].m128_f32[2] - inv_view_matrix_prev.r[3].m128_f32[2];
      
      inv_view_matrix_current = XMMatrixTranspose(inv_view_matrix_current);
      inv_view_matrix_prev = XMMatrixTranspose(inv_view_matrix_prev);
      
      XMMATRIX view_rotation_prev = inv_view_matrix_prev;
      view_rotation_prev.r[0].m128_f32[3] = 0.0;
      view_rotation_prev.r[1].m128_f32[3] = 0.0;
      view_rotation_prev.r[2].m128_f32[3] = 0.0;
      view_rotation_prev.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      
      {
         XMVECTOR c0 = view_rotation_prev.r[0];
         XMVECTOR c1 = view_rotation_prev.r[1];
         XMVECTOR c2 = view_rotation_prev.r[2];

         view_rotation_prev.r[0] = XMVectorSet(XMVectorGetX(c0), XMVectorGetX(c1), XMVectorGetX(c2), 0.0f);
         view_rotation_prev.r[1] = XMVectorSet(XMVectorGetY(c0), XMVectorGetY(c1), XMVectorGetY(c2), 0.0f);
         view_rotation_prev.r[2] = XMVectorSet(XMVectorGetZ(c0), XMVectorGetZ(c1), XMVectorGetZ(c2), 0.0f);
         view_rotation_prev.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      }
      
      XMMATRIX inv_view_delta_matrix_current = inv_view_matrix_current;
      inv_view_delta_matrix_current.r[0].m128_f32[3] = position_delta.x;
      inv_view_delta_matrix_current.r[1].m128_f32[3] = position_delta.y;
      inv_view_delta_matrix_current.r[2].m128_f32[3] = position_delta.z;
      
      XMMATRIX temp = XMMatrixMultiply(proj_prev, view_rotation_prev);
      return XMMatrixMultiply(temp, inv_view_delta_matrix_current);
   }
   
   DirectX::XMMATRIX ComputePreviousViewRotProjectionMatrix()
   {
      using namespace DirectX;
      
      XMMATRIX view_matrix_prev = XMLoadFloat4x4(
         reinterpret_cast<const XMFLOAT4X4*>(&m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_camera.m_cameraMatrix.m_viewMatrix));
      XMMATRIX proj_prev = XMLoadFloat4x4(
         reinterpret_cast<const XMFLOAT4X4*>(&m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_camera.m_cameraMatrix.m_projectionMatrix));

      proj_prev = XMMatrixTranspose(proj_prev);
      view_matrix_prev.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      view_matrix_prev = XMMatrixTranspose(view_matrix_prev);
      /*
      view_matrix_prev.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      {
         XMVECTOR c0 = view_matrix_prev.r[0];
         XMVECTOR c1 = view_matrix_prev.r[1];
         XMVECTOR c2 = view_matrix_prev.r[2];

         view_matrix_prev.r[0] = XMVectorSet(XMVectorGetX(c0), XMVectorGetX(c1), XMVectorGetX(c2), 0.0f);
         view_matrix_prev.r[1] = XMVectorSet(XMVectorGetY(c0), XMVectorGetY(c1), XMVectorGetY(c2), 0.0f);
         view_matrix_prev.r[2] = XMVectorSet(XMVectorGetZ(c0), XMVectorGetZ(c1), XMVectorGetZ(c2), 0.0f);
         view_matrix_prev.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      }
      */
      return XMMatrixMultiply(proj_prev, view_matrix_prev);
   }

   ShaderHashesList shader_hashes_ColorGradingLUT;
   ShaderHashesList shader_hashes_TemporalFiltering;
   ShaderHashesList shader_hashes_SMAA_Reprojection;
   ShaderHashesList shader_hashes_SMAA_EdgeDectction;
   ShaderHashesList shader_hashes_CopyDpeth;
   ShaderHashesList shader_hashes_TemporalResolve;
   ShaderHashesList shader_hashes_TemporalAA;
   ShaderHashesList shader_hashes_WaterGridVectorMap;
}

struct GameDeviceDataWatchDogs2 final : public GameDeviceData
{
   // resources used to identify the deferred context used for scene drawing
   com_ptr<ID3D11CommandList> remainder_command_list;
   std::atomic<ID3D11DeviceContext*> draw_device_context = nullptr;

   // textures we got from the game
   com_ptr<ID3D11Texture2D> source_color;
   com_ptr<ID3D11ShaderResourceView> source_color_srv;
   com_ptr<ID3D11Texture2D> depth_texture;
   com_ptr<ID3D11ShaderResourceView> depth_texture_srv;
   com_ptr<ID3D11UnorderedAccessView> depth_texture_uav;
   com_ptr<ID3D11Texture2D> motion_vectors;
   com_ptr<ID3D11ShaderResourceView> motion_vectors_srv;
   com_ptr<ID3D11Buffer> viewport_cbv;
   com_ptr<ID3D11ShaderResourceView> sr_output_color_srv;
   
   com_ptr<ID3D11Texture2D> decoded_motion_vectors;
   com_ptr<ID3D11UnorderedAccessView> decoded_motion_vectors_uav;

   // the command list we split to interject dlss
   com_ptr<ID3D11CommandList> partial_command_list;

   com_ptr<ID3D11Buffer> modifiable_index_vertex_buffer;
   
   std::mutex game_device_data_mutex;

	DirectX::XMMATRIX CameraSpaceToPreviousProjectedSpace;
	DirectX::XMMATRIX PreviousViewRotProjectionMatrix;
   
   float2 render_resolution;
   
   void CleanMVResources()
   {
      depth_texture = nullptr;
      depth_texture_uav = nullptr;
      decoded_motion_vectors = nullptr;
      decoded_motion_vectors_uav = nullptr;
   }
};

class WatchDogs2 final : public Game
{
   static GameDeviceDataWatchDogs2& GetGameDeviceData(DeviceData& device_data)
   {
      return *static_cast<GameDeviceDataWatchDogs2*>(device_data.game);
   }

   static const GameDeviceDataWatchDogs2& GetGameDeviceData(const DeviceData& device_data)
   {
      return *static_cast<const GameDeviceDataWatchDogs2*>(device_data.game);
   }

public:
   void OnInit(bool async) override
   {
      HMODULE engine_module = nullptr;
      while (!engine_module)
      {
         engine_module = GetModuleHandleA("Disrupt_64.dll");
         Sleep(100);
      }
      auto base_addr = (uintptr_t)engine_module;
      auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(engine_module);
      auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::byte*>(engine_module) + dos_header->e_lfanew);
      std::size_t section_size = nt_headers->OptionalHeader.SizeOfImage;
      
      JitterTableOffset = base_addr + 0x3E3B5C8;;

      auto WILDCARD = System::BytePattern(System::BytePattern::WildcardType::Wildcard);

      std::vector<System::BytePattern> pattern = {
         0x48, 0x89, 0x05,
         WILDCARD, WILDCARD, WILDCARD, WILDCARD,
         0x48, 0x8B, 0x87
      };

      auto results = System::ScanMemoryForPattern(
         reinterpret_cast<std::byte*>(engine_module),
         section_size,
         pattern
         );

      if (!results.empty())
      {
         AAOptionBase = ResolveRipRelative<uintptr_t>(results[0], 3, 7);
      }

      pattern = {
         0x49, 0x89, 0xE3, 0x55, 0x56, 0x57, 0x41, 0x56, 0x48, 0x8D, 0x6C, 0x24
      };

      results = System::ScanMemoryForPattern(
         reinterpret_cast<std::byte*>(engine_module),
         section_size,
         pattern
         );
#if DEBUG_LOG
      for (auto addr : results)
      {
         std::stringstream s;
         s << "Candidate: 0x" << std::hex << (uintptr_t)addr;
         reshade::log::message(reshade::log::level::info, s.str().c_str());
      }
#endif
      if (!results.empty() && !g_deferred_fx_antialias_renderer_hook)
      {
         void* fn = reinterpret_cast<void*>(results[0]);

         g_deferred_fx_antialias_renderer_hook = safetyhook::create_inline(
            fn,
            Hooked_CDeferredFxAntialiasRendererPrepare
            );

         if (g_deferred_fx_antialias_renderer_hook)
         {
            reshade::log::message(reshade::log::level::info, "Hook installed successfully");
         }
         else
         {
            reshade::log::message(reshade::log::level::error, "Failed to create inline hook");
         }
      }
      
      native_shaders_definitions.emplace(CompileTimeStringHash("Decode Motion Vector"), ShaderDefinition{"Luma_DecodeMotionVector", reshade::api::pipeline_subobject_type::compute_shader});

      std::vector<ShaderDefineData> game_shader_defines_data = {
         {"ENABLE_DITHER", '0', true, false, "Allows disabling the game's 8 bit dithering effect (luma disables it by default as it's all HDR)"},
      };
      shader_defines_data.append_range(game_shader_defines_data);
      GetShaderDefineData(POST_PROCESS_SPACE_TYPE_HASH).SetDefaultValue('1'); // Game was all linear, rendering is R16G16B16A16_FLOAT and post processing + UI is R8G8B8A8_UNORM_SRGB or B8G8R8A8_UNORM_SRGB.
      GetShaderDefineData(GAMMA_CORRECTION_TYPE_HASH).SetDefaultValue('0');   // Game seemengly looks better (less crush, less unnatural shadow) in sRGB than 2.2
      GetShaderDefineData(UI_DRAW_TYPE_HASH).SetDefaultValue('2');
   }

   void OnLoad(std::filesystem::path& file_path, bool failed) override
   {
      if (!failed)
      {
         reshade::register_event<reshade::addon_event::execute_secondary_command_list>(WatchDogs2::OnExecuteSecondaryCommandList);
      }
   }

   void OnCreateDevice(ID3D11Device* native_device, DeviceData& device_data) override
   {
      device_data.game = new GameDeviceDataWatchDogs2;
   }

   std::unique_ptr<std::byte[]> ModifyShaderByteCode(const std::byte* code, size_t& size, reshade::api::pipeline_subobject_type type, uint64_t shader_hash, const std::byte* shader_object, size_t shader_object_size) override
   {
      if (type != reshade::api::pipeline_subobject_type::compute_shader)
         return nullptr;

      std::unique_ptr<std::byte[]> new_code = nullptr;

      // This compute shader was unsafe, it was reading and writing to the same coordinates of the same resources, from different threads at the same time, hence it needs some barriers to be added
      // Credits to Nukem, Blisto, doitsujin and pendingchaos for helping figure it out.
      if (shader_hash != 0x28BA3808)
      {
         return new_code;
      }

      std::vector<uint8_t> appended_patch;
      std::vector<const std::byte*> appended_patches_addresses;

      // Matches "AllMemoryBarrierWithGroupSync()" ("sync_uglobal_g_t" in asm)
      constexpr uint32_t flags =
         D3D11_SB_SYNC_THREADS_IN_GROUP |
         D3D11_SB_SYNC_THREAD_GROUP_SHARED_MEMORY |
         D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GROUP |
         D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GLOBAL;
      uint32_t opcode_token =
         ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_SYNC) |
         ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(1) |
         ENCODE_D3D11_SB_SYNC_FLAGS(flags);
#if 1 // TODOFT: test... we got "sync_sat_uglobal_g_t" otherwise?
      // make 100% sure SAT is off (paranoia, but harmless)
      opcode_token &= ~D3D10_SB_INSTRUCTION_SATURATE_MASK;
#endif
      std::vector<uint32_t> opcode_token_patch = std::vector<uint32_t>{opcode_token};

      appended_patch.insert(appended_patch.end(), reinterpret_cast<uint8_t*>(opcode_token_patch.data()), reinterpret_cast<uint8_t*>(opcode_token_patch.data()) + opcode_token_patch.size() * sizeof(uint32_t));

      size_t size_u32 = size / sizeof(uint32_t);
      const uint32_t* code_u32 = reinterpret_cast<const uint32_t*>(code);
      size_t i = 0;
      while (i < size_u32)
      {
         uint32_t opcode_token = code_u32[i];
         D3D10_SB_OPCODE_TYPE opcode_type = DECODE_D3D10_SB_OPCODE_TYPE(opcode_token);
         size_t instruction_size = opcode_type == D3D10_SB_OPCODE_CUSTOMDATA ? code_u32[i + 1] : DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(opcode_token); // Includes itself

         if (opcode_type == D3D10_SB_OPCODE_IF)
         {
            // Add the patch before every single branch value.
            // Shift it by how much the data would have been shifted by prior patches we already added.
            size_t i_add = appended_patches_addresses.size() * appended_patch.size() / sizeof(uint32_t); // Patches should always be a multiple of DWORD
            appended_patches_addresses.emplace_back(reinterpret_cast<const std::byte*>(&code_u32[i + i_add]));
         }

         i += instruction_size;
         if (instruction_size == 0)
            break;
      }

      // Insert the patch for each address
      if (!appended_patches_addresses.empty())
      {
         new_code = std::make_unique<std::byte[]>(size + appended_patch.size() * appended_patches_addresses.size());

         std::memcpy(new_code.get(), code, size);

         size_t valid_size = size;

         std::unique_ptr<std::byte[]> scratch_buffer = std::make_unique<std::byte[]>(size + appended_patch.size() * appended_patches_addresses.size());

         for (const auto appended_patches_address : appended_patches_addresses)
         {
            size_t insert_pos = appended_patches_address - code; // These are already shifted to account for the previously inserted patches

            // Copy from the address we'll insert the patch at, until the end, into a temporary buffer
            std::memcpy(scratch_buffer.get(), new_code.get() + insert_pos, valid_size - insert_pos);
            // Insert the patch
            std::memcpy(new_code.get() + insert_pos, appended_patch.data(), appended_patch.size());
            // Fill back the previous data, shifted
            std::memcpy(new_code.get() + insert_pos + appended_patch.size(), scratch_buffer.get(), valid_size - insert_pos);

            valid_size += appended_patch.size();
         }

         size = valid_size;
      }

      return new_code;
   }

   static bool CreateSRResources(ID3D11Device* native_device, ID3D11Texture2D* sr_output_color, GameDeviceDataWatchDogs2& game_device_data, D3D11_TEXTURE2D_DESC desc)
   {
      if (game_device_data.sr_output_color_srv.get())
      {
         return true;
      }

      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
      srv_desc.Format = desc.Format;
      srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srv_desc.Texture2D.MostDetailedMip = 0;
      srv_desc.Texture2D.MipLevels = 1;
      HRESULT hr = native_device->CreateShaderResourceView(sr_output_color, &srv_desc, &game_device_data.sr_output_color_srv);
      if (FAILED(hr))
      {
         std::stringstream s;
         s << "SR: SRV Creation Failed";
         reshade::log::message(reshade::log::level::info, s.str().c_str());
         return false;
      }

      return true;
   }
   
   static bool CreateMVResources(ID3D11Device* native_device, GameDeviceDataWatchDogs2& game_device_data)
   {
      // Return early if resources already exist with correct dimensions
      bool output_changed = game_device_data.render_resolution.x != (float)m_viewportPrivateData->m_viewportSize[0]
      || game_device_data.render_resolution.y != (float)m_viewportPrivateData->m_viewportSize[1];
      if (game_device_data.decoded_motion_vectors.get())
      {
         if (!output_changed)
         {
            return true;
         }
         game_device_data.CleanMVResources();
      }
      
      HRESULT hr;
      
      D3D11_TEXTURE2D_DESC depth_desc = {};
      depth_desc.Width = m_viewportPrivateData->m_viewportSize[0];
      depth_desc.Height = m_viewportPrivateData->m_viewportSize[1];
      depth_desc.MipLevels = 1;
      depth_desc.ArraySize = 1;
      depth_desc.Format = DXGI_FORMAT_R16G16_FLOAT;
      depth_desc.SampleDesc.Count = 1;
      depth_desc.Usage = D3D11_USAGE_DEFAULT;
      depth_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
      
      hr = native_device->CreateTexture2D(&depth_desc, nullptr, &game_device_data.decoded_motion_vectors);
      if (FAILED(hr))
      {
         std::stringstream s;
         s << "MV: Texture Creation Failed";
         reshade::log::message(reshade::log::level::info, s.str().c_str());
         return false;
      }
      
      depth_desc.Format = DXGI_FORMAT_R32_FLOAT;
      hr = native_device->CreateTexture2D(&depth_desc, nullptr, &game_device_data.depth_texture);
      if (FAILED(hr))
      {
         std::stringstream s;
         s << "Depth: Texture Creation Failed";
         reshade::log::message(reshade::log::level::info, s.str().c_str());
         return false;
      }
      
      D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
      uav_desc.Format = DXGI_FORMAT_R16G16_FLOAT;
      uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
      uav_desc.Texture2D.MipSlice = 0;
      hr = native_device->CreateUnorderedAccessView(game_device_data.decoded_motion_vectors.get(), &uav_desc, &game_device_data.decoded_motion_vectors_uav);
      if (FAILED(hr))
      {
         std::stringstream s;
         s << "MV: UAV Creation Failed";
         reshade::log::message(reshade::log::level::info, s.str().c_str());
         return false;
      }
      
      uav_desc.Format = DXGI_FORMAT_R32_FLOAT;
      hr = native_device->CreateUnorderedAccessView(game_device_data.depth_texture.get(), &uav_desc, &game_device_data.depth_texture_uav);
      if (FAILED(hr))
      {
         std::stringstream s;
         s << "Depth: UAV Creation Failed";
         reshade::log::message(reshade::log::level::info, s.str().c_str());
         return false;
      }

      return true;
   }

   static void LogXMMatrix(const char* label, const DirectX::XMMATRIX& matrix, 
                    reshade::log::level level = reshade::log::level::debug)
   {
#if DEBUG_LOG      
      std::stringstream s;
      s << label << ":\n";
    
      for (int row = 0; row < 4; ++row)
      {
         s << "  [";
         for (int col = 0; col < 4; ++col)
         {
            // Format: fixed-point, 4 decimals, width 8 for alignment
            s << std::fixed << std::setprecision(8) << std::setw(12) 
              << matrix.r[row].m128_f32[col];
            if (col < 3) s << ", ";
         }
         s << "]";
         if (row < 3) s << "\n";
      }
    
      reshade::log::message(level, s.str().c_str());
#endif
   }
   
   DrawOrDispatchOverrideType OnDrawOrDispatch(ID3D11Device* native_device, ID3D11DeviceContext* native_device_context, CommandListData& cmd_list_data, DeviceData& device_data, reshade::api::shader_stage stages, const ShaderHashesList<OneShaderPerPipeline>& original_shader_hashes, bool is_custom_pass, bool& updated_cbuffers, std::function<void()>* original_draw_dispatch_func) override
   {
      // Make sure the swapchain copy shader always and only targets the swapchain RT, otherwise we'd need to branch in it!
      if (is_custom_pass && (stages & reshade::api::shader_stage::compute) != 0 && original_shader_hashes.Contains(shader_hashes_ColorGradingLUT))
      {
         // We need access to a linear sampler in the customized version of this CS, so add it (and make sure it's not overlapping with any other used slot, so we don't pollute the state)
         ID3D11SamplerState* const sampler_state_linear = device_data.sampler_state_linear.get();
         native_device_context->CSSetSamplers(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1, 1, &sampler_state_linear);

         return DrawOrDispatchOverrideType::None;
      }

      if ((stages & reshade::api::shader_stage::compute) != 0 && original_shader_hashes.Contains(shader_hashes_TemporalFiltering))
      {
         static bool has_sent_tf_warning = false;
         if (!has_sent_tf_warning && MessageBoxA(NULL, "Temporal Filtering is broken in Watch Dogs 2 depending on your GPU, Luma suggests against using it.", "Temporal Filtering detected", MB_OK | MB_SETFOREGROUND) == IDOK)
         {
            has_sent_tf_warning = true;
         }
      }

      auto& game_device_data = GetGameDeviceData(device_data);
      
      if (original_shader_hashes.Contains(shader_hashes_WaterGridVectorMap))
      {
         if (CDeferredFxAntialiasRenderer && device_data.sr_type != SR::Type::None)
         {
            native_device_context->Draw(4, 0);
            // split the command list since DLSS must be executed on an immediate context
            native_device_context->FinishCommandList(TRUE, &game_device_data.partial_command_list);
            game_device_data.draw_device_context = native_device_context;
            return DrawOrDispatchOverrideType::Replaced;
         }
      }
      else if (original_shader_hashes.Contains(shader_hashes_TemporalAA))
      {
         if (device_data.sr_type != SR::Type::None)
         {
            com_ptr<ID3D11ShaderResourceView> srvs[3];
            com_ptr<ID3D11Buffer> cbv;
            native_device_context->PSGetShaderResources(0, 3, &srvs[0]);
            native_device_context->PSGetConstantBuffers(0, 1, &cbv);
            
            {
               std::lock_guard<std::mutex> lock(game_device_data.game_device_data_mutex);

               reshade::log::message(reshade::log::level::info, "Getting depth resource");
               game_device_data.depth_texture_srv = srvs[0];

               reshade::log::message(reshade::log::level::info, "Getting source color resource");
               game_device_data.source_color_srv = srvs[1];

               reshade::log::message(reshade::log::level::info, "Getting motion vector resource");
               game_device_data.motion_vectors_srv = srvs[2];
               
               reshade::log::message(reshade::log::level::info, "Getting viewport cbuffer");
               game_device_data.viewport_cbv = cbv;
            }
         }
      }
      else if (original_shader_hashes.Contains(shader_hashes_TemporalResolve))
      {
         if (device_data.sr_type != SR::Type::None)
         {
            return DrawOrDispatchOverrideType::Skip;
         }
      }

      return DrawOrDispatchOverrideType::None;
   }

   static void OnExecuteSecondaryCommandList(reshade::api::command_list* cmd_list, reshade::api::command_list* secondary_cmd_list)
   {
      com_ptr<ID3D11DeviceChild> primary_child = reinterpret_cast<ID3D11DeviceChild*>(cmd_list->get_native());
      com_ptr<ID3D11DeviceChild> secondary_child = reinterpret_cast<ID3D11DeviceChild*>(secondary_cmd_list->get_native());
      if (!primary_child || !secondary_child)
         return;

      auto& device_data = *cmd_list->get_device()->get_private_data<DeviceData>();
      auto& game_device_data = GetGameDeviceData(device_data);

      com_ptr<ID3D11DeviceContext> deferred_ctx;
      com_ptr<ID3D11CommandList> finished_cmd_list;
      if (SUCCEEDED(secondary_child->QueryInterface(&deferred_ctx)) && deferred_ctx)
      {
         // This is a FinishCommandList call - capture the remainder command list
         primary_child->QueryInterface(&finished_cmd_list);
         if (finished_cmd_list && deferred_ctx == game_device_data.draw_device_context)
         {
#if DEBUG_LOG
            reshade::log::message(reshade::log::level::info, "Partial SMAA command list finished (deferred ctx)");
#endif
            game_device_data.remainder_command_list = finished_cmd_list;
            game_device_data.draw_device_context = nullptr;
         }
         return;
      }

      com_ptr<ID3D11DeviceContext> immediate_ctx;
      com_ptr<ID3D11CommandList> exec_cmd_list;
      if (SUCCEEDED(primary_child->QueryInterface(&immediate_ctx)) && immediate_ctx)
      {
         secondary_child->QueryInterface(&exec_cmd_list);
         if (exec_cmd_list == game_device_data.remainder_command_list && game_device_data.partial_command_list)
         {
            immediate_ctx->ExecuteCommandList(game_device_data.partial_command_list.get(), FALSE);
#if DEBUG_LOG
            reshade::log::message(reshade::log::level::info, "Partial SMAA command list executed (immediate ctx)");
#endif

            game_device_data.partial_command_list.reset();
            game_device_data.remainder_command_list.reset();

            //std::lock_guard<std::mutex> lock(game_device_data.game_device_data_mutex);
            const bool dlss_inputs_valid = 
               game_device_data.source_color_srv.get() != nullptr
            && game_device_data.depth_texture_srv.get() != nullptr
            && game_device_data.motion_vectors_srv.get() != nullptr;

            if (!dlss_inputs_valid)
            {
               reshade::log::message(reshade::log::level::info, "No source color resource");
               return;
            }
            
            DrawStateStack<DrawStateStackType::FullGraphics> draw_state_stack;
            DrawStateStack<DrawStateStackType::Compute> compute_state_stack;
            draw_state_stack.Cache(immediate_ctx.get(), device_data.uav_max_count);
            compute_state_stack.Cache(immediate_ctx.get(), device_data.uav_max_count);
            
            {
               if (game_device_data.source_color_srv.get() != nullptr)
               {
                  com_ptr<ID3D11Resource> color_resource;
                  game_device_data.source_color_srv->GetResource(&color_resource);
                  HRESULT hr = color_resource->QueryInterface(&game_device_data.source_color);
               
                  if (FAILED(hr))
                  {
                     reshade::log::message(reshade::log::level::info, "Failed source_color");
                  }
               }
            
               if (game_device_data.motion_vectors_srv.get() != nullptr)
               {
                  com_ptr<ID3D11Resource> mv_resource;
                  game_device_data.motion_vectors_srv->GetResource(&mv_resource);
                  HRESULT hr = mv_resource->QueryInterface(&game_device_data.motion_vectors);
               
                  if (FAILED(hr))
                  {
                     reshade::log::message(reshade::log::level::info, "Failed motion_vectors");
                  }
               }
            }

            if (CDeferredFxAntialiasRenderer && dlss_inputs_valid && device_data.sr_type != SR::Type::None && !device_data.has_drawn_sr)
            {
               game_device_data.CameraSpaceToPreviousProjectedSpace = ComputeCameraSpaceToPreviousProjectedSpaceMatrix();
               game_device_data.PreviousViewRotProjectionMatrix = ComputePreviousViewRotProjectionMatrix();
               
               CommandListData& cmd_list_data = *cmd_list->get_private_data<CommandListData>();
               SetLumaConstantBuffers(immediate_ctx.get(), cmd_list_data, device_data, reshade::api::shader_stage::compute, LumaConstantBufferType::LumaSettings);
               
               auto* sr_instance_data = device_data.GetSRInstanceData();
               {
#if DEBUG_LOG
                  reshade::log::message(reshade::log::level::info, "Setting SR");
#endif
                  SR::SettingsData settings_data;
                  settings_data.output_width = m_viewportPrivateData->m_viewportSize[0];
                  settings_data.output_height = m_viewportPrivateData->m_viewportSize[1];
                  settings_data.render_width = m_viewportPrivateData->m_viewportSize[0];
                  settings_data.render_height = m_viewportPrivateData->m_viewportSize[1];
                  settings_data.dynamic_resolution = false;
                  settings_data.hdr = true;
                  settings_data.inverted_depth = true;
                  settings_data.mvs_jittered = false;
                  settings_data.auto_exposure = device_data.sr_type != SR::Type::FSR;
                  settings_data.mvs_x_scale = -(float)m_viewportPrivateData->m_viewportSize[0];
                  settings_data.mvs_y_scale = -(float)m_viewportPrivateData->m_viewportSize[1];
                  settings_data.render_preset = dlss_render_preset;
                  sr_implementations[device_data.sr_type]->UpdateSettings(sr_instance_data, immediate_ctx.get(), settings_data);
               }

               {
                  device_data.force_reset_sr = false;
                  projection_jitters.x = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_jitter[0] * (float)m_viewportPrivateData->m_viewportSize[0];
                  projection_jitters.y = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_jitter[1] * (float)m_viewportPrivateData->m_viewportSize[1];
               }

               D3D11_TEXTURE2D_DESC taa_output_texture_desc;
               game_device_data.source_color->GetDesc(&taa_output_texture_desc);

               bool skip_dlss = taa_output_texture_desc.Width < sr_instance_data->min_resolution || taa_output_texture_desc.Height < sr_instance_data->min_resolution;
               bool dlss_output_changed = false;

               {
                  D3D11_TEXTURE2D_DESC dlss_output_texture_desc = taa_output_texture_desc;
                  dlss_output_texture_desc.Width = m_viewportPrivateData->m_viewportSize[0];
                  dlss_output_texture_desc.Height = m_viewportPrivateData->m_viewportSize[1];
                  dlss_output_texture_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

                  com_ptr<ID3D11Device> native_device;
                  immediate_ctx->GetDevice(&native_device);

                  if (device_data.sr_output_color.get())
                  {
                     D3D11_TEXTURE2D_DESC prev_dlss_output_texture_desc;
                     device_data.sr_output_color->GetDesc(&prev_dlss_output_texture_desc);
                     dlss_output_changed = prev_dlss_output_texture_desc.Width != dlss_output_texture_desc.Width || prev_dlss_output_texture_desc.Height != dlss_output_texture_desc.Height || prev_dlss_output_texture_desc.Format != dlss_output_texture_desc.Format;
                  }

                  if (!device_data.sr_output_color.get() || dlss_output_changed)
                  {
                     device_data.sr_output_color = nullptr; // Make sure we discard the previous one
                     HRESULT hr = native_device->CreateTexture2D(&dlss_output_texture_desc, nullptr, &device_data.sr_output_color);
                     ASSERT_ONCE(SUCCEEDED(hr));
                  }
                  // Texture creation failed, we can't proceed with DLSS
                  if (!device_data.sr_output_color.get())
                  {
                     skip_dlss = true;
                  }
                  
                  if (CreateMVResources(native_device.get(), game_device_data))
                  {
                     SetLumaConstantBuffers(immediate_ctx.get(), cmd_list_data, device_data, reshade::api::shader_stage::compute, LumaConstantBufferType::LumaData);
                     ID3D11ShaderResourceView* srvs[] = {game_device_data.motion_vectors_srv.get(), game_device_data.depth_texture_srv.get()};
                     ID3D11UnorderedAccessView* uavs[] = {game_device_data.decoded_motion_vectors_uav.get(), game_device_data.depth_texture_uav.get()};
                     ID3D11Buffer* buffers[] = {game_device_data.viewport_cbv.get()};
                     immediate_ctx->CSSetShader(device_data.native_compute_shaders[CompileTimeStringHash("Decode Motion Vector")].get(), nullptr, 0);
                     immediate_ctx->CSSetShaderResources(0, 2, srvs);
                     immediate_ctx->CSSetConstantBuffers(0, 1, buffers);
                     immediate_ctx->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);
               
                     immediate_ctx->Dispatch((m_viewportPrivateData->m_viewportSize[0] + 7) / 8, (m_viewportPrivateData->m_viewportSize[1] + 7) / 8, 1);
                  
                     //unbind the views for bindings
                     uavs[0] = 0;
                     uavs[1] = 0;
                     immediate_ctx->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);
                     srvs[0] = 0;
                     srvs[1] = 0;
                     immediate_ctx->CSSetShaderResources(0, 2, srvs);
                  }
                  else
                  {
                     skip_dlss = true;
                  }
               }
               
               if (!skip_dlss)
               {
#if DEBUG_LOG
                  reshade::log::message(reshade::log::level::info, "Drawing SR");
#endif
                  SR::SuperResolutionImpl::DrawData draw_data;
                  draw_data.source_color = game_device_data.source_color.get();
                  draw_data.output_color = device_data.sr_output_color.get();
                  draw_data.motion_vectors = game_device_data.decoded_motion_vectors.get();
                  draw_data.depth_buffer = game_device_data.depth_texture.get();
                  draw_data.pre_exposure = 0.0f;
                  draw_data.render_width = m_viewportPrivateData->m_viewportSize[0];
                  draw_data.render_height = m_viewportPrivateData->m_viewportSize[1];
                  draw_data.jitter_x = projection_jitters.x;
                  draw_data.jitter_y = -projection_jitters.y;
                  draw_data.vert_fov = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_camera.m_FOV;
                  draw_data.far_plane = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_camera.m_farClipDistance;
                  draw_data.near_plane = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_camera.m_nearClipDistance;
                  draw_data.reset = device_data.force_reset_sr;
                  draw_data.frame_index = cb_luma_global_settings.FrameIndex;//m_viewportPrivateData->m_renderCounter;
                  draw_data.time_delta = m_viewportPrivateData->m_motionBlur.m_lastGameDeltaTime;

                  bool dlss_succeeded = sr_implementations[device_data.sr_type]->Draw(sr_instance_data, immediate_ctx.get(), draw_data);

                  if (dlss_succeeded)
                  {
                     device_data.has_drawn_sr = true;
                  }
                  else
                  {
                     device_data.has_drawn_sr = false;
                     device_data.force_reset_sr = true;
                  }
               }

               if (device_data.has_drawn_sr)
               {
                  immediate_ctx->CopyResource(game_device_data.source_color.get(), device_data.sr_output_color.get());
               }

               draw_state_stack.Restore(immediate_ctx.get());
               compute_state_stack.Restore(immediate_ctx.get());
            }
         }
      }
   }

   void OnPresent(ID3D11Device* native_device, DeviceData& device_data) override
   {
      auto& game_device_data = GetGameDeviceData(device_data);
      
      if (CDeferredFxAntialiasRenderer)
      {
         game_device_data.render_resolution.x = (float)m_viewportPrivateData->m_viewportSize[0];
         game_device_data.render_resolution.y = (float)m_viewportPrivateData->m_viewportSize[1];
         if (device_data.sr_type != SR::Type::None)
         {
            JitterUpdate();
         }
      }
      
      if (!custom_texture_mip_lod_bias_offset && CDeferredFxAntialiasRenderer)
      {
         std::shared_lock shared_lock_samplers(s_mutex_samplers);
         if (device_data.sr_type != SR::Type::None && !device_data.sr_suppressed)
         {
            device_data.texture_mip_lod_bias_offset = SR::GetMipLODBias((float)m_viewportPrivateData->m_viewportSize[1], (float)m_viewportPrivateData->m_viewportSize[1]); // This results in -1 at output res
         }
         else
         {
            device_data.texture_mip_lod_bias_offset = 0.0f;
         }
      }

      // release all resources from the game we got this frame
      // game_device_data.partial_command_list.reset();
      game_device_data.remainder_command_list.reset();
      game_device_data.draw_device_context = nullptr;
      
      {
         std::lock_guard<std::mutex> lock(game_device_data.game_device_data_mutex);
         game_device_data.source_color.reset();
         game_device_data.motion_vectors.reset();
         game_device_data.source_color_srv = nullptr;
         game_device_data.depth_texture_srv = nullptr;
         game_device_data.motion_vectors_srv = nullptr;
         game_device_data.viewport_cbv = nullptr;
      }

      device_data.has_drawn_sr = false;
      
      device_data.cb_luma_global_settings_dirty = true;
      int32_t sr_type = static_cast<int32_t>(device_data.sr_type);
      cb_luma_global_settings.SRType = static_cast<uint32_t>(sr_type + 1);
   }

   void UpdateLumaInstanceDataCB(CB::LumaInstanceDataPadded& data, CommandListData& cmd_list_data, DeviceData& device_data) override
   {
      auto& game_device_data = GetGameDeviceData(device_data);

      memcpy(&data.GameData.CameraSpaceToPreviousProjectedSpace, &game_device_data.CameraSpaceToPreviousProjectedSpace, sizeof(game_device_data.CameraSpaceToPreviousProjectedSpace));
      memcpy(&data.GameData.PreviousViewRotProjectionMatrix, &game_device_data.PreviousViewRotProjectionMatrix, sizeof(game_device_data.PreviousViewRotProjectionMatrix));
      if (CDeferredFxAntialiasRenderer)
      {
         memcpy(&data.GameData.PreviousCameraPosition, &m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_camera.m_position, sizeof(data.GameData.PreviousCameraPosition));
         
         float2 jitter = {0.0, 0.0};
         jitter.x = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_jitter[0] * (float)m_viewportPrivateData->m_viewportSize[0];
         jitter.y = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_jitter[1] * (float)m_viewportPrivateData->m_viewportSize[1];
         memcpy(&data.GameData.CurrJitters, &jitter, sizeof(data.GameData.CurrJitters));
         
         jitter.x = m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_jitter[0] * (float)m_viewportPrivateData->m_viewportSize[0];
         jitter.y = m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_jitter[1] * (float)m_viewportPrivateData->m_viewportSize[1];
         memcpy(&data.GameData.PrevJitters, &jitter, sizeof(data.GameData.PrevJitters));
      }
      
   }
   
   void PrintImGuiAbout() override
   {
      ImGui::Text("Luma for \"Watch Dogs 2\" is developed by Pumbo and is open source and free.\nIf you enjoy it, consider donating.", "");

      const auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
      const auto button_hovered_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
      const auto button_active_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
      ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(70, 134, 0, 255));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(70 + 9, 134 + 9, 0, 255));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(70 + 18, 134 + 18, 0, 255));
      static const std::string donation_link_pumbo = std::string("Buy Pumbo a Coffee on buymeacoffee ") + std::string(ICON_FK_OK);
      if (ImGui::Button(donation_link_pumbo.c_str()))
      {
         system("start https://buymeacoffee.com/realfiloppi");
      }
      static const std::string donation_link_pumbo_2 = std::string("Buy Pumbo a Coffee on ko-fi ") + std::string(ICON_FK_OK);
      if (ImGui::Button(donation_link_pumbo_2.c_str()))
      {
         system("start https://ko-fi.com/realpumbo");
      }
      ImGui::PopStyleColor(3);

      ImGui::NewLine();
      // Restore the previous color, otherwise the state we set would persist even if we popped it
      ImGui::PushStyleColor(ImGuiCol_Button, button_color);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_hovered_color);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_active_color);
#if 0
      static const std::string mod_link = std::string("Nexus Mods Page ") + std::string(ICON_FK_SEARCH);
      if (ImGui::Button(mod_link.c_str()))
      {
         system("start https://www.nexusmods.com/prey2017/mods/149");
      }
#endif
      static const std::string social_link = std::string("Join our \"HDR Den\" Discord ") + std::string(ICON_FK_SEARCH);
      if (ImGui::Button(social_link.c_str()))
      {
         // Unique link for Luma by Pumbo (to track the origin of people joining), do not share for other purposes
         static const std::string obfuscated_link = std::string("start https://discord.gg/J9fM") + std::string("3EVuEZ");
         system(obfuscated_link.c_str());
      }
      static const std::string contributing_link = std::string("Contribute on Github ") + std::string(ICON_FK_FILE_CODE);
      if (ImGui::Button(contributing_link.c_str()))
      {
         system("start https://github.com/Filoppi/Luma-Framework");
      }
      ImGui::PopStyleColor(3);

      ImGui::NewLine();
      ImGui::Text("Credits:"
         "\n\nMain:"
         "\nPumbo"

         "\n\nThird Party:"
         "\nReShade"
         "\nDICE (HDR tonemapper)"
         , "");
   }

   void PrintImGuiInfo(const DeviceData& device_data) override
   {
      ImGui::NewLine();

      if (ImGui::BeginTable("aa_info", 2,
         ImGuiTableFlags_BordersInnerH |
         ImGuiTableFlags_RowBg |
         ImGuiTableFlags_SizingStretchProp))
      {
         ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
         ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
         ImGui::TableHeadersRow();

         // AA Option
         ImGui::TableNextRow();
         ImGui::TableSetColumnIndex(0);
         ImGui::TextUnformatted("AA Option");
         ImGui::TableSetColumnIndex(1);

         if (AAOptionBase && *AAOptionBase)
         {
            AAOptions aa = GetAAOption();

            const char* aa_str = "Unknown";
            switch (aa)
            {
            case OPTION_NO_AA:
               aa_str = "No AA";
               break;
            case OPTION_FXAA:
               aa_str = "FXAA";
               break;
            case OPTION_SMAA:
               aa_str = "SMAA";
               break;
            case OPTION_SMAA_T2X:
               aa_str = "SMAA T2X";
               break;
            default:
               break;
            }

            ImGui::Text("%s (%d)", aa_str, static_cast<int>(aa));
         }
         else
         {
            ImGui::TextUnformatted("N/A");
         }

         // Delta time
         ImGui::TableNextRow();
         ImGui::TableSetColumnIndex(0);
         ImGui::TextUnformatted("Delta Time");
         ImGui::TableSetColumnIndex(1);

         if (CDeferredFxAntialiasRenderer)
         {
            ImGui::Text("%.6f", m_viewportPrivateData->m_motionBlur.m_lastGameDeltaTime);
         }
         else
         {
            ImGui::TextUnformatted("N/A");
         }

         // Viewport Size
         ImGui::TableNextRow();
         ImGui::TableSetColumnIndex(0);
         ImGui::TextUnformatted("Viewport Size");
         ImGui::TableSetColumnIndex(1);

         if (CDeferredFxAntialiasRenderer)
         {
            ImGui::Text("%dx%d", m_viewportPrivateData->m_viewportSize[0], m_viewportPrivateData->m_viewportSize[1]);
         }
         else
         {
            ImGui::TextUnformatted("N/A");
         }

         // Jitter
         ImGui::TableNextRow();
         ImGui::TableSetColumnIndex(0);
         ImGui::TextUnformatted("Jitter Current");
         ImGui::TableSetColumnIndex(1);

         if (CDeferredFxAntialiasRenderer)
         {
            float2 jitter = {0.0, 0.0};
            jitter.x = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_jitter[0] * (float)m_viewportPrivateData->m_viewportSize[0];
            jitter.y = m_viewportPrivateData->m_motionBlur.m_lastCurrentCamera.m_jitter[1] * (float)m_viewportPrivateData->m_viewportSize[1];
            ImGui::Text("%.6f, %.6f", jitter.x, jitter.y);
         }
         else
         {
            ImGui::TextUnformatted("N/A");
         }

         ImGui::TableNextRow();
         ImGui::TableSetColumnIndex(0);
         ImGui::TextUnformatted("Jitter Previous");
         ImGui::TableSetColumnIndex(1);

         if (CDeferredFxAntialiasRenderer)
         {
            float2 jitter = {0.0, 0.0};
            jitter.x = m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_jitter[0] * (float)m_viewportPrivateData->m_viewportSize[0];
            jitter.y = m_viewportPrivateData->m_motionBlur.m_lastPreviousCamera.m_jitter[1] * (float)m_viewportPrivateData->m_viewportSize[1];
            ImGui::Text("%.6f, %.6f", jitter.x, jitter.y);
         }
         else
         {
            ImGui::TextUnformatted("N/A");
         }

         ImGui::EndTable();
      }
   }

};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   if (ul_reason_for_call == DLL_PROCESS_ATTACH)
   {
      Globals::SetGlobals(PROJECT_NAME, "Watch Dogs 2 Luma mod");
      Globals::DEVELOPMENT_STATE = Globals::ModDevelopmentState::Playable;
      Globals::VERSION = 1;

      luma_settings_cbuffer_index = 12; // 13 is used
      luma_data_cbuffer_index = 11;
      
      enable_samplers_upgrade = true;

      swapchain_format_upgrade_type = TextureFormatUpgradesType::AllowedEnabled;
      swapchain_upgrade_type = SwapchainUpgradeType::scRGB;
      texture_format_upgrades_type = TextureFormatUpgradesType::AllowedEnabled;
      texture_upgrade_formats = {
#if 0 // TODO: needed?
         reshade::api::format::r8g8b8a8_unorm,
            reshade::api::format::r8g8b8a8_unorm_srgb,
            reshade::api::format::r8g8b8a8_typeless,
#endif
#if 0 // These are probably not needed (unused) but shouldn't hurt (actually they are!!!)
         reshade::api::format::r8g8b8x8_unorm,
            reshade::api::format::r8g8b8x8_unorm_srgb,
            reshade::api::format::b8g8r8a8_unorm,
            reshade::api::format::b8g8r8a8_unorm_srgb,
            reshade::api::format::b8g8r8a8_typeless,
            reshade::api::format::b8g8r8x8_unorm,
            reshade::api::format::b8g8r8x8_unorm_srgb,
            reshade::api::format::b8g8r8x8_typeless,
#else
         reshade::api::format::r8g8b8a8_typeless,
         reshade::api::format::b8g8r8a8_typeless,
#endif
#if 1
         reshade::api::format::r11g11b10_float,
#endif
      };
      //texture_format_upgrades_2d_size_filters = 0 | (uint32_t)TextureFormatUpgrades2DSizeFilters::SwapchainResolution | (uint32_t)TextureFormatUpgrades2DSizeFilters::SwapchainAspectRatio;
      texture_format_upgrades_2d_size_filters = (uint32_t)TextureFormatUpgrades2DSizeFilters::None;
      //enable_indirect_texture_format_upgrades = true; // Makes the game crash when we copy textures on the CPU in "OnMapTextureRegion" // TODOFT
      enable_chain_indirect_texture_format_upgrades = ChainTextureFormatUpgradesType::DirectDependencies;

#if 1
      // Upgrade post process textures only, the rest is not needed, it damages performance and potentially causes artifacts
      auto_texture_format_upgrade_shader_hashes = {
         // Tonemappers:
         {0x0A7D2AB7, {{0}, {}}},
         {0xAD6E5AAF, {{0}, {}}},
         {0x10FA30C8, {{0}, {}}},
         {0x2D2FB973, {{0}, {}}},
         {0x691AE5AF, {{0}, {}}},
         {0xEA7FA4E5, {{0}, {}}},
         {0x67A672D7, {{0}, {}}},
         {0xC8651827, {{0}, {}}},

         {0x35B62AAF, {{0}, {}}}, // Upscale (Temporal Filtering)
         {0x6C8FE673, {{0}, {}}}, // FXAA
         {0x5554278D, {{0}, {}}}, // SMAA
         {0x8ADB0AAD, {{0}, {}}}, // PostFX
         {0x65D9186F, {{0}, {}}}, // PostFX
         {0x5EA57AF3, {{0}, {}}}, // PostFX
         {0xF584A327, {{0}, {}}}, // PostFX
         {0x84DB2096, {{0}, {}}}, // Blur UI
      };
#endif
      texture_format_upgrades_lut_size = 32;
      texture_format_upgrades_lut_dimensions = LUTDimensions::_3D;

      shader_hashes_ColorGradingLUT.compute_shaders = {
         0xAC50585B,
         0xEED9A3FF,
         0x2B8472D5,
         0x8B8BEC2A,
         0x919F1537,
         0x56F305BB,
         0x2033D7C9,
         0x0A696247,
         0x28816DF0,
         0x7336D9BE,
         0x6F668AD1,
         0x60118D8B,
         0x8F54485D,
         0xB054D156,
         0x21774BE1,
         0x69D3F6E7,
      };
      shader_hashes_TemporalFiltering.compute_shaders = {
         0x45FD59AC,
         0x14AA8AC5,
      };
      shader_hashes_SMAA_Reprojection.compute_shaders = {
         0x1445F2D0,
      };
      shader_hashes_SMAA_EdgeDectction.pixel_shaders = {
         0xE82D1C86,
      };
      shader_hashes_CopyDpeth.pixel_shaders = {
         0x2D56E3E1,
      };
      shader_hashes_TemporalResolve.pixel_shaders = {
         0x4053E8B2,
      };
      shader_hashes_TemporalAA.pixel_shaders = {
         0x29C5D2F6,
         0xCB7D9EAE,
         0x3BF6F7A1,
         0xBE95E1B0,
      };
      shader_hashes_WaterGridVectorMap.pixel_shaders = {
         0xC8873B8F,
      };

      redirected_shader_hashes["ColorGradingLUT"] =
      {
         "AC50585B",
         "EED9A3FF",
         "2B8472D5",
         "8B8BEC2A",
         "919F1537",
         "56F305BB",
         "2033D7C9",
         "0A696247",
         "28816DF0",
         "7336D9BE",
         "6F668AD1",
         "60118D8B",
         "8F54485D",
         "B054D156",
         "21774BE1",
         "69D3F6E7",
      };
      // TODO: edge cases are still missing
      redirected_shader_hashes["Tonemap"] =
      {
         "0A7D2AB7",
         "AD6E5AAF",
         "10FA30C8",
         "2D2FB973",
         "691AE5AF",
         "EA7FA4E5",
         "67A672D7",
         "C8651827",
      };

#if DEVELOPMENT
      forced_shader_names.emplace(Shader::Hash_StrToNum("74F79E89"), "Clean to Black");
      forced_shader_names.emplace(Shader::Hash_StrToNum("4B06125F"), "Clean to Black");
      forced_shader_names.emplace(Shader::Hash_StrToNum("765C1510"), "UPlay Overlay");
      forced_shader_names.emplace(Shader::Hash_StrToNum("C941F7C4"), "Copy Depth");
      forced_shader_names.emplace(Shader::Hash_StrToNum("E82D1C86"), "SMAA Edges Detection");
      forced_shader_names.emplace(Shader::Hash_StrToNum("B9DD88BE"), "SMAA Weights Detection");
      forced_shader_names.emplace(Shader::Hash_StrToNum("1445F2D0"), "SMAA Weights Detection + Temporal Reprojection");
      forced_shader_names.emplace(Shader::Hash_StrToNum("5554278D"), "SMAA");
      forced_shader_names.emplace(Shader::Hash_StrToNum("29C5D2F6"), "Temporal Accumulation");
      forced_shader_names.emplace(Shader::Hash_StrToNum("4053E8B2"), "Temporal Accumulation Resolve");
#endif

      game = new WatchDogs2();
   }
   else if (ul_reason_for_call == DLL_PROCESS_DETACH)
   {
      g_deferred_fx_antialias_renderer_hook.reset();
      reshade::unregister_event<reshade::addon_event::execute_secondary_command_list>(WatchDogs2::OnExecuteSecondaryCommandList);
   }

   CoreMain(hModule, ul_reason_for_call, lpReserved);

   return TRUE;
}