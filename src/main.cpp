
#include "sdl.hpp"
#include "emberjs/websocket.hpp"
#include "emberjs/config.hpp"

#include "entities.hpp"
#include "components.hpp"
#include "font.hpp"
#include "gui.hpp"
#include "lua_gui.hpp"
#include "sushi_renderer.hpp"
#include "shaders.hpp"
#include "utility.hpp"
#include "resource_cache.hpp"
#include "json_lua.hpp"

#include <sushi/sushi.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/noise.hpp>

#include <sol.hpp>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstddef>
#include <cmath>
#include <functional>
#include <memory>
#include <random>

using namespace std::literals;

inline auto imod(double x, double y) -> int {
    return x - y * std::floor(x / y);
}

class engine {
public:
    engine() {
        config = emberjs::get_config();

        lua.open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table,
            sol::lib::debug,
            sol::lib::package);

        lua["package"]["path"] = "data/scripts/?.lua";
        lua["package"]["cpath"] = "";

        lua.new_usertype<glm::vec2>(
            "vec2", sol::constructors<glm::vec2(), glm::vec2(const glm::vec2 &)>{},
            "x", &glm::vec2::x,
            "y", &glm::vec2::y);

        lua_gui::register_types(lua.globals());

        register_engine_module();

        {
            auto json_table = lua.create_table();
            json_table["json_to_lua"] = json_lua::json_to_lua;
            json_table["lua_to_json"] = json_lua::lua_to_json;
            json_table["load_file"] = json_lua::load_file;
            json_table["dump_json"] = [](const nlohmann::json &json) { return json.dump(); };
            lua["package"]["loaded"]["json"] = json_table;
        }

        {
            auto component_table = lua.create_named_table("component");
            component::register_all_components(component_table);
        }

        {
            auto sushi_table = lua.create_named_table("sushi");
            sushi_table.new_usertype<sushi::texture_2d>("texture_2d");
            sushi_table.new_usertype<sushi::static_mesh>("static_mesh");
        }

        display_width = config["display"]["width"];
        display_height = config["display"]["height"];
        aspect_ratio = float(display_width) / float(display_height);

        g_window = SDL_CreateWindow(
            "LD44",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            display_width,
            display_height,
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        glcontext = SDL_GL_CreateContext(g_window);

        SDL_StartTextInput();

        glEnable(GL_DEPTH_TEST);

        program_basic = basic_shader_program("data/shaders/basic.vert", "data/shaders/basic.frag");
        program_msdf = msdf_shader_program("data/shaders/msdf.vert", "data/shaders/msdf.frag");

        program_basic.bind();
        program_basic.set_s_texture(0);
        glBindAttribLocation(program_basic.get_program().get(), sushi::attrib_location::POSITION, "position");
        glBindAttribLocation(program_basic.get_program().get(), sushi::attrib_location::TEXCOORD, "texcoord");
        glBindAttribLocation(program_basic.get_program().get(), sushi::attrib_location::NORMAL, "normal");

        program_msdf.bind();
        glBindAttribLocation(program_msdf.get_program().get(), sushi::attrib_location::POSITION, "position");
        glBindAttribLocation(program_msdf.get_program().get(), sushi::attrib_location::TEXCOORD, "texcoord");
        glBindAttribLocation(program_msdf.get_program().get(), sushi::attrib_location::NORMAL, "normal");

        mesh_cache = [](const std::string& name) {
            return sushi::load_static_mesh_file("data/models/" + name + ".obj");
        };

        texture_cache = [](const std::string& name) {
            if (name == ":white") {
                unsigned char white[4] = { 0xff, 0xff, 0xff, 0xff };
                auto tex = sushi::create_uninitialized_texture_2d(1, 1, sushi::TexType::COLORA);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, white);
                return tex;
            } else {
                return sushi::load_texture_2d("data/textures/" + name + ".png", true, false, true, false);
            }
        };

        font_cache = [](const std::string& fontname) {
            return msdf_font("data/fonts/"+fontname+".ttf");
        };

        renderer = sushi_renderer(
            {display_width, display_height},
            program_basic,
            program_msdf,
            font_cache,
            texture_cache);

        root_widget = std::make_shared<gui::widget>(renderer);
        root_widget->set_attribute("width", std::to_string(display_width));
        root_widget->set_attribute("height", std::to_string(display_height));

