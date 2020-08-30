#include "pch.h"
#include "keyboard.h"
#include "input_utils.h"
#include "debug_tools.h"
keyboard::keyboard() : is_connected(true) { }
keyboard::~keyboard() { }
std::vector<keyboard::button> keyboard::get_buttons() {
	return this->current;
}
void keyboard::update() {
	this->is_connected = SUCCEEDED(this->device->Poll());
	this->device->GetDeviceState(sizeof(unsigned char) * 256, &this->state);
	for (size_t i = 0; i < this->keys; i++) {
		this->last[i] = this->current[i].held;
	}
	calc_pressed(this->current, this->state, this->keys);
	calc_down(this->current, this->last, this->keys);
	calc_up(this->current, this->last, this->keys);
}
bool keyboard::connected() {
	return this->is_connected;
}
input_manager::device_type keyboard::get_type() {
	return input_manager::device_type::keyboard;
}
const DIDATAFORMAT* keyboard::get_format() {
	return &c_dfDIKeyboard;
}
void keyboard::device_specific_init() {
	DIDEVCAPS capabilities;
	ZeroMemory(&capabilities, sizeof(DIDEVCAPS));
	capabilities.dwSize = sizeof(DIDEVCAPS);
	this->device->GetCapabilities(&capabilities);
	this->keys = capabilities.dwButtons;
	this->current.resize(this->keys);
	this->last.resize(this->keys);
	log_print("created keyboard");
}
