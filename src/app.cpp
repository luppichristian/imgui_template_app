#include "app.h"
#include "user.h"

SDL_Window *window;
SDL_GPUDevice *gpuDevice;
bool isFullscreen;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
  // Initialize SDL3
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
  {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create window
  window = SDL_CreateWindow(TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS);
  if (!window)
  {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create GPU device
  gpuDevice = SDL_CreateGPUDevice(
      SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
      true,
      nullptr);

  if (!gpuDevice)
  {
    SDL_Log("Failed to create GPU device: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    return SDL_APP_FAILURE;
  }

  // Claim window for GPU
  if (!SDL_ClaimWindowForGPUDevice(gpuDevice, window))
  {
    SDL_Log("Failed to claim window for GPU: %s", SDL_GetError());
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_DestroyWindow(window);
    return SDL_APP_FAILURE;
  }

  // Set present mode to VSync
  if (!SDL_SetGPUSwapchainParameters(gpuDevice, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC))
  {
    SDL_Log("Failed to set swapchain parameters: %s", SDL_GetError());
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_DestroyWindow(window);
    return SDL_APP_FAILURE;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  if (!ImGui_ImplSDL3_InitForSDLGPU(window))
  {
    SDL_Log("Failed to initialize ImGui SDL3 backend");
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_DestroyWindow(window);
    return SDL_APP_FAILURE;
  }

  ImGui_ImplSDLGPU3_InitInfo init_info = {};
  init_info.Device = gpuDevice;
  init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpuDevice, window);
  init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
  if (!ImGui_ImplSDLGPU3_Init(&init_info))
  {
    SDL_Log("Failed to initialize ImGui SDL GPU backend");
    ImGui_ImplSDL3_Shutdown();
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_DestroyWindow(window);
    return SDL_APP_FAILURE;
  }

  isFullscreen = false;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
  ImGui_ImplSDL3_ProcessEvent(event);

  if (event->type == SDL_EVENT_QUIT)
    return SDL_APP_SUCCESS;

  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
      event->window.windowID == SDL_GetWindowID(window))
    return SDL_APP_SUCCESS;

  // Handle F11 for fullscreen toggle
  if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_F11)
  {
    isFullscreen = !isFullscreen;
    SDL_SetWindowFullscreen(window, isFullscreen);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
  ImGuiIO &io = ImGui::GetIO();

  // Start the Dear ImGui frame
  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // Build UI
  buildUI();

  // Rendering
  ImGui::Render();

  SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpuDevice);
  if (cmd)
  {
    ImGui_ImplSDLGPU3_PrepareDrawData(ImGui::GetDrawData(), cmd);

    SDL_GPUTexture *swapchain_texture;
    if (SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain_texture, nullptr, nullptr))
    {
      if (swapchain_texture)
      {
        SDL_GPUColorTargetInfo colorTarget = {};
        colorTarget.texture = swapchain_texture;
        colorTarget.clear_color.r = 0.0f;
        colorTarget.clear_color.g = 0.0f;
        colorTarget.clear_color.b = 0.0f;
        colorTarget.clear_color.a = 1.0f;
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, nullptr);
        ImGui_ImplSDLGPU3_RenderDrawData(ImGui::GetDrawData(), cmd, render_pass);
        SDL_EndGPURenderPass(render_pass);
      }
    }
    SDL_SubmitGPUCommandBuffer(cmd);
  }

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
  // Cleanup
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
  SDL_DestroyGPUDevice(gpuDevice);
  SDL_DestroyWindow(window);
}