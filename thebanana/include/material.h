#pragma once
#include "graphics/texture.h"
namespace thebanana {
	class material {
	public:
		material();
		void set_albedo(const std::string& image_path);
		void set_albedo(void* data, int width, int height, int channels, bool clear_path = true);
		void set_albedo(graphics::texture* texture);
		const std::shared_ptr<graphics::texture>& get_albedo();
		std::string get_albedo_path();
		void set_color(glm::vec3 color);
		glm::vec3 get_color();
		void set_shininess(float shininess);
		float get_shininess();
		void send_to_shader(unsigned int shader_id, const std::string& uniform_name);
		void set_uuid(unsigned long long uuid);
		unsigned long long get_uuid();
	private:
		std::shared_ptr<graphics::texture> m_albedo;
		glm::vec3 m_color;
		float m_shininess;
		unsigned long long m_uuid;
		std::string m_albedo_path;
	};
	class material_registry {
	public:
		unsigned long long new_material();
		material* find(unsigned long long uuid);
		void clear();
		size_t get_count();
		material* get(size_t index);
	private:
		std::vector<std::unique_ptr<material>> m_materials;
	};
}