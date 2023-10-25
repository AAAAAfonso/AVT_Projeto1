#include <geometry.h>
#include <update_info.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include "meshFromAssimp.h"
#include "assimp/scene.h"
#include <string>

using namespace std;

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];
std::string filepath = "./slitta_babbo/SLITTABABBO.obj";
std::string dirpathSleigh = "./slitta_babbo/";

extern float scaleFactor;



class Sleigh {
private:
	const aiScene* sceneSleigh = NULL;

	GLint normalMap_loc2;
	GLint specularMap_loc2;
	GLint diffMapCount_loc2;
	std::vector<struct MyMesh> myMeshes;

	float pos[4];
	float dir[4];
	float hAngle;
	float scaleFactorSleigh;
	float vAngle = 0.0f;
	float speed = 0.0f;
	int lives = 5;
	int points = 0;

	void createMesh() {
	
			
		if (!Import3DFromFile( &(this->sceneSleigh), filepath))
			return;
		this->scaleFactorSleigh = scaleFactor;
	 	myMeshes = createMeshFromAssimp(this->sceneSleigh, dirpathSleigh);

	}

public:
	float aabb_max[4];
	float aabb_min[4];

	Sleigh(float x, float y, float z, float hAngle) {
		pos[0] = x; pos[1] = y; pos[2] = z; pos[3] = 0;
		this->hAngle = hAngle;

		createMesh();

		dir[0] = cos(vAngle * 3.14f / 180) * sin(hAngle * 3.14f / 180);
		dir[1] = -sin(vAngle * 3.14f / 180);
		dir[2] = cos(vAngle * 3.14f / 180) * cos(hAngle * 3.14f / 180);

		
		updateAABB();
	}

	float *get_direction() {
		return dir;
	}

	void decrease_lives() {
		lives--;
		if (lives == 0) reset();
	}

	void reset() {
		points = 0;
		lives = 5;
	}

	int get_lives() {
		return lives;
	}

	int get_points() {
		return points;
	}

	bool detectColisionAABBbox(struct update_info* uInfo) {
		bool col_detected = false;

		if ((*(uInfo->statue)).aabb_min[0] <= this->aabb_max[0] &&
			(*(uInfo->statue)).aabb_max[0] >= this->aabb_min[0] &&
			(*(uInfo->statue)).aabb_min[1] <= this->aabb_max[1] &&
			(*(uInfo->statue)).aabb_max[1] >= this->aabb_min[1] &&
			(*(uInfo->statue)).aabb_min[2] <= this->aabb_max[2] &&
			(*(uInfo->statue)).aabb_max[2] >= this->aabb_min[2]) {
			(*(uInfo->statue)).setColided(dir);
			col_detected = true;
		}

		for (unsigned int i = 0; i < uInfo->houses->size(); i++) {
			if ((*(uInfo->houses))[i].aabb_min[0] <= this->aabb_max[0] &&
				(*(uInfo->houses))[i].aabb_max[0] >= this->aabb_min[0] &&
				(*(uInfo->houses))[i].aabb_min[1] <= this->aabb_max[1] &&
				(*(uInfo->houses))[i].aabb_max[1] >= this->aabb_min[1] &&
		 		(*(uInfo->houses))[i].aabb_min[2] <= this->aabb_max[2] &&
				(*(uInfo->houses))[i].aabb_max[2] >= this->aabb_min[2]) {
				(*(uInfo->houses))[i].setColided(dir);
				col_detected = true;
			}
		}
		
		for (unsigned int i = 0; i < uInfo->trees->size(); i++) {
			if ((*(uInfo->trees))[i].aabb_min[0] <= this->aabb_max[0] &&
				(*(uInfo->trees))[i].aabb_max[0] >= this->aabb_min[0] &&
				(*(uInfo->trees))[i].aabb_min[1] <= this->aabb_max[1] &&
				(*(uInfo->trees))[i].aabb_max[1] >= this->aabb_min[1] &&
				(*(uInfo->trees))[i].aabb_min[2] <= this->aabb_max[2] &&
				(*(uInfo->trees))[i].aabb_max[2] >= this->aabb_min[2]) {
				(*(uInfo->trees))[i].setColided(dir);
				col_detected = true;
			}
		}
		for (unsigned int i = 0; i < uInfo->lampposts->size(); i++) {
			if ((*(uInfo->lampposts))[i].aabb_min[0] <= this->aabb_max[0] &&
				(*(uInfo->lampposts))[i].aabb_max[0] >= this->aabb_min[0] &&
				(*(uInfo->lampposts))[i].aabb_min[1] <= this->aabb_max[1] &&
				(*(uInfo->lampposts))[i].aabb_max[1] >= this->aabb_min[1] &&
				(*(uInfo->lampposts))[i].aabb_min[2] <= this->aabb_max[2] &&
				(*(uInfo->lampposts))[i].aabb_max[2] >= this->aabb_min[2]) {
				(*(uInfo->lampposts))[i].setColided(dir);
				col_detected = true;
			}
		}
		if (uInfo->present->aabb_min[0] <= this->aabb_max[0] &&
			uInfo->present->aabb_max[0] >= this->aabb_min[0] &&
			uInfo->present->aabb_min[1] <= this->aabb_max[1] &&
			uInfo->present->aabb_max[1] >= this->aabb_min[1] &&
			uInfo->present->aabb_min[2] <= this->aabb_max[2] &&
			uInfo->present->aabb_max[2] >= this->aabb_min[2]) {
			uInfo->present->setColided();
			points++;
		}

		return col_detected;
	}

