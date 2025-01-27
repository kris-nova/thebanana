#include "pch.h"
#include "game.h"
#include "scene.h"
#include "input_manager.h"
#include "debug_tools.h"
#include "graphics/framebuffer.h"
#include "graphics/quad.h"
#include "graphics/opengl/opengl_framebuffer.h"
#include "graphics/opengl/opengl_quad.h"
#include "graphics/context.h"
#include "shader_registry.h"
#include "script_registry.h"
#include "util.h"
#include "internal_util.h"
#include "material.h"
#include "cursor.h"
#include "platform_misc.h"
#ifdef BANANA_WINDOWS
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
namespace thebanana {
#ifdef BANANA_MACOSX
	namespace platform_specific {
		extern void* load_opengl_function(const char* name);
	}
#endif
	void opengl_debug_function(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* user_param) {
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
		game* g = (game*)(void*)user_param;
		g->debug_print(message);
#ifdef BANANA_WINDOWS
		__debugbreak();
#endif
	}
	game* g_game = NULL;
	game::game(const std::string& title, script_module::module_t module) : m_executable(module) {
		srand(CURRENT_TIME(unsigned int));
		this->m_has_custom_size = false;
		this->m_frame = 0;
		constexpr int width = 1600;
		constexpr int height = 900;
		this->m_window.m = platform_specific::create_window(title, width, height, this);
		if (!this->m_window.m) {
			this->debug_print("last error: " + std::to_string(platform_specific::get_last_os_error()));
#ifdef BANANA_WINDOWS
			__debugbreak();
#endif
			this->destroy();
		}
		this->update_aspect_ratio();
		void* context_data;
#ifdef RENDERER_OPENGL
		graphics::context::opengl_data ogl_data;
		ogl_data.major_version = 4;
#ifdef BANANA_MACOSX
		ogl_data.minor_version = 1; // the most recent opengl version on mac is 4.1
#endif
#ifdef BANANA_WINDOWS
		ogl_data.minor_version = 6;
#endif
#ifdef BANANA_LINUX
		ogl_data.minor_version = 3;
#endif
		context_data = &ogl_data;
		graphics::set_default_graphics_api(graphics::graphics_api::opengl);
#else
		context_data = NULL;
		graphics::set_default_graphics_api(graphics::graphics_api::none);
#endif
		this->m_context = graphics::context::create(this->m_window, context_data);
#ifdef BANANA_MACOSX
		gladLoadGLLoader(platform_specific::load_opengl_function);
#else
		unsigned int glew_error = glewInit();
		if (glew_error != GLEW_OK) {
			this->debug_print("got glew error: " + std::string((const char*)glewGetErrorString(glew_error)));
#ifdef BANANA_WINDOWS
			__debugbreak();
#endif
			this->destroy();
		}
#endif
		this->m_scene = new scene(this);
		this->m_input_manager = new input_manager(this);
		this->m_shader_registry = new shader_registry(this);
		this->m_script_registry = new script_registry(this);
		this->m_model_registry = new model_registry;
		this->m_material_registry = new material_registry;
		auto ed = this->m_input_manager->get_enumerated_devices();
		for (size_t i = 0; i < ed.size(); i++) {
			this->m_input_manager->add_device(i);
		}
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
#ifdef BANANA_DEBUG
		int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(opengl_debug_function, this);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, NULL, NULL, true);
		}
#endif
		this->m_show_cursor = false;
		this->m_clip_cursor = true;
		this->m_debug_menus_initialized = false;
		this->m_steam_initialized = false;
		this->m_module = NULL;
		this->fill_imgui_input_pointers();
		std::string filename = platform_specific::get_module_name((platform_specific::module_t)this->m_executable);
		this->m_file_log = std::ofstream(filename + ".log");
		this->m_file_log << "[start of " + filename + ".log]\n";
		char sizebuf[256];
		sprintf(sizebuf, "width: %d, height: %d", width, height);
		this->debug_print("successfully created graphics context:\n\tbackend: " + graphics::get_backend_version() + "\n\t" + sizebuf);
	}
	game::~game() {
		glDisable(GL_DEBUG_OUTPUT);
		if (this->m_steam_initialized) this->shutdown_steam();
		if (this->m_debug_menus_initialized) debug::clean_up_imgui();
		delete this->m_scene;
		delete this->m_material_registry;
		delete this->m_model_registry;
		delete this->m_context;
		delete this->m_shader_registry;
		delete this->m_input_manager;
		delete this->m_module;
		delete this->m_script_registry;
#ifdef BANANA_MACOSX
        delete (platform_specific::_window_t*)this->m_window.m;
#endif
	}
	void game::destroy() {
		platform_specific::destroy_window(this->m_window.m);
	}
	void game::update() {
		if (!this->m_show_cursor) {
			const char* location = platform_specific::make_cursor_location(platform_specific::cursor_types::none);
			platform_specific::cursor_t cursor = platform_specific::load_cursor(location);
			platform_specific::set_cursor(cursor);
		}
		this->m_frame++;
		this->m_input_manager->update_devices();
		this->m_scene->update();
	}
	void game::render() {
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		this->clear_screen();
		this->m_scene->render();
		if (this->m_debug_menus_initialized) debug::render_imgui(this);
	}
	unsigned int game::get_current_frame() {
		return this->m_frame;
	}
	input_manager* game::get_input_manager() {
		return this->m_input_manager;
	}
	model_registry* game::get_model_registry() {
		return this->m_model_registry;
	}
	ui::menu_manager* game::get_menu_manager() {
		return this->m_menu_manager;
	}
