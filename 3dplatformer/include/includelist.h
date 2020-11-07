#pragma once
#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Windows.h>
#include <initguid.h>
#include <dinput.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef interface
#undef interface
#endif
#include <model_loader.h>
#include <opengl_shader.h>
#include <opengl_viewport.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <chrono>
#include <cassert>
#include <typeinfo>
#include <thread>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkSurface.h>
#include <include/utils/SkRandom.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <AL/al.h>
#include <AL/alc.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <lua.h>
#include <lualib.h>
#ifdef __cplusplus
}
#endif
#include <lua.hpp>