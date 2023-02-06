#include <rabbit/ui/ui.hpp>

using namespace rb;

ui::ui(window& window, font& font)
    : m_window(window), m_font(font) {
    m_window.on<mouse_move_event>().connect<&ui::on_mouse_move>(this);
    m_window.on<mouse_button_event>().connect<&ui::on_mouse_button>(this);
}

ui::~ui() {
    m_window.on<mouse_button_event>().disconnect<&ui::on_mouse_button>(this);
    m_window.on<mouse_move_event>().disconnect<&ui::on_mouse_move>(this);
}

std::size_t ui::get_id() const {
    return !m_ids.empty() ? m_ids.top() : 0;
}

std::size_t ui::pop_id() {
    assert(!m_ids.empty());
    m_ids.pop();
    return get_id();
}

std::size_t ui::push_id(std::size_t id) {
    return m_ids.emplace(combine(get_id(), id));
}

std::size_t ui::push_id(std::string_view text) {
    std::hash<std::string_view> hash;
    return push_id(hash(text));
}

void ui::begin() {
}

void ui::end() {
    if (is_mouse_button_pressed(mouse_button::left)) {
        if (std::none_of(m_panels.begin(), m_panels.end(), [this](auto&& pair) {
            return pair.second.rect.contains(m_mouse_position);
        })) {
            m_active_id = 0;
        }
    }

    for (auto&& [button, pressed] : m_mouse_buttons) {
        m_last_mouse_buttons[button] = pressed;
    }
}

bool ui::begin_main_menu_bar() {
    // This is a special case of panel.
    m_panel_id = push_id("#main_menu_bar");

    panel& panel = m_panels[m_panel_id];
    panel.rect.x = 0.0f;
    panel.rect.y = 0.0f;
    panel.rect.z = float(m_window.size().x);
    panel.rect.w = 24.0f;
    panel.offset = vec2::zero();

    if (panel.rect.contains(m_mouse_position) && is_mouse_button_pressed(mouse_button::left)) {
        m_active_id = m_panel_id;
    }

    draw_rect(panel, panel.rect, { 60, 63, 65 });

    vec4 bottom_line = { panel.rect.x, panel.rect.y + 23.0f, panel.rect.z, 1.0f };

    draw_rect(panel, bottom_line, { 51, 51, 51 });
    return true;
}

void ui::end_main_menu_bar() {
    pop_id();

    m_panel_id = 0;
}

bool ui::begin_menu(std::string_view label) {
    std::size_t id = push_id(label);

    panel& panel = m_panels[m_panel_id];

    vec2 text_size = m_font.get_text_size(13, label);
    vec2 position = panel.rect.position() + panel.offset;
    vec2 size = { text_size.x + 16.0f, 23.0f };
    vec2 text_position = position + vec2(size.x / 2.0f - text_size.x / 2.0f, size.y / 2.0f - text_size.y / 2.0f - 2.0f);
    vec4 rect = { position.x, position.y, size.x, size.y };

    if (rect.contains(m_mouse_position) && is_mouse_button_pressed(mouse_button::left)) {
        m_active_id = id;
    }

    if (m_active_id == id) {
        draw_rect(panel, rect, color(92, 92, 92));
    } else {
        draw_rect(panel, rect, rect.contains(m_mouse_position) ? color(122, 128, 132) : color(60, 63, 65));
    }

    draw_text(panel, m_font, 13, label, text_position, color::white());

    panel.offset.x += size.x;
    return true;
}

void ui::end_menu() {
    pop_id();
}

bool ui::begin_window(std::string_view title) {
    m_panel_id = push_id(title);

    panel& panel = m_panels[m_panel_id];

    if (panel.rect.contains(m_mouse_position) && is_mouse_button_pressed(mouse_button::left)) {
        m_active_id = m_panel_id;
    }

    draw_rect(panel, panel.rect, color(60, 63, 65));

    vec4 top_line = { panel.rect.x, panel.rect.y, panel.rect.z, 1.0f };

    draw_rect(panel, top_line, color(81, 81, 81));

    vec4 bottom_line = { panel.rect.x, panel.rect.y + 23.0f, panel.rect.z, 1.0f };

    draw_rect(panel, bottom_line, { 51, 51, 51 });

    vec2 text_size = m_font.get_text_size(13, title);
    vec2 text_position = panel.rect.position() + vec2(8.0f, 12.0f - text_size.y / 2.0f - 2.0f);

    draw_text(panel, m_font, 13, title, text_position, color::white());
    return true;
}

