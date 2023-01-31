#include <rabbit/ui.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

using namespace rb;

ui::ui(window& p_window, graphics& p_graphics)
    : m_window(p_window), m_graphics(p_graphics) {
    p_window.on_mouse_move().connect<&ui::on_mouse_move>(this);
    p_window.on_mouse_button().connect<&ui::on_mouse_button>(this);

    assert(!ImGui::GetCurrentContext());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    io.BackendPlatformName = "RabBit";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    // io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = ImVec4(240.0f / 255.0f, 239.0f / 255.0f, 241.0f / 255.0f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(60.0f / 255.0f, 63.0f / 255.0f, 65.0f / 255.0f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(51.0f / 255.0f, 51.0f / 255.0f, 51.0f / 255.0f, 0.94f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 0.94f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 0.94f);
    // style.Colors[ImGuiCol_HeaderActive] = ImVec4(92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 0.94f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 0.94f);
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    style.WindowRounding = 0.0f;

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    const auto id = m_graphics.create_texture();
    m_graphics.set_texture_data(id, width, height, pixels);

    io.Fonts->SetTexID((ImTextureID)(std::uintptr_t)id);
}

ui::~ui() {
    ImGuiIO& io = ImGui::GetIO();
    m_graphics.destroy_texture((id_type)(std::uintptr_t)io.Fonts->TexID);
    ImGui::DestroyContext();
}

void ui::begin() {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280, 720);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    ImGui::NewFrame();
}

void ui::end() {
    ImGui::Render();
}

bool ui::begin_window(std::string_view p_title) {
    return true;
}

void ui::end_window() {
}

void ui::render() {
    const auto draw_data = ImGui::GetDrawData();

    const auto clip_off = draw_data->DisplayPos;
    const auto clip_scale = draw_data->FramebufferScale;

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmdList = draw_data->CmdLists[n];

        span<const vertex2d> vertices((const vertex2d*)cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size);
        span<const std::uint32_t> indices(cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size);

        for (int i = 0; i < cmdList->CmdBuffer.Size; i++) {
            const ImDrawCmd* command = &cmdList->CmdBuffer[i];

            if (command->UserCallback) {

            } else {
                ImVec2 clip_min((command->ClipRect.x - clip_off.x) * clip_scale.x, (command->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((command->ClipRect.z - clip_off.x) * clip_scale.x, (command->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                m_graphics.push_canvas_clip(clip_min.x, clip_min.y, clip_max.x - clip_min.x, clip_max.y - clip_min.y);

                span<const std::uint32_t> cmd_indices(indices.data() + command->IdxOffset, command->ElemCount);
                std::uint32_t max_vertex_index = 0;
                for (auto&& index : cmd_indices) {
                    max_vertex_index = max(max_vertex_index, index);
                }

                span<const vertex2d> cmd_vertices(vertices.data() + command->VtxOffset, std::size_t(max_vertex_index) + 1);

                m_graphics.push_canvas_primitives((id_type)(std::uintptr_t)command->GetTexID(), cmd_vertices, cmd_indices);
            }
        }
    }
}

void ui::on_mouse_move(const mouse_move_event& p_event) {
    auto& io = ImGui::GetIO();
    io.AddMousePosEvent(p_event.position.x, p_event.position.y);
}

void ui::on_mouse_button(const mouse_button_event& p_event) {
    auto& io = ImGui::GetIO();
    io.AddMouseButtonEvent(int(p_event.button), p_event.pressed);
}
