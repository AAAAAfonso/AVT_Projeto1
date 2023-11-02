#pragma once
#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include <random>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class Statue {
private:
	float x, z;

	bool colided = false;
	const float timer = 1.0f;
	const float speed = 0.2f;
	float currentTime = 0.0f;
	float dir[2];

	MyMesh head;
	MyMesh body;

	void createMesh() {
		float ice_amb[4] = { 0.2f, 0.2f, 0.3f, 0.1f };
		float ice_diff[4] = { 0.2f, 0.2f, 0.3f, 0.2f };
		float ice_spec[4] = { 0.8f, 0.8f, 1.0f, 0.1f };
		float ice_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float ice_shininess = 100.0f;
		int ice_texcount = 0;

		head = createSphere(0.25, 8);
		memcpy(head.mat.ambient, ice_amb, 4 * sizeof(float));
		memcpy(head.mat.diffuse, ice_diff, 4 * sizeof(float));
		memcpy(head.mat.specular, ice_spec, 4 * sizeof(float));
		memcpy(head.mat.emissive, ice_emissive, 4 * sizeof(float));
		head.mat.shininess = ice_shininess;
		head.mat.texCount = ice_texcount;

		body = createSphere(0.5, 16);
		memcpy(body.mat.ambient, ice_amb, 4 * sizeof(float));
		memcpy(body.mat.diffuse, ice_diff, 4 * sizeof(float));
		memcpy(body.mat.specular, ice_spec, 4 * sizeof(float));
		memcpy(body.mat.emissive, ice_emissive, 4 * sizeof(float));
		body.mat.shininess = ice_shininess;
		body.mat.texCount = ice_texcount;
	}

public:
	float aabb_max[4];
	float aabb_min[4];

	Statue(float x, float z) {

		this->x = x; this->z = z;

		aabb_max[0] = this->x + 0.5f; aabb_max[1] = 1.4f; aabb_max[2] = this->z + 0.5f;
		aabb_min[0] = this->x - 0.5f; aabb_min[1] = 0.0f; aabb_min[2] = this->z - 0.5f;

		createMesh();
	}

	void render(struct render_info rInfo) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, head.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, head.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, head.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, head.mat.shininess);

		glUniform1i(rInfo.textMode_uniformId, 6);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texmap");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rInfo.TextureArray[6]);
		glUniform1i(loc, 0);

		pushMatrix(MODEL);
		translate(MODEL, this->x, 1.15f, this->z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(head.vao);

		glDrawElements(head.type, head.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, body.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, body.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, body.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, body.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, this->x, 0.5f, this->z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(body.vao);

		glDrawElements(body.type, body.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		glDisable(GL_BLEND);
	}

	void render_reflected(struct render_info rInfo) {
		glFrontFace(GL_CW);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, head.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, head.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, head.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, head.mat.shininess);

		glUniform1i(rInfo.textMode_uniformId, 6);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texmap");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rInfo.TextureArray[6]);
		glUniform1i(loc, 0);

		pushMatrix(MODEL);
		translate(MODEL, -7.0f, 0.0f, 0.0f);
		scale(MODEL, -1.0f, 1.0f, 1.0f);
		translate(MODEL, 7.0f, 0.0f, 0.0f);
		translate(MODEL, this->x, 1.15f, this->z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(head.vao);

		glDrawElements(head.type, head.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, body.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, body.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, body.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, body.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, -7.0f, 0.0f, 0.0f);
		scale(MODEL, -1.0f, 1.0f, 1.0f);
		translate(MODEL, 7.0f, 0.0f, 0.0f);
		translate(MODEL, this->x, 0.5f, this->z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(body.vao);

		glDrawElements(body.type, body.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		glDisable(GL_BLEND);

		glFrontFace(GL_CCW);
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

	void updateStatue(float deltatime) {
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