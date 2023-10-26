#pragma once
#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class Skybox {
private:
	float x, z;

	bool colided = false;

	MyMesh mesh;

	void createMesh() {
		float amb[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
		float diff[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		float spec[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 100.0f;
		int texcount = 0;

		mesh = createCube();
		memcpy(mesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh.mat.emissive, emissive, 4 * sizeof(float));
		mesh.mat.shininess = shininess;
		mesh.mat.texCount = texcount;
	}

public:

	Skybox() {
		createMesh();
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

		glUniform1i(rInfo.textMode_uniformId, 5);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texmap");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rInfo.TextureArray[5]);
		glUniform1i(loc, 0);

		glDepthMask(GL_FALSE);
		glFrontFace(GL_CW);

		pushMatrix(MODEL);
		translate(MODEL, -100.0f, -100.0f, -100.0f);
		scale(MODEL, 200.0f, 200.0f, 200.0f);

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

		glFrontFace(GL_CCW); // restore counter clockwise vertex order to mean the front
		glDepthMask(GL_TRUE);
	}


	bool getColided() {
		return this->colided;
	}

	void setColided() {
		this->colided = true;
	}
};
