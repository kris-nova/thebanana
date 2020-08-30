#include "pch.h"
#include "prop.h"
prop::prop(const std::string& model_name) : m_model_name(model_name) {
	std::string output = "created prop with model: " + model_name + "\n";
	OutputDebugStringA(output.c_str());
	this->m_nickname = "prop: " + model_name;
}
void prop::update() {
	this->prepare_for_update();
	this->post_update();
}
void prop::render() {
	this->prepare_for_render();
	this->render_model(this->m_model_name);
	this->post_render();
}
prop::~prop() { }