void ui::end_window() {
    m_panels[m_panel_id].offset = vec2::zero();

    pop_id();

    m_panel_id = 0;
}

void ui::draw(renderer& renderer) {
    for (auto&& [id, panel] : m_panels) {
        for (draw_command& command : panel.commands) {
            span<const vertex2d> vertices(panel.vertices.data() + command.vertex_offset, command.vertex_count);
            span<const unsigned int> indices(panel.indices.data() + command.index_offset, command.index_count);
            renderer.draw(command.texture, vertices, indices);
        }

        panel.vertices.clear();
        panel.indices.clear();
        panel.commands.clear();
    }
}

std::size_t ui::combine(std::size_t seed, std::size_t id) const {
    std::hash<std::size_t> hash;
    return seed ^ (hash(id) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

void ui::draw_rect(panel& panel, const vec4& rect, color color) {
    std::size_t index_offset = panel.indices.size();
    std::size_t vertex_offset = panel.vertices.size();

    panel.indices.emplace_back(0);
    panel.indices.emplace_back(1);
    panel.indices.emplace_back(2);
    panel.indices.emplace_back(2);
    panel.indices.emplace_back(3);
    panel.indices.emplace_back(0);

    panel.vertices.push_back({ { rect.x, rect.y }, vec2::zero(), color });
    panel.vertices.push_back({ { rect.x, rect.y + rect.w}, vec2::zero(), color });
    panel.vertices.push_back({ { rect.x + rect.z, rect.y + rect.w }, vec2::zero(), color });
    panel.vertices.push_back({ { rect.x + rect.z, rect.y }, vec2::zero(), color });

    panel.commands.push_back(draw_command{ null, index_offset, std::size_t(6), vertex_offset, std::size_t(4) });
}

void ui::draw_rect(panel& panel, const texture& texture, const ivec4& src, const vec4& dst, color color) {
    std::size_t index_offset = panel.indices.size();
    std::size_t vertex_offset = panel.vertices.size();

    panel.indices.emplace_back(0);
    panel.indices.emplace_back(1);
    panel.indices.emplace_back(2);
    panel.indices.emplace_back(2);
    panel.indices.emplace_back(3);
    panel.indices.emplace_back(0);

    uvec2 size = texture.size();
    vec2 inv_size = { 1.0f / size.x, 1.0f / size.y };

    panel.vertices.push_back({ { dst.x, dst.y }, { src.x * inv_size.x, src.y * inv_size.y }, color });
    panel.vertices.push_back({ { dst.x, dst.y + dst.w}, { src.x * inv_size.x, (src.y + src.w) * inv_size.y } , color });
    panel.vertices.push_back({ { dst.x + dst.z, dst.y + dst.w }, { (src.x + src.z) * inv_size.x, (src.y + src.w) * inv_size.y } , color });
    panel.vertices.push_back({ { dst.x + dst.z, dst.y }, { (src.x + src.z) * inv_size.x, src.y * inv_size.y } , color });

    panel.commands.push_back(draw_command{ texture, index_offset, std::size_t(6), vertex_offset, std::size_t(4) });
}

void ui::draw_text(panel& panel, const font& font, unsigned char size, std::string_view text, const vec2& position, color color) {
    vec2 location = { position.x, position.y + size };
    for (std::size_t i = 0; i < text.size(); ++i) {
        const glyph& glyph = font.get_glyph(text[i], size);

        draw_rect(panel, font.atlas(), glyph.rect, { location.x + glyph.offset.x, location.y + glyph.offset.y, float(glyph.rect.z), float(glyph.rect.w) }, color);

        location.x += glyph.advance;

        if (i + 1 < text.size()) {
            location.x += font.get_kerning(text[i], text[i + 1], size);
        }
    }
}

void ui::on_mouse_move(const mouse_move_event& event) {
    m_mouse_position = event.position;
}

void ui::on_mouse_button(const mouse_button_event& event) {
    m_mouse_position = event.position;
    m_mouse_buttons[event.button] = event.pressed;
}

bool ui::is_mouse_button_pressed(mouse_button button) {
    return m_mouse_buttons[button] && !m_last_mouse_buttons[button];
}
