#include "pch.h"
#include "static_mesh.h"
#include "debug_tools.h"
#include "physics/rigidbody.h"
#include "components/mesh.h"
namespace thebanana {
	static_mesh::static_mesh(const std::string& model_name) {
		this->m_nickname = "static mesh";
		this->add_tag("staticmesh");
		this->add_component<mesh_component>().set_mesh_name(model_name);
		this->add_component<rigidbody>().set_collision_model_name(model_name);
	}
	static_mesh::~static_mesh() { }
}