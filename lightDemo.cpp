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
// Author: João Madeiras Pereira
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

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

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


using namespace std;

#define CAPTION "AVT Demo: Phong Shading and Text rendered with FreeType"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

#define FPS 40

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Vector with meshes
vector<struct MyMesh> myMeshes;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint tex_loc, tex_loc1, tex_loc2;

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

GLint textured_uniformId;
GLuint TextureArray[2];

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];
float dirLightPos[4] = {1.0f, -0.5f, 0.0f, 0.0f};

bool dirLightToggled = true;
bool pointLightToggled = true;
bool spotLightToggled = true;

float spotLightAngle = 20.0f;

vector<Camera> cams;
short active_camera = 0;

float ratio = WinX / WinY;

struct update_info uInfo = { 0.0f, 0.0f, 0.0f };

// Create objects
Terrain* terrain;
Sleigh* sleigh;
vector<Lamppost> lampposts;
vector<SnowBall> snowballs;
vector<House> houses;
vector<Tree> trees;
Statue* statue;


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
	sleigh->update(1.0f / FPS, &uInfo);
	if (!tracking)
		cams[2].update(sleigh->get_pos(), sleigh->get_direction());
	for (int i = 0; i < snowballs.size(); i++) {
		snowballs[i].updateSnowBallPosition(1.0f / FPS);
	}
	for (int i = 0; i < houses.size(); i++) {
		if (houses[i].getColided()) {
			houses[i].updateHouse(1.0/FPS);
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
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, refresh, 0);

}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	if(h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);
}


// ------------------------------------------------------------
//
// Render stufff
//

void renderScene(void) {

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

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

		for (int i = 0; i < 6; i++) {
			float *pos = lampposts[i].get_pointlight_pos();
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
		dir[3] = 1.0f;
		multMatrixPoint(VIEW, dir, res);
		glUniform4fv(spotLSpot_uniformId, 1, res);
		glUniform1f(spotLThreshold_uniformId, cos(spotLightAngle));
		glUniform1i(spotLToggled_uniformId, spotLightToggled);

		glUniform1i(fogToggled_uniformId, fogToggled);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureArray[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureArray[1]);
		glUniform1i(tex_loc, 0);
		glUniform1i(tex_loc1, 1);

	struct render_info rInfo = {shader, vm_uniformId, pvm_uniformId, normal_uniformId, textured_uniformId};

	terrain->render(rInfo);
	sleigh->render(rInfo);
	for (int i = 0; i < 6; i++) lampposts[i].render(rInfo);
	for (int i = 0; i < snowballs.size(); i++) snowballs[i].render(rInfo);
	for (int i = 0; i < houses.size(); i++) houses[i].render(rInfo);
	for (int i = 0; i < trees.size(); i++) trees[i].render(rInfo);
	statue->render(rInfo);

	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending
	glEnable(GL_BLEND);  
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch(key) {
		case 'w':
			uInfo.v_turning = -1;
			break;
		case 'a':
			uInfo.h_turning = 1;
			break;
		case 's':
			uInfo.v_turning = 1;
			break;
		case 'd':
			uInfo.h_turning = -1;
			break;
		case 'o':
			uInfo.accelerating = 1;
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


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN)  {
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

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

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
	cams[2].set_pos(1, rAux *   						       sin(betaAux * 3.14f / 180.0f) + sleigh->get_pos()[1]);
//  uncomment this if not using an idle or refresh func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	cams[2].set_pos(0, r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f));
	cams[2].set_pos(2, r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f));
	cams[2].set_pos(1, r *   						     sin(beta * 3.14f / 180.0f));

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
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
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
	textured_uniformId = glGetUniformLocation(shader.getProgramIndex(), "textured");

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
	spotLSpot_uniformId =glGetUniformLocation(shader.getProgramIndex(), "s_l_spot");
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
	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	//Texture Object definition

	glGenTextures(2, TextureArray);
	Texture2D_Loader(TextureArray, "texmap.jpg", 0);
	Texture2D_Loader(TextureArray, "texmap1.jpg", 1);

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	terrain = new Terrain(25.0f, 25.0f);
	sleigh = new Sleigh(0.0f, 0.0f, 0.0f, 90.0f);
	for (int i = 0; i < 360; i += 360 / 12) {
		snowballs.push_back(SnowBall(0.5f, i, 7.0f));
	}
	for (int i = 0; i < 6; i++) {
		lampposts.push_back(Lamppost(5.0f * ((i % 3) - 1), 2.5f * ((i / 3) * 2 - 1)));
	}
	for (int i = 0; i < 8; i++) {
		houses.push_back(House(5.0f * ((i % 4) - 1) - 2.5f, 4.0f * ((i / 4) * 2 - 1)));
	}
	for (int i = 0; i < 20; i++) {
		float size = rand() % 15 * 0.01f + 0.05f;
		trees.push_back(Tree(size, size * (2.0f + rand() % 10 * 0.2f), rand() % 24 - 11.5f + (rand() % 10) * 0.1f - 0.5, rand() % 6 - 11.5f + (rand() % 10) * 0.1f - 0.5));
		size = rand() % 15 * 0.01f + 0.05f;
		trees.push_back(Tree(size, size * (2.0f + rand() % 10 * 0.2f), rand() % 24 - 11.5f + (rand() % 10) * 0.1f - 0.5, rand() % 6 + 6.5f + (rand() % 10) * 0.1f - 0.5));
	}
	statue = new Statue(7.0f, 0.0f);

	uInfo.snowballs = &snowballs;
	uInfo.houses = &houses;
	uInfo.trees = &trees;
	uInfo.lampposts = &lampposts;
	uInfo.statue = statue;

	cams.push_back(Camera(0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 0));
	cams.push_back(Camera(0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 1));
	cams.push_back(Camera(-sleigh->get_direction()[0] * 5.0f, -sleigh->get_direction()[1] * 5.0f + 2.0f, -sleigh->get_direction()[2] * 5.0f, 
		sleigh->get_pos()[0], sleigh->get_pos()[1], sleigh->get_pos()[2], 
		0, 1, 0, 0));

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char **argv) {

//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE);

	glutInitContextVersion (4, 3);
	glutInitContextProfile (GLUT_CORE_PROFILE );
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100,100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc ( mouseWheel ) ;
	

//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	glutTimerFunc(10000, updateGameSpeed, 0);

	//  GLUT main loop
	glutMainLoop();

	return(0);
}



