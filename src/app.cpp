#include "app.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
  // Initialize SDL3
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
  {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create window
  SDL_Window *window = SDL_CreateWindow(
      "imgui_template_app",
      1280, 720,
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

  if (!window)
  {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create GPU device
  SDL_GPUDevice *gpuDevice = SDL_CreateGPUDevice(
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

  // Allocate app state
  AppState *state = new AppState();
  state->window = window;
  state->gpuDevice = gpuDevice;
  state->isFullscreen = false;
  *appstate = state;

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
  ImGui_ImplSDL3_ProcessEvent(event);

  if (event->type == SDL_EVENT_QUIT)
    return SDL_APP_SUCCESS;

  AppState *state = (AppState *)appstate;
  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
      event->window.windowID == SDL_GetWindowID(state->window))
    return SDL_APP_SUCCESS;

  // Handle F11 for fullscreen toggle
  if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_F11)
  {
    state->isFullscreen = !state->isFullscreen;
    SDL_SetWindowFullscreen(state->window, state->isFullscreen);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
  AppState *state = (AppState *)appstate;
  ImGuiIO &io = ImGui::GetIO();

  // Start the Dear ImGui frame
  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // Create a fullscreen dockspace
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                  ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace", nullptr, window_flags);
  ImGui::PopStyleVar(3);

  ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

  // Build the dockspace layout on first run
  static bool first_time = true;
  if (first_time)
  {
    first_time = false;
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

    // Dock the window to fill the space
    ImGui::DockBuilderDockWindow("Example", dockspace_id);

    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking);
  ImGui::End();

  // Example ImGui window
  {
    ImGui::Begin("Example", nullptr, ImGuiWindowFlags_NoMove);
    ImGui::Text("This is a sample ImGui window.");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
  }

  // Rendering
  ImGui::Render();

  SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(state->gpuDevice);
  if (cmd)
  {
    ImGui_ImplSDLGPU3_PrepareDrawData(ImGui::GetDrawData(), cmd);

    SDL_GPUTexture *swapchain_texture;
    if (SDL_WaitAndAcquireGPUSwapchainTexture(cmd, state->window, &swapchain_texture, nullptr, nullptr))
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
  AppState *state = (AppState *)appstate;

  // Cleanup
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_ReleaseWindowFromGPUDevice(state->gpuDevice, state->window);
  SDL_DestroyGPUDevice(state->gpuDevice);
  SDL_DestroyWindow(state->window);

  delete state;
}