#ifdef BANANA_WINDOWS
	long long __stdcall game::wndproc(HWND window, unsigned int msg, unsigned long long w_param, long long l_param) {
		ImGui_ImplWin32_WndProcHandler(window, msg, w_param, l_param);
		switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
		{
			if (g_game) {
				g_game->update_aspect_ratio();
				size_t width, height;
				platform_specific::get_size(g_game->m_window.m, width, height);
				glViewport(0, 0, (int)width, (int)height);
			}
		}
			break;
		default:
			return DefWindowProc(window, msg, w_param, l_param);
			break;
		}
		return 0;
	}
#endif
	float game::get_aspect_ratio() {
		return this->m_aspect_ratio;
	}
	scene* game::get_scene() {
		return this->m_scene;
	}
	void game::add_model_desc(const model_registry::model_descriptor& desc) {
		this->m_descriptors.push_back(desc);
	}
	void game::load_models() {
		this->m_model_registry->load(this->m_descriptors);
	}
	window& game::get_window() {
		return this->m_window;
	}
	sound::sound_manager* game::get_sound_manager() {
		return this->m_sound_manager;
	}
	lua_interpreter* game::get_lua_interpreter() {
		return this->m_interpreter;
	}
	bool& game::showing_cursor() {
		return this->m_show_cursor;
	}
	void game::show_cursor() {
		this->m_show_cursor = true;
		const char* location = platform_specific::make_cursor_location(platform_specific::cursor_types::arrow);
		platform_specific::cursor_t cursor = platform_specific::load_cursor(location);
		platform_specific::set_cursor(cursor);
	}
	void game::hide_cursor() {
		this->m_show_cursor = false;
		const char* location = platform_specific::make_cursor_location(platform_specific::cursor_types::none);
		platform_specific::cursor_t cursor = platform_specific::load_cursor(location);
		platform_specific::set_cursor(cursor);
	}
	void game::toggle_cursor() {
		if (this->m_show_cursor) this->hide_cursor();
		else this->show_cursor();
	}
	std::string game::get_steam_name() {
		if (this->m_steam_initialized) return std::string("todo: implement");
		else return "test player";
	}
	shader_registry* game::get_shader_registry() {
		return this->m_shader_registry;
	}
	script_registry* game::get_script_registry() {
		return this->m_script_registry;
	}
	material_registry* game::get_material_registry() {
		return this->m_material_registry;
	}
	void game::init_debug_menus() {
		debug::init_imgui(this->m_window.m);
		this->m_debug_menus_initialized = true;
	}
	void game::init_steam() {
		// todo: init
		this->m_steam_initialized = true;
	}
	bool game::should_clip_cursor() {
		return this->m_clip_cursor;
	}
	void game::clip_cursor() {
		this->m_clip_cursor = true;
	}
	void game::unclip_cursor() {
		this->m_clip_cursor = false;
	}
	void game::toggle_cursor_clip() {
		this->m_clip_cursor = !this->m_clip_cursor;
	}
	void game::make_context_current() {
		this->m_context->bind();
	}
	void game::swap_buffers() {
		this->m_context->swap_buffers();
	}
	void game::clear_screen() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	float game::get_timestep() {
		return ::get_current_time();
	}
	std::vector<std::string> game::get_command_line() {
		std::vector<std::string> arguments;
		std::string cmdline = platform_specific::get_command_line();
		size_t offset = 0;
		do {
			size_t nextpos = cmdline.find("\" ", offset);
			if (nextpos == std::string::npos) nextpos = cmdline.find(' ', offset);
			else nextpos++;
			std::string arg;
			if (cmdline[offset] == '\"') {
				size_t start = offset + 1;
				size_t end;
				if (nextpos != std::string::npos) end = nextpos - 1;
				else end = cmdline.length() - 1;
				arg = cmdline.substr(start, end - start);
			} else arg = cmdline.substr(offset, nextpos - offset);
			offset = (nextpos == std::string::npos ? nextpos : nextpos + 1);
			arguments.push_back(arg);
		} while (offset != std::string::npos);
		return arguments;
	}
	void game::load_script_module(const std::string& dllpath) {
		delete this->m_module;
		this->m_module = new script_module(this->m_script_registry, dllpath);
	}
	void game::unload_script_module() {
		delete this->m_module;
		this->m_module = NULL;
	}
	void game::debug_print(const std::string& message) {
		std::string msg = message + "\n";
		this->m_debug_log << msg;
		this->m_file_log << msg;
#ifdef BANANA_WINDOWS
		OutputDebugStringA(msg.c_str());
#else
		std::cout << msg << std::flush;
#endif
	}
	std::list<rigidbody*>& game::get_rigidbody_list() {
		return this->m_rigidbodies;
	}
	std::string game::get_debug_log() {
		return this->m_debug_log.str();
	}
	glm::vec2 game::get_window_size() {
		if (this->m_has_custom_size) {
			return this->m_custom_size;
		} else {
			size_t width, height;
			platform_specific::get_size(this->m_window.m, width, height);
			return glm::vec2((float)width, (float)height);
		}
	}
	void game::shutdown_steam() {
		// todo: shutdown
	}
	static void int_input(const char* label, int* v) {
		ImGui::InputInt(label, v);
	}
	static void bool_input(const char* label, bool* v) {
		ImGui::Checkbox(label, v);
	}
	static void float_input(const char* label, float* v) {
		ImGui::DragFloat(label, v);
	}
	static void double_input(const char* label, double* v) {
		ImGui::InputDouble(label, v);
	}
	static void float2_input(const char* label, glm::vec2* v) {
		ImGui::DragFloat2(label, &v->x);
	}
	static void float3_input(const char* label, glm::vec3* v) {
		ImGui::DragFloat3(label, &v->x);
	}
	static void float4_input(const char* label, glm::vec4* v) {
		ImGui::DragFloat4(label, &v->x);
	}
	static void text_input(const char* label, std::string* v) {
		ImGui::InputText(label, v);
	}
	static void dropdown_input(const char* label, property_classes::dropdown* value) {
		const std::vector<std::string>& std_items = value->get_items();
		std::vector<const char*> items;
		for (auto& str : std_items) {
			items.push_back(str.c_str());
		}
		ImGui::Combo(label, value->get_index_ptr(), items.data(), items.size());
	}
	static void readonly_input(const char* label, property_classes::read_only_text* value) {
		ImGui::InputText(label, &value->get_text(), ImGuiInputTextFlags_ReadOnly);
	}
	static void color_input(const char* label, property_classes::color* value) {
		ImGui::ColorEdit3(label, &value->get_vector().x);
	}
	void game::fill_imgui_input_pointers() {
		this->m_imgui_input_functions[typeid(int).hash_code()] = (void*)int_input;
		this->m_imgui_input_functions[typeid(bool).hash_code()] = (void*)bool_input;
		this->m_imgui_input_functions[typeid(float).hash_code()] = (void*)float_input;
		this->m_imgui_input_functions[typeid(double).hash_code()] = (void*)double_input;
		this->m_imgui_input_functions[typeid(std::string).hash_code()] = (void*)text_input;
		this->m_imgui_input_functions[typeid(glm::vec2).hash_code()] = (void*)float2_input;
		this->m_imgui_input_functions[typeid(glm::vec3).hash_code()] = (void*)float3_input;
		this->m_imgui_input_functions[typeid(glm::vec4).hash_code()] = (void*)float4_input;
		this->m_imgui_input_functions[typeid(property_classes::dropdown).hash_code()] = (void*)dropdown_input;
		this->m_imgui_input_functions[typeid(property_classes::read_only_text).hash_code()] = (void*)readonly_input;
		this->m_imgui_input_functions[typeid(property_classes::color).hash_code()] = (void*)color_input;
	}
	void game::update_aspect_ratio() {
		glm::vec2 size = this->get_window_size();
		this->m_aspect_ratio = size.x / size.y;
	}
	void game::set_aspect_ratio(float ratio) {
		this->m_aspect_ratio = ratio;
	}
	void game::set_custom_window_size(glm::vec2 size) {
		this->m_has_custom_size = true;
		this->m_custom_size = size;
	}
	void window::setup_window() {
		platform_specific::setup_window();
	}
	int window::window_loop(application_layer* app_layer) {
		return platform_specific::window_loop(app_layer);
	}
}
