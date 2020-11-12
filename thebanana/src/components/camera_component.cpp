#include "pch.h"
#include "components/camera_component.h"
#include "gameobject.h"
#include "game.h"
#include "scene.h"
namespace thebanana {
	camera_component::camera_component(gameobject* object) : component(object) {
		this->add_property(new property<glm::vec3>(glm::vec3(0.f, 0.f, 1.f), "Direction"));
		this->add_property(new property<glm::vec2>(glm::vec2(0.f, -90.f), "Angle"));
	}
	void camera_component::update() {
		glm::vec2 angle = *this->get_property<glm::vec2>("Angle");
		glm::vec3 d;
		d.x = cos(glm::radians(angle.y)) * cos(glm::radians(angle.x));
		d.y = sin(glm::radians(angle.x));
		d.z = sin(glm::radians(angle.y)) * cos(glm::radians(angle.x));
		this->set_property<glm::vec3>("Direction", glm::normalize(d));
	}
	void camera_component::render() {
		glm::mat4 projection = glm::perspective(glm::radians(45.f), this->parent->get_game()->get_aspect_ratio(), 0.1f, 100.f);
		this->parent->get_scene()->get_shader()->get_uniforms().mat4("projection", projection);
		glm::vec3 pos = (glm::vec3)this->get_transform();
		glm::vec3 direction = *this->get_property<glm::vec3>("Direction");
		glm::mat4 view = glm::lookAt(pos, pos + direction, glm::vec3(0.f, 1.f, 0.f));
		this->parent->get_scene()->get_shader()->get_uniforms().mat4("view", view);
	}
}