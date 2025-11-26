#define GAME_WATCH_DOGS 1
#define CHECK_GRAPHICS_API_COMPATIBILITY 1

#include "..\..\Core\core.hpp"

class WatchDogs final : public Game
{
public:
   void OnInit(bool async) override
   {
      // ### Update these (find the right values) ###
      // ### See the "GameCBuffers.hlsl" in the shader directory to expand settings ###
   }

   void PrintImGuiAbout() override
   {
      ImGui::Text("WATCH_DOGS Luma mod - about and credits section", "");
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