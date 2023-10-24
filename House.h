#pragma once
#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include <random>
#include "meshFromAssimp.h"
#include "assimp/scene.h"
#include <string>

using namespace std;

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];
std::string filepath1 = "./LogHutv1.1/LogHutv1.1.fbx";
std::string model_dirHouse = "./LogHutv1.1/";
extern float scaleFactor;

class House {
private:
	GLint normalMap_loc;
	GLint specularMap_loc;
	GLint diffMapCount_loc;
	const aiScene* sceneHouse;

	std::vector<struct MyMesh> myMeshes;

	float x, z;

	bool colided = false;
	const float timer = 0.5f;
	const float speed = 1.0f;
	float currentTime = 0.0f;
	float dir[2];
	float ScaleFactorHouse;
	MyMesh house;
	MyMesh rooftop;

	void createMesh() {
		/*float amb[4] = {0.15f, 0.1f, 0.06f, 1.0f};
		float diff[4] = { 0.2f, 0.15f, 0.09f, 1.0f };
		float spec[4] = { 0.03f, 0.02f, 0.01f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 100.0f;
		int texcount = 0;*/


		if (!Import3DFromFile(&(this->sceneHouse), filepath1))
			return;
		this->ScaleFactorHouse = 0.002;
		myMeshes = createMeshFromAssimp(this->sceneHouse, model_dirHouse);

		/*float house_amb[4] = {0.2f, 0.2f, 0.2f, 1.0f};
		float house_diff[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		float house_spec[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float house_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float house_shininess = 100.0f;
		int house_texcount = 0;

		house = createCube();
		memcpy(house.mat.ambient, house_amb, 4 * sizeof(float));
		memcpy(house.mat.diffuse, house_diff, 4 * sizeof(float));
		memcpy(house.mat.specular, house_spec, 4 * sizeof(float));
		memcpy(house.mat.emissive, house_emissive, 4 * sizeof(float));
		house.mat.shininess = house_shininess;
		house.mat.texCount = house_texcount;

		float rooftop_amb[4] = { 0.2f, 0.1f, 0.07f, 1.0f };
		float rooftop_diff[4] = { 0.4f, 0.2f, 0.15f, 1.0f };
		float rooftop_spec[4] = { 0.1f, 0.07f, 0.05f, 1.0f };
		float rooftop_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float rooftop_shininess = 100.0f;
		int rooftop_texcount = 0;

		rooftop = createCone(1.0f, 1.42f, 4);
		memcpy(rooftop.mat.ambient, rooftop_amb, 4 * sizeof(float));
		memcpy(rooftop.mat.diffuse, rooftop_diff, 4 * sizeof(float));
		memcpy(rooftop.mat.specular, rooftop_spec, 4 * sizeof(float));
		memcpy(rooftop.mat.emissive, rooftop_emissive, 4 * sizeof(float));
		rooftop.mat.shininess = rooftop_shininess;
		rooftop.mat.texCount = rooftop_texcount;*/
	}

public:
	float aabb_max[4];
	float aabb_min[4];

	House(float x, float z) {

		this->x = x; this->z = z;

		aabb_max[0] = this->x + 1.0f; aabb_max[1] = 2.25f; aabb_max[2] = this->z + 1.0f;
		aabb_min[0] = this->x - 1.0f; aabb_min[1] = 0.0f; aabb_min[2] = this->z - 1.0f;
		if(this->myMeshes.size() == 0)
			createMesh();
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

			glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
			glUniform1i(specularMap_loc, false);
			glUniform1ui(diffMapCount_loc, 0);

			if (myMeshes[nd->mMeshes[n]].mat.texCount != 0)
				for (unsigned int i = 0; i < myMeshes[nd->mMeshes[n]].mat.texCount; ++i) {
					if (myMeshes[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
						if (diffMapCount == 0) {
							diffMapCount++;
							loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texUnitDiff");
							glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i]);
							glUniform1ui(diffMapCount_loc, diffMapCount);
						}
						else if (diffMapCount == 1) {
							diffMapCount++;
							loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texUnitDiff1");
							glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i]);
							glUniform1ui(diffMapCount_loc, diffMapCount);
						}
						else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
					}
					else if (myMeshes[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
						loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texUnitSpec");
						glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i]);
						glUniform1i(specularMap_loc, true);
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
			this->aiRecursive_render(rInfo, sc, nd->mChildren[n]);
		}
		popMatrix(MODEL);
	}

	void render(struct render_info rInfo) {
		pushMatrix(MODEL);


		translate(MODEL, this->x, 0, this->z);
		scale(MODEL, this->ScaleFactorHouse, this->ScaleFactorHouse, this->ScaleFactorHouse);
		aiRecursive_render(rInfo, this->sceneHouse, this->sceneHouse->mRootNode);
		popMatrix(MODEL);
	}


	bool getColided() {
		return this->colided;
	}

	void setColided(float* new_dir) {
		this->dir[0] = new_dir[0];
		this->dir[1] = new_dir[2];
		float size = std::sqrt(new_dir[0] * new_dir[0] + new_dir[2] * new_dir[2]);
		this->dir[0] /= size;
		this->dir[1] /= size;
		this->colided = true;
		this->currentTime = 0.0f;
	}

	void updateHouse(float deltatime) {
		this->currentTime += deltatime;
		if (this->currentTime < this->timer) {
			this->x += this->dir[0] * this->speed * deltatime;
			this->aabb_max[0] += this->dir[0] * this->speed * deltatime;
			this->aabb_min[0] += this->dir[0] * this->speed * deltatime;

			this->z += this->dir[1] * this->speed * deltatime;
			this->aabb_max[2] += this->dir[1] * this->speed * deltatime;
			this->aabb_min[2] += this->dir[1] * this->speed * deltatime;
		}
		else {
			this->currentTime = 0.0f;
			this->colided = false;
		}

		if (std::fabs(this->x) >= 13) {
			this->x = 11 * (this->x < 0 ? -1 : 1);
		}
		if (std::fabs(this->z) >= 13) {
			this->z = 11 * (this->z < 0 ? -1 : 1);
		}
	}
};