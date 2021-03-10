#pragma once
#ifdef BANANA_BUILD
#include <GL/glew.h>
#ifdef BANANA_WINDOWS
#include <GL/wglew.h>
#endif
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>
#include <steam/steam_api.h>
#include <yaml-cpp/yaml.h>
#include <opengl_shader.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif