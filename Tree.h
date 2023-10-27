#pragma once

#pragma once
#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include <random>
#include "I3DBillboard.h"

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class Tree{
private:
	float x, z, radius, height;


	bool colided = false;
	const float timer = 0.5f;
	const float speed = 1.0;
	float currentTime = 0.0f;
	float dir[2];

	MyMesh trunk;
	MyMesh leafs;

	MyMesh TreeBillboard;

	void createMesh(float height) {
		/*float trunk_amb[4] = {0.15f, 0.1f, 0.06f, 1.0f};
		float trunk_diff[4] = { 0.2f, 0.15f, 0.09f, 1.0f };
		float trunk_spec[4] = { 0.03f, 0.02f, 0.01f, 1.0f };
		float trunk_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float trunk_shininess = 100.0f;
		int trunk_texcount = 0;

		trunk = createCylinder(this->height, this->radius, 20);
		memcpy(trunk.mat.ambient, trunk_amb, 4 * sizeof(float));
		memcpy(trunk.mat.diffuse, trunk_diff, 4 * sizeof(float));
		memcpy(trunk.mat.specular, trunk_spec, 4 * sizeof(float));
		memcpy(trunk.mat.emissive, trunk_emissive, 4 * sizeof(float));
		trunk.mat.shininess = trunk_shininess;
		trunk.mat.texCount = trunk_texcount;

		float leafs_amb[4] = { 0.15f, 0.25f, 0.15f, 1.0f };
		float leafs_diff[4] = { 0.15f, 0.3f, 0.15f, 1.0f };
		float leafs_spec[4] = { 0.02f, 0.05f, 0.01f, 1.0f };
		float leafs_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float leafs_shininess = 100.0f;
		int leafs_texcount = 0;

		leafs = createCone(this->height * 3.0f, this->radius * 3.0f, 20);
		memcpy(leafs.mat.ambient, leafs_amb, 4 * sizeof(float));
		memcpy(leafs.mat.diffuse, leafs_diff, 4 * sizeof(float));
		memcpy(leafs.mat.specular, leafs_spec, 4 * sizeof(float));
		memcpy(leafs.mat.emissive, leafs_emissive, 4 * sizeof(float));
		leafs.mat.shininess = leafs_shininess;
		leafs.mat.texCount = leafs_texcount;*/
		this->TreeBillboard = createQuad(height, height);

	}


public:
	float aabb_max[4];
	float aabb_min[4];

	Tree(float radius, float height, float x, float z) {

		this->x = x; this->z = z; this->radius = radius; this->height = height;

		aabb_max[0] = this->x + this->radius * 2.5f; 
		aabb_max[1] = this->height + this->height * 3.0f;
		aabb_max[2] = this->z + this->radius * 3.0f;

		aabb_min[0] = this->x - this->radius * 3.0f;
		aabb_min[1] = 0.0f;
		aabb_min[2] = this->z - this->radius * 3.0f;

		createMesh(this->height + this->height * 3.0f);
	}

	void render(struct render_info rInfo, int type) {
		float pos[3];
		GLint loc;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUniform1i(rInfo.textMode_uniformId, 8); // draw textured quads
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texmap");
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, rInfo.TextureArray[7]);
		glUniform1i(loc, 6);

		pushMatrix(MODEL);
		translate(MODEL, this->x, 0, this->z);

		pos[0] = this->x; pos[1] = 0; pos[2] = this->z;

		if (type == 2)
			l3dBillboardSphericalBegin(rInfo.cam_pos, pos);
		else if (type == 3)
			l3dBillboardCylindricalBegin(rInfo.cam_pos, pos);
				//diffuse and ambient color are not used in the tree quads
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, this->TreeBillboard.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, this->TreeBillboard.mat.shininess);

		pushMatrix(MODEL);
	    translate(MODEL, 0.0, (this->height + this->height * 3.0f)/2, 0.0f);

				// send matrices to OGL
		if (type == 0 || type == 1) {     //Cheating matrix reset billboard techniques
			computeDerivedMatrix(VIEW_MODEL);
				//reset VIEW_MODEL
			if (type == 0) BillboardCheatSphericalBegin();
			else BillboardCheatCylindricalBegin();
			computeDerivedMatrix_PVM(); // calculate PROJ_VIEW_MODEL
		}
		else computeDerivedMatrix(PROJ_VIEW_MODEL);

		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);
		glBindVertexArray(this->TreeBillboard.vao);
		glDrawElements(this->TreeBillboard.type, this->TreeBillboard.numIndexes, GL_UNSIGNED_INT, 0);
		popMatrix(MODEL);

			//	if (type==0 || type==1) // restore matrix VIEW_MODEL não é necessário pois a PVM é sempre calculada a pArtir da MODEL e da VIEW que não são ALTERADAS

		popMatrix(MODEL);
	
	}

	bool getColided() {
		return this->colided;
	}

	void setColided(float *new_dir) {
		this->dir[0] = new_dir[0];
		this->dir[1] = new_dir[2];
		float size = std::sqrt(new_dir[0] * new_dir[0] + new_dir[2] * new_dir[2]);
		this->dir[0] /= size;
		this->dir[1] /= size;
		this->colided = true;
		this->currentTime = 0.0f;

	}

	void updateTree(float deltatime) {
		this->currentTime += deltatime;
		if (this->currentTime < this->timer) {

			this->x += this->dir[0]*this->speed * deltatime;
			this->aabb_max[0] += this->dir[0] * this->speed * deltatime;
			this->aabb_min[0] += this->dir[0] * this->speed * deltatime;

			this->z += this->dir[1]*this->speed * deltatime;
			this->aabb_max[2] += this->dir[1] * this->speed * deltatime;
			this->aabb_min[2] += this->dir[1] * this->speed * deltatime;

		}
		else {
			this->currentTime = 0.0f;
			this->colided = false;
		}

		if (std::fabs(this->x) >= 13) {
			this->x = 12.5 * (this->x < 0 ? -1 : 1);
		}
		if (std::fabs(this->z) >= 13) {
			this->z = 11 * (this->z < 0 ? -1 : 1);
		}
	}
};