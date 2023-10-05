#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class Lamppost {
private:
	float x;
	float z;
	MyMesh lamp;
	MyMesh post;

	void createMesh() {
		float lamp_amb[4] = { 0.9f, 0.9f, 0.75f, 1.0f };
		float lamp_diff[4] = { 0.8f, 0.8f, 0.7f, 1.0f };
		float lamp_spec[4] = { 0.8f, 0.8f, 0.7f, 1.0f };
		float lamp_emissive[4] = { 0.9f, 0.9f, 0.75f, 1.0f };
		float lamp_shininess = 100.0f;
		int lamp_texcount = 0;

		lamp = createSphere(0.1, 8);
		memcpy(lamp.mat.ambient, lamp_amb, 4 * sizeof(float));
		memcpy(lamp.mat.diffuse, lamp_diff, 4 * sizeof(float));
		memcpy(lamp.mat.specular, lamp_spec, 4 * sizeof(float));
		memcpy(lamp.mat.emissive, lamp_emissive, 4 * sizeof(float));
		lamp.mat.shininess = lamp_shininess;
		lamp.mat.texCount = lamp_texcount;

		float post_amb[4] = { 0.05f, 0.1f, 0.075f, 1.0f };
		float post_diff[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
		float post_spec[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
		float post_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float post_shininess = 100.0f;
		int post_texcount = 0;

		post = createCylinder(2.0f, 0.03, 8);
		memcpy(post.mat.ambient, post_amb, 4 * sizeof(float));
		memcpy(post.mat.diffuse, post_diff, 4 * sizeof(float));
		memcpy(post.mat.specular, post_spec, 4 * sizeof(float));
		memcpy(post.mat.emissive, post_emissive, 4 * sizeof(float));
		post.mat.shininess = post_shininess;
		post.mat.texCount = post_texcount;
	}

public:
	Lamppost(float x, float z) {
		this->x = x; this->z = z;

		createMesh();
	}

	float *get_pointlight_pos() {
		float *pos = new float[4];
		pos[0] = x, pos[1] = 2.5f, pos[2] = z, pos[3] = 1.0f;
		return pos;
	}

	void render(struct render_info rInfo) {
		GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, lamp.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, lamp.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, lamp.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, lamp.mat.shininess);

		glUniform1i(rInfo.textured_uniformId, false);

		pushMatrix(MODEL);
		translate(MODEL, x, 2.0f, z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(lamp.vao);

		glDrawElements(lamp.type, lamp.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, post.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, post.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, post.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, post.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, x, 1.0f, z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(post.vao);

		glDrawElements(post.type, post.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
};