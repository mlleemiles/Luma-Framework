
#pragma once

struct GBFROutputReplayState
{
   ComPtr<ID3D11VertexShader> vs;
   ComPtr<ID3D11PixelShader> ps;
   ComPtr<ID3D11RenderTargetView> rtv;
   D3D11_VIEWPORT viewport = {};
   ComPtr<ID3D11ShaderResourceView> original_t0_srv; // game's original t0 — used when no UI phase
   GBFRBufferInfo original_b1_cb;
   ComPtr<ID3D11RasterizerState> rs_state;
   ComPtr<ID3D11BlendState> blend_state;
   FLOAT blend_factor[4] = {};
   UINT sample_mask = 0xFFFFFFFF;
   // IA state — VS takes two vertex buffers: slot 0 = position (R32G32B32), slot 1 = UV (R32G32).
   ComPtr<ID3D11InputLayout> input_layout;
   ComPtr<ID3D11Buffer> vertex_buffers[2];
   UINT vb_strides[2] = {};
   UINT vb_offsets[2] = {};
   // Sampler captured from the game draw (MIN_MAG_LINEAR_MIP_POINT + BORDER).
   ComPtr<ID3D11SamplerState> ps_sampler;
   bool valid = false;

   void Reset()
   {
      vs = nullptr;
      ps = nullptr;
      rtv = nullptr;
      viewport = {};
      original_t0_srv = nullptr;
      original_b1_cb.Reset();
      rs_state = nullptr;
      blend_state = nullptr;
      blend_factor[0] = blend_factor[1] = blend_factor[2] = blend_factor[3] = 0.f;
      sample_mask = 0xFFFFFFFF;
      input_layout = nullptr;
      vertex_buffers[0] = vertex_buffers[1] = nullptr;
      vb_strides[0] = vb_strides[1] = 0;
      vb_offsets[0] = vb_offsets[1] = 0;
      ps_sampler = nullptr;
      valid = false;
   }
};

// State for upscaling the scene background before UI draws so the UI
// renders at output resolution on top of a bilinearly-upscaled scene.
// scaled_texture is pre-allocated in OnInitSwapchain and always valid.
struct UIScaleState
{
   // Persistent resources (recreated only on resolution change).
   ComPtr<ID3D11Texture2D> scaled_texture;
   ComPtr<ID3D11ShaderResourceView> scaled_texture_srv;
   ComPtr<ID3D11RenderTargetView> scaled_texture_rtv;

   // Per-frame state — reset in OnPresent.
   ComPtr<ID3D11RenderTargetView> original_ui_rtv = nullptr; // raw pointer for RTV/SRV comparison in RedirectUIDrawToScaledTarget

   // Capture/replay state for the Output shader pass.
   std::atomic<bool> ui_phase_seen_this_frame = false;
   std::atomic<bool> ui_scaled_needed = false;
   ComPtr<ID3D11CommandList> output_partial_command_list;
   GBFROutputReplayState output_replay_state;
   std::atomic<bool> output_pending = false;
   std::atomic<ID3D11DeviceContext*> output_device_context = nullptr;
   std::atomic<ID3D11CommandList*> output_remainder_command_list = nullptr;
   std::atomic<ID3D11DeviceContext*> ui_detected_context = nullptr;
   // Command list created from the cached UI deferred context during FinishCommandList.
   std::atomic<ID3D11CommandList*> ui_finish_command_list = nullptr;

   void ResetPerFrame()
   {
      original_ui_rtv = nullptr;
      ui_phase_seen_this_frame = false;
      output_partial_command_list.reset();
      output_replay_state.Reset();
      output_pending.store(false, std::memory_order_relaxed);
      output_device_context.store(nullptr, std::memory_order_relaxed);
      output_remainder_command_list.store(nullptr, std::memory_order_relaxed);
      ui_detected_context.store(nullptr, std::memory_order_relaxed);
      ui_finish_command_list.store(nullptr, std::memory_order_relaxed);
   }
};