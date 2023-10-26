#pragma once
#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class Particle {
private:
	float start[4];
	float pos[4];
	float speed[4];
	float rotation;
	float time;
	float time_passed = 0.0f;
	MyMesh mesh;

	void createMesh() {
		float amb[4] = { 0.2f, 0.2f, 0.21f, 1.0f };
		float diff[4] = { 0.4f, 0.4f, 0.42f, 1.0f };
		float spec[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 30.0f;
		int texcount = 0;

		mesh = createQuad(0.15f, 0.15f);
		memcpy(mesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh.mat.emissive, emissive, 4 * sizeof(float));
		mesh.mat.shininess = shininess;
		mesh.mat.texCount = texcount;
	}

public:
	Particle(float x, float y, float z, float s_x, float s_y, float s_z, float rotation, float time) {
		pos[0] = start[0] = x, pos[1] = start[1] = y, pos[2] = start[2] = z, pos[3] = start[3] = 1;
		speed[0] = s_x, speed[1] = s_y, speed[2] = s_z; speed[3] = 0;
		this->rotation = rotation;
		this->time = time;

		createMesh();
	}

	void update(float deltatime) {
		pos[0] += speed[0] * deltatime;
		pos[1] += speed[1] * deltatime;
		pos[2] += speed[2] * deltatime;

		if (time_passed < this->time && pos[1] > 0) {
			time_passed += deltatime;
		}
		else if (time_passed >= this->time) {
			time_passed = 0.0f;
			for (int i = 0; i < 4; i++) pos[i] = start[i];
		} else if (pos[1] <= 0)
			for (int i = 0; i < 4; i++) pos[i] = start[i];
	}

	void render(struct render_info rInfo) {
		GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh.mat.shininess);

		glUniform1i(rInfo.textMode_uniformId, 2);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texmap");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rInfo.TextureArray[2]);
		glUniform1i(loc, 0);

		pushMatrix(MODEL);
		rotate(MODEL, rotation, 0.0f, 1.0f, 0.0f);
		translate(MODEL, pos[0], pos[1], pos[2]);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(mesh.vao);

		glDrawElements(mesh.type, mesh.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
};