	void updateAABB() {

		//float angle_0 = 0.5 * cos(0);
		//float angle_x = 0.5 * cos(this->vAngle * 3.14f / 180);

		//float small_dif = (angle_0 - angle_x) *cos(this->hAngle * 3.14f / 180);

		//TO DO
		this->aabb_max[0] = this->pos[0] + (0.3f * std::fabs(cos(this->hAngle * 3.14f / 180)) + 0.5f * std::fabs(sin(this->hAngle * 3.14f / 180))) * cos(this->vAngle * 3.14f / 180)
										 + std::fmax(0.6 * sin(this->vAngle * 3.14 / 180) * sin(this->hAngle * 3.14 / 180), 0);
		
		this->aabb_max[1] = this->pos[1] + 0.6 * std::fabs(cos(this->vAngle * 3.14f / 180)) + 0.5f * std::fabs(sin(this->vAngle * 3.14f / 180));
		this->aabb_max[2] = this->pos[2] + (0.3f * std::fabs(sin(this->hAngle * 3.14f / 180)) + 0.5f*std::fabs(cos(this->hAngle * 3.14f / 180))) * cos(this->vAngle * 3.14f / 180) +
							std::fmax(0.6*sin(this->vAngle * 3.14 /180)*cos(this->hAngle*3.14 / 180), 0);

		this->aabb_min[0] = this->pos[0] - (0.3f * std::fabs(cos(this->hAngle * 3.14f / 180)) + 0.5f * std::fabs(sin(this->hAngle * 3.14f / 180))) * cos(this->vAngle * 3.14f / 180)
										+ std::fmin(0.6 * sin(this->vAngle * 3.14 / 180) * sin(this->hAngle * 3.14 / 180), 0);
		;
		this->aabb_min[1] = this->pos[1] - 0.5f * std::fabs(sin(this->vAngle));
		this->aabb_min[2] = this->pos[2] - (0.3*std::fabs(sin(this->hAngle * 3.14f / 180)) + 0.5*std::fabs(cos(this->hAngle * 3.14f / 180))) * cos(this->vAngle * 3.14f / 180)
							+ std::fmin(0.6 * sin(this->vAngle * 3.14 / 180) * cos(this->hAngle * 3.14 / 180), 0);
		;

	}

	void missionFail() {
		this->pos[0] = 0;
		this->pos[1] = 0;
		this->pos[2] = 0;
		this->speed = 0.0f;
		dir[0] = cos(vAngle * 3.14f / 180) * sin(hAngle * 3.14f / 180);
		dir[1] = -sin(vAngle * 3.14f / 180);
		dir[2] = cos(vAngle * 3.14f / 180) * cos(hAngle * 3.14f / 180);
		this->hAngle = 0.0;
		this->vAngle = 0.0;
		decrease_lives();
		updateAABB();

	}
	bool detectColisionSphere(struct update_info* uInfo) {
		for (int i = 0; i < uInfo->snowballs->size(); i++) {
			if (!(*(uInfo->snowballs))[i].isAlive())
				continue;

			float pos[3];
			pos[0] = (*(uInfo->snowballs))[i].getPosition()[0];
			pos[1] = (*(uInfo->snowballs))[i].getPosition()[1];
			pos[2] = (*(uInfo->snowballs))[i].getPosition()[2];
			float x = std::max(this->aabb_min[0], std::min(pos[0], this->aabb_max[0]));
			float y = std::max(this->aabb_min[1], std::min(pos[1], this->aabb_max[1]));
			float z = std::max(this->aabb_min[2], std::min(pos[2], this->aabb_max[2]));

			float distance = std::sqrt(
				(x - pos[0]) * (x - pos[0]) +
				(y - pos[1]) * (y - pos[1]) +
				(z - pos[2]) * (z - pos[2]));

			if (distance < (*(uInfo->snowballs))[i].getRadius() ) {
				missionFail();
				(*(uInfo->snowballs))[i].KillBall();
				return true;
			}
		}
		return false;

	}

	float *get_pos() {
		return pos;
	}

	float* get_spotlight_pos(int n) {
		float* pos = new float[4];

		pos[0] = this->pos[0] + 0.5 * sin(hAngle * 3.14f / 180 - (n * 2 - 1) * 0.4) * cos(vAngle * 3.14f / 180);
		pos[1] = 0.2f + this->pos[1] + 0.5 * sin(vAngle * 3.14f / 180);
		pos[2] = this->pos[2] + 0.5 * cos(hAngle * 3.14f / 180 - (n * 2 - 1) * 0.4) * cos(vAngle * 3.14f / 180);
		pos[3] = 1.0f;
		
		return pos;
	}

