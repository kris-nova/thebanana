#include <thebanana.h>
#include "camera.h"
camera_behavior::camera_behavior(thebanana::gameobject* object, thebanana::native_script_component* nsc) : script(object, nsc) {
	thebanana::debug::log_print("created camera");
	this->m_direction = glm::vec3(0.f, 0.f, 1.f);
	this->m_angle = glm::vec2(0.f, -90.f);
	this->parent->get_nickname() = "camera";
	this->add_property(new thebanana::component::property<float>(2.f, "distance"));
#ifdef _DEBUG
	this->remove_component<thebanana::debug_component>();
#endif
}
void camera_behavior::update() {
#ifdef _DEBUG
	if (thebanana::debug::control) {
#endif
		glm::vec2 result;
		if (this->parent->get_game()->get_input_manager()->get_device_type(0) == thebanana::input_manager::device_type::controller) {
			thebanana::controller* c = (thebanana::controller*)this->parent->get_game()->get_input_manager()->get_device(0);
			result = c->get_joysticks().right;
		}
		else {
			static glm::vec2 last(0.f);
			thebanana::mouse* m = (thebanana::mouse*)this->parent->get_game()->get_input_manager()->get_device(1);
			glm::vec2 current = m->get_pos();
			result = current - last;
			last = current;
		}
		result *= glm::vec2(10.f, -10.f);
		result = glm::vec2(result.y, result.x);
		this->m_angle += result;
		const float limit = 89.f;
		if (this->m_angle.x > limit)
			this->m_angle.x = limit;
		if (this->m_angle.x < -limit)
			this->m_angle.x = -limit;
		glm::vec3 d;
		d.x = cos(glm::radians(this->m_angle.y)) * cos(glm::radians(this->m_angle.x));
		d.y = sin(glm::radians(this->m_angle.x));
		d.z = sin(glm::radians(this->m_angle.y)) * cos(glm::radians(this->m_angle.x));
		this->m_direction = glm::normalize(d);
#ifdef _DEBUG
	}
#endif
	thebanana::component::property<float>* distance = this->find_property<float>("distance");
	this->get_transform() = thebanana::transform().translate(glm::vec3(this->m_player->get_transform()) + this->m_direction * (distance ? * distance->get_value() : 2.f));
}
void camera_behavior::render() {
	glm::mat4 projection = glm::perspective(glm::radians(45.f), this->parent->get_game()->get_aspect_ratio(), 0.1f, 100.f);
	this->parent->get_scene()->get_shader()->get_uniforms().mat4("projection",projection);
	glm::vec3 pos = this->get_transform().get_matrix() * glm::vec4(0.f, 1.f, 0.f, 1.f);
	glm::vec3 player_pos = this->m_player->get_transform().get_matrix() * glm::vec4(0.f, 0.75f, 0.f, 1.f);
	glm::mat4 rotation = this->m_player->get_transform().get_matrix();
	rotation[3] = glm::vec4(0.f, 0.f, 0.f, rotation[3].w);
	glm::vec3 up = glm::vec3(rotation * glm::vec4(0.f, 1.f, 0.f, 1.f));
	glm::mat4 view = glm::lookAt(pos, player_pos, up);
	this->parent->get_scene()->get_shader()->get_uniforms().mat4("view", view);
}
glm::vec3 camera_behavior::get_direction() {
	return this->m_direction;
}
glm::vec2 camera_behavior::get_angle() {
	return this->m_angle;
}
void camera_behavior::set_player(thebanana::gameobject* p) {
	this->m_player = p;
}
