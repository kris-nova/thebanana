#pragma once
#include "gameobject.h"
class null_object : public gameobject {
public:
	null_object();
	virtual void update() override;
	virtual void render() override;
	virtual float& get_last_walk_speed() override;
};