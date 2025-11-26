#define GAME_WATCH_DOGS 1
#define CHECK_GRAPHICS_API_COMPATIBILITY 1

#include "..\..\Core\core.hpp"

uint32_t GetHaltonSequencePhases(float renderResY, float outputResY, float basePhases)
{
   // NV DLSS suggested formula
   return std::lrintf(basePhases * pow(outputResY / renderResY, 2.f));
}

float Halton(int32_t Index, int32_t Base)
{
   float Result = 0.0f;
   float InvBase = 1.0f / Base;
   float Fraction = InvBase;
   while (Index > 0)
   {
      Result += (Index % Base) * Fraction;
      Index /= Base;
      Fraction *= InvBase;
   }
   return Result;
}

namespace
{
   ShaderHashesList shader_hashes_TAA; // DeferredFX AA
}

struct GameDeviceDataWatchDogs final : public GameDeviceData
{
   com_ptr<ID3D11Resource> motion_vectors;
   com_ptr<ID3D11Resource> depth;

   com_ptr<ID3D11Resource> last_motion_vectors;
   com_ptr<ID3D11RenderTargetView> last_motion_vectors_rtv;

   DirectX::XMMATRIX prev_view_projection_mat;
   DirectX::XMMATRIX view_projection_mat;

   float2 taa_jitters = {};
   bool found_per_view_globals = false;
};

class WatchDogs final : public Game
{
public:
   static const GameDeviceDataWatchDogs& GetGameDeviceData(const DeviceData& device_data)
   {
      return *static_cast<const GameDeviceDataWatchDogs*>(device_data.game);
   }
   static GameDeviceDataWatchDogs& GetGameDeviceData(DeviceData& device_data)
   {
      return *static_cast<GameDeviceDataWatchDogs*>(device_data.game);
   }

   void OnInit(bool async) override
   {
      // ### Update these (find the right values) ###
      // ### See the "GameCBuffers.hlsl" in the shader directory to expand settings ###
   }

   void OnLoad(std::filesystem::path& file_path, bool failed) override
   {
      if (!failed)
      {
#if DEVELOPMENT
         // reshade::register_event<reshade::addon_event::clear_render_target_view>(WatchDogs::OnClearRenderTargetView);
#endif
         reshade::register_event<reshade::addon_event::map_buffer_region>(WatchDogs::OnMapBufferRegion);
         reshade::register_event<reshade::addon_event::unmap_buffer_region>(WatchDogs::OnUnmapBufferRegion);
         reshade::register_event<reshade::addon_event::update_buffer_region>(WatchDogs::OnUpdateBufferRegion);
      }
   }

   void OnCreateDevice(ID3D11Device* native_device, DeviceData& device_data) override
   {
      device_data.game = new GameDeviceDataWatchDogs;
   }

   void OnInitSwapchain(reshade::api::swapchain* swapchain) override
   {
      auto& device_data = *swapchain->get_device()->get_private_data<DeviceData>();

      cb_luma_global_settings.GameSettings.InvOutputRes.x = 1.f / device_data.output_resolution.x;
      cb_luma_global_settings.GameSettings.InvOutputRes.y = 1.f / device_data.output_resolution.y;
      device_data.cb_luma_global_settings_dirty = true;
   }

   void PrintImGuiAbout() override
   {
      ImGui::Text("WATCH_DOGS Luma mod - about and credits section", "");
   }

	static constexpr uint32_t CBPerViewGlobal_buffer_size = 960;

	static void OnMapBufferRegion(reshade::api::device* device, reshade::api::resource resource, uint64_t offset, uint64_t size, reshade::api::map_access access, void** data)
    {
      ID3D11Device* native_device = (ID3D11Device*)(device->get_native());
      ID3D11Buffer* buffer = reinterpret_cast<ID3D11Buffer*>(resource.handle);
      DeviceData& device_data = *device->get_private_data<DeviceData>();
      // auto& game_device_data = GetGameDeviceData(device_data);

      if (access == reshade::api::map_access::write_only || access == reshade::api::map_access::write_discard || access == reshade::api::map_access::read_write)
      {
         D3D11_BUFFER_DESC buffer_desc;
         buffer->GetDesc(&buffer_desc);

         // There seems to only ever be one buffer type of this size, but it's not guaranteed (we might have found more, but it doesn't matter, they are discarded later)...
         // They seemingly all happen on the same thread.
         // Some how these are not marked as "D3D11_BIND_CONSTANT_BUFFER", probably because it copies them over to some other buffer later?
         if (buffer_desc.ByteWidth == CBPerViewGlobal_buffer_size)
         {
            device_data.cb_per_view_global_buffer = buffer;
#if DEVELOPMENT
            ASSERT_ONCE(buffer_desc.Usage == D3D11_USAGE_DYNAMIC && buffer_desc.BindFlags == D3D11_BIND_CONSTANT_BUFFER && buffer_desc.CPUAccessFlags == D3D11_CPU_ACCESS_WRITE && buffer_desc.MiscFlags == 0 && buffer_desc.StructureByteStride == 0);
#endif // DEVELOPMENT
            ASSERT_ONCE(!device_data.cb_per_view_global_buffer_map_data);
            device_data.cb_per_view_global_buffer_map_data = *data;
         }
#if 0
         res_map[buffer] = *data;
#endif
      }
    }

	static void OnUnmapBufferRegion(reshade::api::device* device, reshade::api::resource resource)
   {
      DeviceData& device_data = *device->get_private_data<DeviceData>();
      auto& game_device_data = GetGameDeviceData(device_data);
      ID3D11Device* native_device = (ID3D11Device*)(device->get_native());

      ID3D11Buffer* buffer = reinterpret_cast<ID3D11Buffer*>(resource.handle);

      bool is_global_cbuffer = device_data.cb_per_view_global_buffer != nullptr && device_data.cb_per_view_global_buffer == buffer;

      ASSERT_ONCE(!device_data.cb_per_view_global_buffer_map_data || is_global_cbuffer);

		if (is_global_cbuffer && device_data.cb_per_view_global_buffer_map_data != nullptr)
      {
         float4(&float_data)[CBPerViewGlobal_buffer_size / sizeof(float4)] = *((float4(*)[CBPerViewGlobal_buffer_size / sizeof(float4)]) device_data.cb_per_view_global_buffer_map_data);

			bool is_valid_cbuffer = true
                                 //&& float_data[22].x == 0.f && float_data[22].y == 0.f
                                 && float_data[35].x == device_data.render_resolution.x && float_data[35].y == device_data.render_resolution.y && float_data[35].z == 1.f / device_data.render_resolution.x && float_data[35].w == 1.f / device_data.render_resolution.y;
   }

};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   if (ul_reason_for_call == DLL_PROCESS_ATTACH)
   {
      Globals::SetGlobals(PROJECT_NAME, "WATCH_DOGS Luma mod");
      Globals::DEVELOPMENT_STATE = Globals::ModDevelopmentState::WorkInProgress;
      Globals::VERSION = 1;

      luma_settings_cbuffer_index = 13;
      luma_data_cbuffer_index = 12;

      swapchain_format_upgrade_type = TextureFormatUpgradesType::AllowedEnabled;
      swapchain_upgrade_type = SwapchainUpgradeType::scRGB;
      texture_format_upgrades_type = TextureFormatUpgradesType::None;
      enable_indirect_texture_format_upgrades = false; // This is generally safer so enable it in the generic mod
      enable_automatic_indirect_texture_format_upgrades = false;
      texture_upgrade_formats = {
      };
      // ### Check these if textures are not upgraded ###
      // texture_format_upgrades_2d_size_filters = 0 | (uint32_t)TextureFormatUpgrades2DSizeFilters::SwapchainResolution | (uint32_t)TextureFormatUpgrades2DSizeFilters::SwapchainAspectRatio;

      game = new WatchDogs();
   }

   CoreMain(hModule, ul_reason_for_call, lpReserved);

   return TRUE;
}