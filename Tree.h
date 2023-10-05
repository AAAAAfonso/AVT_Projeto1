#pragma once

#pragma once
#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include <random>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class Tree{
private:
	float x, z, radius, height;


	bool colided = false;
	const float timer = 1.5f;
	const float speed = 2.0f;
	float currentTime = 0.0f;
	float dir[2];

	MyMesh trunk;
	MyMesh leafs;


	MyMesh createTrunk() { return createCylinder(this->height, this->radius, 20); }
	MyMesh createLeafs() { return createCone(this->height*1.2f, this->radius*2.5f, 20); }


public:
	float aabb_max[4];
	float aabb_min[4];

	Tree(float radius, float height, float x, float z) {

		this->x = x; this->z = z; this->radius = radius; this->height = height;
		float amb[4] = { 0.2f, 0.15f, 0.1f, 1.0f };
		float diff[4] = { 0.8f, 0.6f, 0.4f, 1.0f };
		float spec[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 100.0f;
		int texcount = 0;

		aabb_max[0] = this->x + this->radius * 2.5f; 
		aabb_max[1] = this->height + this->height * 1.2f;
		aabb_max[2] = this->z + this->radius * 2.5f;

		aabb_min[0] = this->x - this->radius * 2.5f;
		aabb_min[1] = 0.0f;
		aabb_min[2] = this->z - this->radius * 2.5f;


		trunk = createTrunk();
		leafs = createLeafs();
		memcpy(trunk.mat.ambient, amb, 4 * sizeof(float));
		memcpy(trunk.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(trunk.mat.specular, spec, 4 * sizeof(float));
		memcpy(trunk.mat.emissive, emissive, 4 * sizeof(float));
		trunk.mat.shininess = shininess;
		trunk.mat.texCount = texcount;

		memcpy(leafs.mat.ambient, amb, 4 * sizeof(float));
		memcpy(leafs.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(leafs.mat.specular, spec, 4 * sizeof(float));
		memcpy(leafs.mat.emissive, emissive, 4 * sizeof(float));
		leafs.mat.shininess = shininess;
		leafs.mat.texCount = texcount;
	}

	void render(struct render_info rInfo) {
		GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, trunk.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, trunk.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, trunk.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, trunk.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, this->x, this->height/2, this->z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(trunk.vao);

		glDrawElements(trunk.type, trunk.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, leafs.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, leafs.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, leafs.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, leafs.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, this->x, this->height, this->z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(leafs.vao);

		glDrawElements(leafs.type, leafs.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

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