        lua["root_widget"] = root_widget;
        lua["focus_widget"] = [this](gui::widget* widget) {
            focused_widget = widget->weak_from_this();
        };

        gui_state = lua.create_named_table("gui_state");
        gui_state["fps"] = 0;
        gui_state["version"] = "ALPHA 0.0.0";

        auto init_gui_result = lua.do_file("data/scripts/init_gui.lua");

        if (!init_gui_result.valid()) {
            sol::error err = init_gui_result;
            throw std::runtime_error("init_gui(): "s + err.what());
        }

        update_gui_state = lua["update_gui_state"];

        luakeys = lua.create_named_table("keys");
        luakeys["left"] = false;
        luakeys["right"] = false;
        luakeys["up"] = false;
        luakeys["down"] = false;
        luakeys["shoot"] = false;
        luakeys["slow"] = false;

        prev_time = clock::now();

        framerate_buffer.reserve(10);

        enable_lighting = true;
        
        auto init_result = lua.safe_script_file("data/scripts/init.lua");
        if (!init_result.valid()) {
            sol::error err = init_result;
            throw std::runtime_error("Init error: "s + err.what());
        }
    }

    ~engine() {
        SDL_GL_DeleteContext(glcontext);
        SDL_DestroyWindow(g_window);
    }

    void register_engine_module() {
        auto global_table = sol::table(lua.globals());

        scripting::register_type<ember_database>(global_table);

        auto engine_table = lua.create_table();

        engine_table["entities"] = std::ref(entities);
        engine_table["get_texture"] = [this](const std::string& name) {
            return texture_cache.get(name);
        };
        engine_table["get_mesh"] = [this](const std::string& name) {
            return mesh_cache.get(name);
        };
        lua["package"]["loaded"]["engine"] = engine_table;
    }

    auto handle_game_input(const SDL_Event& event) -> bool {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_L: enable_lighting = !enable_lighting; break;
                    default: return false;
                }
                return true;
            case SDL_QUIT:
                std::cout << "Goodbye!" << std::endl;
                return true;
        }

        return false;
    }

    auto handle_gui_input(SDL_Event& e) -> bool {
        switch (e.type) {
            case SDL_TEXTINPUT: {
                if (auto widget = focused_widget.lock()) {
                    if (widget->on_textinput) {
                        widget->on_textinput(widget, e.text.text);
                        return true;
                    }
                }
                break;
            }
            case SDL_KEYDOWN: {
                if (auto widget = focused_widget.lock()) {
                    if (widget->on_keydown) {
                        return widget->on_keydown(widget, SDL_GetKeyName(e.key.keysym.sym));
                    }
                }
            }
            case SDL_MOUSEBUTTONDOWN: {
                switch (e.button.button) {
                    case SDL_BUTTON_LEFT: {
                        auto abs_click_pos = glm::vec2{e.button.x, display_height - e.button.y + 1};
                        auto widget_stack = get_descendent_stack(*root_widget, abs_click_pos);
                        while (!widget_stack.empty()) {
                            auto cur_widget = widget_stack.back();
                            auto widget_pos = cur_widget->get_layout().position;
                            auto rel_click_pos = abs_click_pos - widget_pos;
                            if (cur_widget->on_click) {
                                cur_widget->on_click(cur_widget->shared_from_this(), rel_click_pos);
                                return true;
                            } else if (cur_widget->pointer_opaque()) {
                                return true;
                            } else {
                                widget_stack.pop_back();
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
        return false;
    }

    void tick() {
        auto now = clock::now();
        auto delta_time = now - prev_time;
        prev_time = now;

        const auto delta = std::chrono::duration<double>(delta_time).count();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (handle_gui_input(event)) break;
            if (handle_game_input(event)) break;
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        luakeys["left"] = bool(keys[SDL_SCANCODE_LEFT]);
        luakeys["right"] = bool(keys[SDL_SCANCODE_RIGHT]);
        luakeys["up"] = bool(keys[SDL_SCANCODE_UP]);
        luakeys["down"] = bool(keys[SDL_SCANCODE_DOWN]);
        luakeys["shoot"] = bool(keys[SDL_SCANCODE_SPACE]);
        luakeys["slow"] = bool(keys[SDL_SCANCODE_LSHIFT]);

        framerate_buffer.push_back(delta_time);

        if (framerate_buffer.size() >= 10) {
            auto avg_frame_dur = std::accumulate(begin(framerate_buffer), end(framerate_buffer), 0ns) / framerate_buffer.size();
            auto framerate = 1.0 / std::chrono::duration<double>(avg_frame_dur).count();

            gui_state["fps"] = std::to_string(std::lround(framerate)) + "fps";

            framerate_buffer.clear();
        }

        auto run_system = [this](const std::string &name, const auto&... args) {
            auto module_name = "systems." + name;
            auto file_name = "data/scripts/systems/" + name + ".lua";
            auto scripting = sol::table(lua.require_file(module_name, file_name));
            sol::protected_function visit = scripting["visit"];
            auto visit_result = visit(args...);

            if (!visit_result.valid()) {
                sol::error err = visit_result;
                throw std::runtime_error(std::string("System failure: " + name + ": ") + err.what());
            }
        };

        run_system("controller", delta);
        run_system("physics", delta);
        run_system("scripting", delta);

        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto proj = glm::ortho(-10.f, 10.f, 0.f, 15.f, 5.f, -5.f);
        auto view = glm::mat4(1.f);

        program_basic.bind();
        program_basic.set_cam_forward({0.0, 0.0, -1.0});
        program_basic.set_enable_lighting(enable_lighting);
        program_basic.set_tint({1, 1, 1, 1});
        program_basic.set_hue(0);
        program_basic.set_saturation(1);

        if (enable_lighting) {
            using glm::vec3;
            using glm::normalize;

            const auto sky_dir = normalize(vec3{1, 1, 1});
            const auto sky_color = vec3{1, 1, 1};
            const auto ambient_color = vec3{0.5, 0.5, 0.5};

            program_basic.set_sky_dir(sky_dir);
            program_basic.set_sky_color(sky_color);
            program_basic.set_ambient_color(ambient_color);
        }

        auto frustum = sushi::frustum(proj*view);

        // Render entities
        entities.visit([&](ember_database::ent_id eid, const component::position& position, const component::model& model) {
            if (model.mesh && model.texture) {
                const auto pos = glm::vec3(position.pos, 0);
                if (frustum.contains(pos, 2)) {
                    auto modelmat = glm::scale(glm::translate(glm::mat4(1.f), pos), glm::vec3{model.scale});
                    program_basic.set_normal_mat(glm::inverseTranspose(view * modelmat));
                    program_basic.set_MVP(proj * view * modelmat);
                    sushi::set_texture(0, *model.texture);
                    sushi::draw_mesh(*model.mesh);
                }
            }
        });

        update_gui_state();
        gui::calculate_all_layouts(*root_widget);

        renderer.begin();
        gui::draw_all(*root_widget);
        renderer.end();

        lua.collect_garbage();

        SDL_GL_SwapWindow(g_window);
    }

private:
    using clock = std::chrono::steady_clock;

    sol::state lua;
    nlohmann::json config;
    int display_width;
    int display_height;
    float aspect_ratio;
    SDL_Window* g_window;
    SDL_GLContext glcontext;
    basic_shader_program program_basic;
    msdf_shader_program program_msdf;
    resource_cache<sushi::static_mesh, std::string> mesh_cache;
    resource_cache<sushi::texture_2d, std::string> texture_cache;
    resource_cache<msdf_font, std::string> font_cache;
    ember_database entities;
    sushi_renderer renderer;
    std::shared_ptr<gui::widget> root_widget;
    std::weak_ptr<gui::widget> focused_widget;
    clock::time_point prev_time;
    std::vector<std::chrono::nanoseconds> framerate_buffer;
    sol::table gui_state;
    sol::function update_gui_state;
    sol::table luakeys;
    bool enable_lighting;
};

std::function<void()> loop;
void main_loop() try {
    loop();
} catch (const std::exception& e) {
    std::cerr << "Fatal exception: " << e.what() << std::endl;
    std::terminate();
}

int main() try {
    std::cout << "Init..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    auto e = std::make_unique<engine>();

    std::cout << "Success." << std::endl;

    loop = [&e]{ e->tick(); };

    emscripten_set_main_loop(main_loop, 0, 1);

    SDL_Quit();

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::cerr << "Fatal exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
