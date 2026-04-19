#define GAME_METRO_REDUX 1

#define DISABLE_AUTO_DEBUGGER 1

#define DISABLE_FOCUS_LOSS_SUPPRESSION 1

#define CHECK_GRAPHICS_API_COMPATIBILITY 1

#define AVOID_INPUT_LOSS 1

// Needed because the game exclusively set CBuffers once on boot, and uses all of them!
#define ENABLE_AUTO_CBUFFER_RESTORATION 1

#include "..\..\Core\core.hpp"

class MetroRedux final : public Game
{
public:
   void OnInit(bool async) override
   {
      // ### Update these (find the right values) ###
      // ### See the "GameCBuffers.hlsl" in the shader directory to expand settings ###
      luma_settings_cbuffer_index = 13;
      luma_data_cbuffer_index = 12;
   }

   void PrintImGuiAbout() override
   {
      ImGui::Text("Metro 2033 Redux and Metro Last Light Redux Luma mod - about and credits section", ""); // ### Rename this ###
   }
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   if (ul_reason_for_call == DLL_PROCESS_ATTACH)
   {
      Globals::SetGlobals(PROJECT_NAME, "Luma for Metro 2033 Redux and Metro Last Light Redux");
      Globals::VERSION = 1;

      swapchain_format_upgrade_type = TextureFormatUpgradesType::AllowedEnabled;
      swapchain_upgrade_type = SwapchainUpgradeType::scRGB;
      texture_format_upgrades_type = TextureFormatUpgradesType::AllowedEnabled;
      texture_upgrade_formats = {
         // reshade::api::format::r8g8b8a8_unorm,
         // reshade::api::format::r8g8b8a8_unorm_srgb,
         // reshade::api::format::r8g8b8a8_typeless,
         // reshade::api::format::r8g8b8x8_unorm,
         reshade::api::format::r8g8b8x8_unorm_srgb,
         /*reshade::api::format::b8g8r8a8_unorm,
         reshade::api::format::b8g8r8a8_unorm_srgb,
         reshade::api::format::b8g8r8a8_typeless,
         reshade::api::format::b8g8r8x8_unorm,
         reshade::api::format::b8g8r8x8_unorm_srgb,
         reshade::api::format::b8g8r8x8_typeless,

         reshade::api::format::r11g11b10_float,*/
      };
      // ### Check these if textures are not upgraded ###
      texture_format_upgrades_2d_size_filters = 0 | (uint32_t)TextureFormatUpgrades2DSizeFilters::SwapchainResolution | (uint32_t)TextureFormatUpgrades2DSizeFilters::SwapchainAspectRatio;

      force_borderless = true; // These games have very bad window managment, we might need to force windowed mode and then reroute it as borderless
      force_ignore_dpi = true; // Not sure whether it makes a difference, but shouldn't hurt

      game = new MetroRedux();
   }

   CoreMain(hModule, ul_reason_for_call, lpReserved);

   return TRUE;
}