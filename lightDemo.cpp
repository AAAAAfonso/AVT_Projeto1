//
// AVT: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: Joï¿½o Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>


// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>

//assimp lib
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "flare.h"

#include "avtFreeType.h"

#include "Texture_Loader.h"

#include <random>

#include "Camera.h"

#include "update_info.h"
#include "render_info.h"
#include "Terrain.h"
#include "Sleigh.h"
#include "Snowball.h"
#include "Lamppost.h"
#include "House.h"
#include "Tree.h"
#include "Statue.h"
#include "Particle.h"
#include "Present.h"
#include "LensFlare.h"
#include "Skybox.h"



using namespace std;

#define CAPTION "AVT Demo: Phong Shading and Text rendered with FreeType"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

#define FPS 75

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";
bool renderFlare = true;

//Vector with meshes
struct MyMesh rearViewModel;
vector<struct MyMesh> myMeshes;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

extern const aiScene* scene;

char model_dir[50];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint tex_loc, tex_loc1, tex_loc2, tex_loc3, bump_loc;

GLint dirLPos_uniformId;
GLint dirLToggled_uniformId;

vector<GLint> pointLPos_uniformIds;
GLint pointLToggled_uniformId;

vector<GLint> spotLPos_uniformIds;
GLint spotLSpot_uniformId;
GLint spotLThreshold_uniformId;
GLint spotLToggled_uniformId;

bool fogToggled = true;
GLint fogToggled_uniformId;

GLint textMode_uniformId;
GLint shadowMode_uniformId;
GLuint TextureArray[8];
GLfloat plano_chao[4] = { 0,1,0,0 };

GLint normalMap_loc;
GLint specularMap_loc;
GLint diffMapCount_loc;



// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 15.0f;

//Flare effect
float lightScreenPos[3];  //Position of the light in Window Coordinates
GLuint FlareTextureArray[6];

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
float dirLightPos[4] = { 1.0f, -0.5f, 0.0f, 0.0f };

bool dirLightToggled = true;
bool pointLightToggled = true;
bool spotLightToggled = true;

float spotLightAngle = 10.0f;

vector<Camera> cams;
short active_camera = 0;

float ratio = WinX / WinY;

struct update_info uInfo = { 0.0f, 0.0f, 0.0f };
struct keyboard_key_tracking uTrack = { false, false, false, false, false };


// Create objects
Terrain* terrain;
Sleigh* sleigh;
vector<Lamppost> lampposts;
vector<SnowBall> snowballs;
vector<House> houses;
vector<Tree> trees;
Statue* statue;
vector<Particle> particles;
Present* present;
Skybox* skybox;
LensFlare* lensflare;
MyMesh mirror;


bool paused = false;
int type = 0;




void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

void updateGameSpeed(int value) {
	for (int i = 0; i < snowballs.size(); i++) {
		snowballs[i].updateSnowBallSpeed();
	}
	glutTimerFunc(10000, updateGameSpeed, 0);
}
void refresh(int value)
{
	if (!paused) {
		sleigh->update(1.0f / FPS, &uInfo);
		if (!tracking)
			cams[2].update(sleigh->get_pos(), sleigh->get_direction());
		for (int i = 0; i < snowballs.size(); i++) {
			snowballs[i].updateSnowBallPosition(1.0f / FPS);
		}
		for (int i = 0; i < houses.size(); i++) {
			if (houses[i].getColided()) {
				houses[i].updateHouse(1.0 / FPS);
			}
		}
		for (int i = 0; i < trees.size(); i++) {
			if (trees[i].getColided()) {
				trees[i].updateTree(1.0 / FPS);
			}
		}
		for (int i = 0; i < lampposts.size(); i++) {
			if (lampposts[i].getColided()) {
				lampposts[i].updateLamppost(1.0 / FPS);
			}
		}
		if (statue->getColided()) {
			statue->updateStatue(1.0 / FPS);
		}
		for (int i = 0; i < particles.size(); i++) {
			particles[i].update(1.0 / FPS);
		}
		if (present->getColided()) {
			delete present;
			present = new Present((rand() % 200) / 10 - 10.0f, (rand() % 200) / 10 - 10.0f);
			uInfo.present = present;
		}
	}

	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, refresh, 0);

}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
}


// ------------------------------------------------------------
//
// Render stufff
//
void clearStencil(void) {
	glClear(GL_STENCIL_BUFFER_BIT);
}

void setRearViewStencil(void) {
	glEnable(GL_STENCIL_TEST);

	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	int w = m_viewport[2]; int h = m_viewport[3];

	loadIdentity(PROJECTION);
	if (w <= h)
		ortho(-2.0, 2.0, -2.0 * (GLfloat)h / (GLfloat)w,
			2.0 * (GLfloat)h / (GLfloat)w, -10, 10);
	else
		ortho(-2.0 * (GLfloat)w / (GLfloat)h,
			2.0 * (GLfloat)w / (GLfloat)h, -2.0, 2.0, -10, 10);

	// load identity matrices for Model-View
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	glUseProgram(shader.getProgramIndex());

	//n�o vai ser preciso enviar o material pois o cubo n�o � desenhado

	translate(MODEL, -0.9f, 1.2f, -0.5f);
	scale(MODEL, 1.8f, 0.6f, 1.0f);
	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	//glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	glStencilFunc(GL_NEVER, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	glBindVertexArray(rearViewModel.vao);
	glDrawElements(rearViewModel.type, rearViewModel.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);

	glDisable(GL_STENCIL_TEST);
}


void setMirrorStencil(void) {
	glEnable(GL_STENCIL_TEST);

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	loadIdentity(PROJECTION);
	if (cams[active_camera].get_type() == 0) {
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
	else if (cams[active_camera].get_type() == 1) {
		ortho(-12.5f * ratio, 12.5f * ratio, -12.5f, 12.5f, -1, 100);
	}
	lookAt(cams[active_camera].get_pos(0), cams[active_camera].get_pos(1), cams[active_camera].get_pos(2),
		cams[active_camera].get_target(0), cams[active_camera].get_target(1), cams[active_camera].get_target(2),
		cams[active_camera].get_up(0), cams[active_camera].get_up(1), cams[active_camera].get_up(2));


	glUseProgram(shader.getProgramIndex());

	//n�o vai ser preciso enviar o material pois o cubo n�o � desenhado

	translate(MODEL, -7.0f, 1.5f, 0.0f);
	rotate(MODEL, 90, 0.0f, 1.0f, 0.0f);
	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	glStencilFunc(GL_NEVER, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	glBindVertexArray(mirror.vao);
	glDrawElements(mirror.type, mirror.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);

	glDisable(GL_STENCIL_TEST);
}


void renderRearView(void) {
	clearStencil();
	setRearViewStencil();

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);
	float* p = sleigh->get_pos();
	float *dir = sleigh->get_direction();
	lookAt(p[0] - 0.5*dir[0], p[1] - 0.5*dir[1], p[2] - 0.5*dir[2],
		p[0] - 2*dir[0], p[1] - 2*dir[1], p[2] - 2*dir[2],
		cams[2].get_up(0), cams[2].get_up(1), cams[2].get_up(2));

	// use our shader

	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	float res[4];
	multMatrixPoint(VIEW, dirLightPos, res);
	glUniform4fv(dirLPos_uniformId, 1, res);
	glUniform1i(dirLToggled_uniformId, dirLightToggled);

	for (int i = 0; i < 6; i++) {
		float* pos = lampposts[i].get_pointlight_pos();
		multMatrixPoint(VIEW, pos, res);
		delete[] pos;
		glUniform4fv(pointLPos_uniformIds[i], 1, res);
	}
	glUniform1i(pointLToggled_uniformId, pointLightToggled);

	for (int i = 0; i < 2; i++) {
		float* pos = sleigh->get_spotlight_pos(i);
		multMatrixPoint(VIEW, pos, res);
		delete[] pos;
		glUniform4fv(spotLPos_uniformIds[i], 1, res);
	}
	dir[3] = 0.0f;
	multMatrixPoint(VIEW, dir, res);
	glUniform4fv(spotLSpot_uniformId, 1, res);
	glUniform1f(spotLThreshold_uniformId, cos(spotLightAngle * 3.14f / 180));
	glUniform1i(spotLToggled_uniformId, spotLightToggled);

	glUniform1i(fogToggled_uniformId, fogToggled);

	struct render_info rInfo = { shader, vm_uniformId, pvm_uniformId, normal_uniformId, textMode_uniformId, TextureArray,  cams[active_camera].get_xyzpos(), 
		normalMap_loc, specularMap_loc, diffMapCount_loc };

	// draw the tori where the stencil is not 1 
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	sleigh->render(rInfo);
	skybox->render(rInfo);
	terrain->render(rInfo);
	for (int i = 0; i < lampposts.size(); i++) lampposts[i].render(rInfo);
	for (int i = 0; i < snowballs.size(); i++) snowballs[i].render(rInfo);
	for (int i = 0; i < houses.size(); i++) houses[i].render(rInfo);
	for (int i = 0; i < trees.size(); i++) trees[i].render(rInfo, type);
	for (int i = 0; i < particles.size(); i++) particles[i].render(rInfo);
	present->render(rInfo);
	statue->render(rInfo);

	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
}


void renderMirror(void) {
	clearStencil();
	setMirrorStencil();

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	loadIdentity(PROJECTION);
	if (cams[active_camera].get_type() == 0) {
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
	else if (cams[active_camera].get_type() == 1) {
		ortho(-12.5f * ratio, 12.5f * ratio, -12.5f, 12.5f, -1, 100);
	}
	lookAt(cams[active_camera].get_pos(0), cams[active_camera].get_pos(1), cams[active_camera].get_pos(2),
		cams[active_camera].get_target(0), cams[active_camera].get_target(1), cams[active_camera].get_target(2),
		cams[active_camera].get_up(0), cams[active_camera].get_up(1), cams[active_camera].get_up(2));

	// use our shader

	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	float res[4];


	multMatrixPoint(VIEW, dirLightPos, res);
	glUniform4fv(dirLPos_uniformId, 1, res);
	glUniform1i(dirLToggled_uniformId, dirLightToggled);

	for (int i = 0; i < lampposts.size(); i++) {
		float* pos = lampposts[i].get_pointlight_pos();
		multMatrixPoint(VIEW, pos, res);
		delete[] pos;
		glUniform4fv(pointLPos_uniformIds[i], 1, res);
	}
	glUniform1i(pointLToggled_uniformId, pointLightToggled);

	for (int i = 0; i < 2; i++) {
		float* pos = sleigh->get_spotlight_pos(i);
		multMatrixPoint(VIEW, pos, res);
		delete[] pos;
		glUniform4fv(spotLPos_uniformIds[i], 1, res);
	}
	float* dir = sleigh->get_direction();
	dir[3] = 0.0f;
	multMatrixPoint(VIEW, dir, res);
	glUniform4fv(spotLSpot_uniformId, 1, res);
	glUniform1f(spotLThreshold_uniformId, cos(spotLightAngle * 3.14f / 180));
	glUniform1i(spotLToggled_uniformId, spotLightToggled);

	glUniform1i(fogToggled_uniformId, fogToggled);

	struct render_info rInfo = { shader, vm_uniformId, pvm_uniformId, normal_uniformId, textMode_uniformId, TextureArray,  cams[active_camera].get_xyzpos(),
		normalMap_loc, specularMap_loc, diffMapCount_loc };

	// draw the tori where the stencil is not 1
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	sleigh->render(rInfo);
	skybox->render(rInfo);
	terrain->render(rInfo);
	for (int i = 0; i < lampposts.size(); i++) lampposts[i].render_reflected(rInfo);
	for (int i = 0; i < snowballs.size(); i++) snowballs[i].render_reflected(rInfo);
	for (int i = 0; i < houses.size(); i++) houses[i].render_reflected(rInfo);
	for (int i = 0; i < trees.size(); i++) trees[i].render_reflected(rInfo, type);
	for (int i = 0; i < particles.size(); i++) particles[i].render_reflected(rInfo);
	present->render_reflected(rInfo);
	statue->render_reflected(rInfo);

	glDisable(GL_STENCIL_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mirror.mat.ambient);
	loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mirror.mat.diffuse);
	loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mirror.mat.specular);
	loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mirror.mat.shininess);

	glUniform1i(rInfo.textMode_uniformId, 1);

	translate(MODEL, -7.0f, 1.5f, 0.0f);
	rotate(MODEL, 90, 0.0f, 1.0f, 0.0f);
	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	glStencilFunc(GL_NEVER, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	glBindVertexArray(mirror.vao);
	glDrawElements(mirror.type, mirror.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);

	glDisable(GL_BLEND);
}