	void update(float deltaTime, struct update_info *uInfo) {
		static float hAngle_prev = 0.0f; static float vAngle_prev = 0.0f;
		hAngle += uInfo->h_turning * 180.0f * deltaTime;
		vAngle += uInfo->v_turning * 180.0f * deltaTime;
		if (vAngle < -30.0f) vAngle = -30.0f;
		else if (vAngle > 30.0f) vAngle = 30.0f;


		speed += uInfo->accelerating * 10.0f * deltaTime;
		if (speed < 0) speed = 0;
		else if (speed > 10.0f) speed = 10.0f;


		updateAABB();
		if(detectColisionSphere(uInfo)) return;
		if (!detectColisionAABBbox(uInfo)) {
			dir[0] = cos(vAngle * 3.14f / 180) * sin(hAngle * 3.14f / 180);
			dir[1] = -sin(vAngle * 3.14f / 180);
			dir[2] = cos(vAngle * 3.14f / 180) * cos(hAngle * 3.14f / 180);
			for (int i = 0; i < 3; i++) pos[i] += speed * dir[i] * deltaTime;
			vAngle_prev = vAngle;
			hAngle_prev = hAngle;
		}
		else {
			uInfo->accelerating = -1.0;	
			hAngle = hAngle_prev;
			vAngle = vAngle_prev;
			speed = 0;
		}

		if (pos[1] < 0) pos[1] = 0.0f; 
		if (pos[1] > 10.0f) pos[1] = 10.f;
		if (std::fabs(pos[0]) > 11) pos[0] = 11.0f*(pos[0] < 0? -1 : 1);
		if (std::fabs(pos[2]) > 11) pos[2] = 11.0f*(pos[2] < 0 ? -1 : 1);

	}

	void aiRecursive_render(struct render_info rInfo, const aiScene* sc, const aiNode* nd)
	{
		GLint loc;

		// Get node transformation matrix
		aiMatrix4x4 m = nd->mTransformation;
		// OpenGL matrices are column major
		m.Transpose();

		// save model matrix and apply node transformation
		pushMatrix(MODEL);

		float aux[16];
		memcpy(aux, &m, sizeof(float) * 16);
		multMatrix(MODEL, aux);


		// draw all meshes assigned to this node
		for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {


			// send the material
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.ambient);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.diffuse);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.specular);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.emissive");
			glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.emissive);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, myMeshes[nd->mMeshes[n]].mat.shininess);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.texCount");
			glUniform1i(loc, myMeshes[nd->mMeshes[n]].mat.texCount);

			unsigned int  diffMapCount = 0;  //read 2 diffuse textures

			//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

			glUniform1i(normalMap_loc2, false);   //GLSL normalMap variable initialized to 0
			glUniform1i(specularMap_loc2, false);
			glUniform1ui(diffMapCount_loc2, 0);

			if (myMeshes[nd->mMeshes[n]].mat.texCount != 0)
				for (unsigned int i = 0; i < myMeshes[nd->mMeshes[n]].mat.texCount; ++i) {
					if (myMeshes[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
						if (diffMapCount == 0) {
							diffMapCount++;
							loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texUnitDiff");
							glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i]);
							glUniform1ui(diffMapCount_loc2, diffMapCount);
						}
						else if (diffMapCount == 1) {
							diffMapCount++;
							loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texUnitDiff1");
							glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i]);
							glUniform1ui(diffMapCount_loc2, diffMapCount);
						}
						else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
					}
					else if (myMeshes[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
						loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texUnitSpec");
						glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i]);
						glUniform1i(specularMap_loc2, true);
					}
					else if (myMeshes[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
						loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texUnitNormalMap");
						/*if (normalMapKey)
							glUniform1i(normalMap_loc, normalMapKey);*/
						glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i]);

					}
					else printf("Texture Map not supported\n");
				}

			// send matrices to OGL
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// bind VAO
			glBindVertexArray(myMeshes[nd->mMeshes[n]].vao);

			if (!rInfo.shader.isProgramValid()) {
				printf("Program Not Valid!\n");
				exit(1);
			}
			// draw
			glDrawElements(myMeshes[nd->mMeshes[n]].type, myMeshes[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		// draw all children
		for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
			this->aiRecursive_render(rInfo,sc, nd->mChildren[n]);
		}
		popMatrix(MODEL);
	}

	void render(struct render_info rInfo) {
		pushMatrix(MODEL);
		

		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle - 90, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 0.0, 0.0f, -1.0f);
		scale(MODEL, this->scaleFactorSleigh, this->scaleFactorSleigh, this->scaleFactorSleigh);
		aiRecursive_render(rInfo, this->sceneSleigh, this->sceneSleigh->mRootNode);
		popMatrix(MODEL);
	}
};