#include <GL/freeglut.h>

#ifndef __RENDERINFO_H__
#define __RENDERINFO_H__

struct render_info {
	VSShaderLib shader;
	GLint vm_uniformId;
	GLint pvm_uniformId;
	GLint normal_uniformId;
	GLint textMode_uniformId;
};

#endif