void renderShadows(void) {
	struct render_info rInfo = { shader, vm_uniformId, pvm_uniformId, normal_uniformId, textMode_uniformId, TextureArray,  cams[active_camera].get_xyzpos(),
		normalMap_loc, specularMap_loc, diffMapCount_loc };
	float mat[16];
	float res[4];
	glEnable(GL_DEPTH_TEST);
	float* lightPos = lampposts[3].get_pointlight_pos();
	lightPos[1] -= 0.5;
	if (cams[active_camera].get_pos(1) > 0 && pointLightToggled) {
		//SHADOWS
		glEnable(GL_STENCIL_TEST);        // Escrever 1 no stencil buffer onde se for desenhar a reflexão e a sombra
		glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
		glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);

		terrain->render(rInfo);
		
		glUniform1i(shadowMode_uniformId, 0);  //iluminação phong

		// Desenhar apenas onde o stencil buffer esta a 1
		glStencilFunc(GL_EQUAL, 0x1, 0x1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// Render the reflected geometry REFLEXO TO DO
		/*lightPos[1] *= (-1.0f);  //mirror the position of light
		multMatrixPoint(VIEW, lightPos, res);

		glUniform4fv(pointLPos_uniformIds[3], 1, res);
		pushMatrix(MODEL);
		scale(MODEL, 1.0f, -1.0f, 1.0f);
		glCullFace(GL_FRONT);
		sleigh->render(rInfo);
		for (int i = 0; i < lampposts.size(); i++) lampposts[i].render(rInfo);
		for (int i = 0; i < snowballs.size(); i++) snowballs[i].render(rInfo);
		for (int i = 0; i < houses.size(); i++) houses[i].render(rInfo);
		present->render(rInfo);
		glCullFace(GL_BACK);
		popMatrix(MODEL);

		lightPos[1] *= (-1.0f);  //reset the light position
		multMatrixPoint(VIEW, lightPos, res);
		glUniform4fv(pointLPos_uniformIds[3], 1, res);*/

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Blend specular Ground with reflected geometry
		terrain->render(rInfo);

		// Render the Shadows
		glUniform1i(shadowMode_uniformId, 1);  //Render with constant color
		shadow_matrix(mat, plano_chao, lightPos);

		glDisable(GL_DEPTH_TEST); //To force the shadow geometry to be rendered even if behind the floor

		//Dark the color stored in color buffer
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		pushMatrix(MODEL);
		multMatrix(MODEL, mat);
		sleigh->render(rInfo);
		for (int i = 0; i < lampposts.size(); i++) lampposts[i].render(rInfo);
		for (int i = 0; i < snowballs.size(); i++) snowballs[i].render(rInfo);
		for (int i = 0; i < houses.size(); i++) houses[i].render(rInfo);
		present->render(rInfo);
		popMatrix(MODEL);

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glUniform1i(shadowMode_uniformId, 0);  //Render with constant color

	}
}

void renderScene(void) {
	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderMirror();
	
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	loadIdentity(PROJECTION);
	if (cams[active_camera].get_type() == 0) {
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
	else if (cams[active_camera].get_type() == 1) {
		ortho(-12.5f * ratio, 12.5f * ratio, -12.5f, 12.5f, -1, 100);
	}
	lookAt(cams[active_camera].get_pos(0), cams[active_camera].get_pos(1), cams[active_camera].get_pos(2),
		cams[active_camera].get_target(0), cams[active_camera].get_target(1), cams[active_camera].get_target(2),
		cams[active_camera].get_up(0), cams[active_camera].get_up(1), cams[active_camera].get_up(2));

	// use our shader

	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	float res[4];

	
	multMatrixPoint(VIEW, dirLightPos, res);
	glUniform4fv(dirLPos_uniformId, 1, res);
	glUniform1i(dirLToggled_uniformId, dirLightToggled);

	for (int i = 0; i < lampposts.size(); i++) {
		float* pos = lampposts[i].get_pointlight_pos();
		multMatrixPoint(VIEW, pos, res);
		delete[] pos;
		glUniform4fv(pointLPos_uniformIds[i], 1, res);
	}
	glUniform1i(pointLToggled_uniformId, pointLightToggled);

	for (int i = 0; i < 2; i++) {
		float* pos = sleigh->get_spotlight_pos(i);
		multMatrixPoint(VIEW, pos, res);
		delete[] pos;
		glUniform4fv(spotLPos_uniformIds[i], 1, res);
	}
	float* dir = sleigh->get_direction();
	dir[3] = 0.0f;
	multMatrixPoint(VIEW, dir, res);
	glUniform4fv(spotLSpot_uniformId, 1, res);
	glUniform1f(spotLThreshold_uniformId, cos(spotLightAngle * 3.14f / 180));
	glUniform1i(spotLToggled_uniformId, spotLightToggled);

	glUniform1i(fogToggled_uniformId, fogToggled);

	struct render_info rInfo = { shader, vm_uniformId, pvm_uniformId, normal_uniformId, textMode_uniformId, TextureArray, cams[active_camera].get_xyzpos()};

	// draw the tori where the stencil is not 1
	renderShadows();
	sleigh->render(rInfo);
	skybox->render(rInfo);
	terrain->render(rInfo);
	for (int i = 0; i < lampposts.size(); i++) lampposts[i].render(rInfo);
	for (int i = 0; i < snowballs.size(); i++) snowballs[i].render(rInfo);
	for (int i = 0; i < houses.size(); i++) houses[i].render(rInfo);
	for (int i = 0; i < trees.size(); i++) trees[i].render(rInfo, type);
	for (int i = 0; i < particles.size(); i++) particles[i].render(rInfo);
	present->render(rInfo);
	statue->render(rInfo);
	
	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int flarePos[2];
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	pushMatrix(MODEL);
	loadIdentity(MODEL);
	computeDerivedMatrix(PROJ_VIEW_MODEL);  //pvm to be applied to lightPost. pvm is used in project function

	float* pos = lampposts[0].get_pointlight_pos();
	if (!project(pos, lightScreenPos, m_viewport))
		printf("Error in getting projected light in screen\n");  //Calculate the FIX THIS window Coordinates of the light position: the projected position of light on viewport
	flarePos[0] = clampi((int)lightScreenPos[0], m_viewport[0], m_viewport[0] + m_viewport[2] - 1);
	flarePos[1] = clampi((int)lightScreenPos[1], m_viewport[1], m_viewport[1] + m_viewport[3] - 1);
	
	popMatrix(MODEL);


	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	lensflare->render(rInfo, flarePos[0], flarePos[1], m_viewport);
	//if (paused)
		//RenderText(shaderText, "PAUSED", m_viewport[2] / 2.0f - 100.0f, m_viewport[3] / 2.0f + 25.0f, 1.0f, 1.0f, 1.0f, 1.01f);
	//RenderText(shaderText, "LIVES: ", 25.0f, m_viewport[3] - 50.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	//RenderText(shaderText, to_string(sleigh->get_lives()), 180.0f, m_viewport[3] - 50.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	//RenderText(shaderText, "POINTS: ", 25.0f, m_viewport[3] - 100.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	//RenderText(shaderText, to_string(sleigh->get_points()), 220.0f, m_viewport[3] - 100.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if (active_camera == 2) renderRearView();

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeysDown(unsigned char key, int xx, int yy)
{
	switch (key) {
	case 'w':
		if (uTrack.w_key == false)
			uInfo.v_turning += -1;
		uTrack.w_key = true;
		break;
	case 'a':
		if (uTrack.a_key == false)
			uInfo.h_turning += 1;
		uTrack.a_key = true;
		break;
	case 's':
		if (uTrack.s_key == false)

			uInfo.v_turning += 1;
		uTrack.s_key = true;

		break;
	case 'd':
		if (uTrack.d_key == false)
			uInfo.h_turning += -1;
		uTrack.d_key = true;
		break;
	case 'o':
		if (uTrack.o_key == false)
			uInfo.accelerating = 1;
		uTrack.o_key = true;
		break;

	case '1':
		active_camera = 0;
		break;
	case '2':
		active_camera = 1;
		break;
	case '3':
		active_camera = 2;
		break;
	case '4':
		type++; 
		if (type == 5) type = 0;
	case 'n':
		dirLightToggled = !dirLightToggled;
		break;
	case 'c':
		pointLightToggled = !pointLightToggled;
		break;
	case 'h':
		spotLightToggled = !spotLightToggled;
		break;
	case 'f':
		fogToggled = !fogToggled;
		break;
	case 'l':
		renderFlare = !renderFlare;
	case 27:
		glutLeaveMainLoop();
		break;

		//case 'c': 
		//	printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
		//	break;
		//case 'm': glEnable(GL_MULTISAMPLE); break;
		//case 'n': glDisable(GL_MULTISAMPLE); break;
	}
}


void processKeysUp(unsigned char key, int xx, int yy)
{
	switch (key) {
	case 'w':
		uInfo.v_turning -= -1;
		uTrack.w_key = false;
		break;
	case 'a':
		uInfo.h_turning -= 1;
		uTrack.a_key = false;
		break;
	case 's':
		uInfo.v_turning -= 1;
		uTrack.s_key = false;

		break;
	case 'd':
		uInfo.h_turning -= -1;
		uTrack.d_key = false;

		break;
	case 'o':
		uInfo.accelerating = -1;
		uTrack.o_key = false;
		break;

	case 'p':
		paused = !paused;
		break;
	case 'r':
		sleigh->missionFail();
		sleigh->reset();
		break;

	case 27:
		glutLeaveMainLoop();
		break;
	}
}

// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	cams[2].set_pos(0, rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f) + sleigh->get_pos()[0]);
	cams[2].set_pos(2, rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f) + sleigh->get_pos()[2]);
	cams[2].set_pos(1, rAux * sin(betaAux * 3.14f / 180.0f) + sleigh->get_pos()[1]);
	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	cams[2].set_pos(0, r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f));
	cams[2].set_pos(2, r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f));
	cams[2].set_pos(1, r * sin(beta * 3.14f / 180.0f));

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	tex_loc3 = glGetUniformLocation(shader.getProgramIndex(), "texmap3");
	bump_loc = glGetUniformLocation(shader.getProgramIndex(), "bumpmap");
	textMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "text_mode");
	shadowMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "shadowMode");
	normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");


	dirLPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "d_l_pos");
	dirLToggled_uniformId = glGetUniformLocation(shader.getProgramIndex(), "dir_l_toggled");

	pointLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "p_l_pos0"));
	pointLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "p_l_pos1"));
	pointLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "p_l_pos2"));
	pointLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "p_l_pos3"));
	pointLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "p_l_pos4"));
	pointLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "p_l_pos5"));
	pointLToggled_uniformId = glGetUniformLocation(shader.getProgramIndex(), "point_l_toggled");

	spotLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "s_l_pos0"));
	spotLPos_uniformIds.push_back(glGetUniformLocation(shader.getProgramIndex(), "s_l_pos1"));
	spotLSpot_uniformId = glGetUniformLocation(shader.getProgramIndex(), "s_l_spot");
	spotLThreshold_uniformId = glGetUniformLocation(shader.getProgramIndex(), "spot_l_threshold");
	spotLToggled_uniformId = glGetUniformLocation(shader.getProgramIndex(), "spot_l_toggled");

	fogToggled_uniformId = glGetUniformLocation(shader.getProgramIndex(), "fog_toggled");

	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	std::string filepath;

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	//Texture Object definition

	glGenTextures(8, TextureArray);
	Texture2D_Loader(TextureArray, "texmap.jpg", 0);
	Texture2D_Loader(TextureArray, "texmap1.jpg", 1);
	Texture2D_Loader(TextureArray, "texmap2.png", 2);
	Texture2D_Loader(TextureArray, "texmap3.png", 3);
	Texture2D_Loader(TextureArray, "bumpmap.jpg", 4);
	Texture2D_Loader(TextureArray, "texmap4.jpg", 5);
	Texture2D_Loader(TextureArray, "texmap5.jpg", 6);
	Texture2D_Loader(TextureArray, "tree.tga", 7);

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	lensflare = new LensFlare(1.0f, "./flare.txt"); 
	terrain = new Terrain(40.0f, 40.0f);
	sleigh = new Sleigh(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 360; i += 360 / 12) {
		snowballs.push_back(SnowBall(0.5f, i, 7.0f));
	}
	for (int i = 0; i < 6; i++) {
		lampposts.push_back(Lamppost(5.0f * ((i % 3) - 1), 2.5f * ((i / 3) * 2 - 1)));
	}
	for (int i = 0; i < 8; i++) {
		houses.push_back(House(5.0f * ((i % 4) - 1) - 2.5f, 4.0f * ((i / 4) * 2 - 1)));
	}
	sleigh->get_direction();
	for (int i = 0; i < 40; i++) {
		float size = rand() % 15 * 0.01f + 0.05f;
		trees.push_back(Tree(size, size * (2.0f + rand() % 10 * 0.2f), rand() % 39 - 19.5f + (rand() % 10) * 0.1f - 0.5, rand() % 13 - 18.5f + (rand() % 10) * 0.1f - 0.5));
		size = rand() % 15 * 0.01f + 0.05f;
		trees.push_back(Tree(size, size * (2.0f + rand() % 10 * 0.2f), rand() % 39 - 19.5f + (rand() % 10) * 0.1f - 0.5, rand() % 13 + 6.5f + (rand() % 10) * 0.1f - 0.5));
	}
	statue = new Statue(7.0f, 0.0f);
	for (int i = 0; i < 1000; i++) {
		float x = (rand() % 250) / 10 - 12.5f; float y = 8.0f + (rand() % 50) / 10; float z = (rand() % 250) / 10 - 12.5f;
		float s_y = -1.0f - (rand() % 10) / 10.0f; 
		float time = 10.0f + (rand() % 50) / 10.0f;
		float rotation = rand() % 360;
		particles.push_back(Particle(x, y, z, 0.0f, s_y, 0.0f, rotation, time));
	}
	present = new Present((rand() % 200) / 10 - 10.0f, (rand() % 200) / 10 - 10.0f);
	skybox = new Skybox();

	uInfo.snowballs = &snowballs;
	uInfo.houses = &houses;
	uInfo.trees = &trees;
	uInfo.lampposts = &lampposts;
	uInfo.statue = statue;
	uInfo.present = present;

	// create geometry and VAO of the rearViewWindow
	float amb[] = { 0.2f, 0.15f, 0.1f, 0.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 0.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 0.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	rearViewModel = createCube();
	memcpy(rearViewModel.mat.ambient, amb, 4 * sizeof(float));
	memcpy(rearViewModel.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(rearViewModel.mat.specular, spec, 4 * sizeof(float));
	memcpy(rearViewModel.mat.emissive, emissive, 4 * sizeof(float));
	rearViewModel.mat.shininess = 100.0f;
	rearViewModel.mat.texCount = 0;

	//objId = 2;  specular floor with quad
	mirror = createQuad(3.0f, 3.0f);
	memcpy(mirror.mat.ambient, amb, 4 * sizeof(float));
	memcpy(mirror.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mirror.mat.specular, spec, 4 * sizeof(float));
	memcpy(mirror.mat.emissive, emissive, 4 * sizeof(float));
	mirror.mat.shininess = 50.0f;
	mirror.mat.texCount = 0;

	cams.push_back(Camera(0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 0));
	cams.push_back(Camera(0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 1));
	cams.push_back(Camera(-sleigh->get_direction()[0] * 5.0f, -sleigh->get_direction()[1] * 5.0f + 2.0f, -sleigh->get_direction()[2] * 5.0f,
		sleigh->get_pos()[0], sleigh->get_pos()[1], sleigh->get_pos()[2],
		0, 1, 0, 0));

	//Load flare from file
	//loadFlareFile(&AVTflare, "flare.txt");

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearStencil(0x0);
	glEnable(GL_STENCIL_TEST);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_STENCIL|GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeysDown);
	glutKeyboardUpFunc(processKeysUp);

	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	glutTimerFunc(10000, updateGameSpeed, 0);

	//  GLUT main loop
	glutMainLoop();

	return(0);
}


