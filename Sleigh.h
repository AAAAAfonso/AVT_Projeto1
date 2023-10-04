#include <geometry.h>
#include <update_info.h>
#include <render_info.h>
#include <AVTmathLib.h>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class Sleigh {
private:
	float pos[4];
	float dir[4];
	float hAngle;
	float vAngle = 0.0f;
	float speed = 0.0f;
	MyMesh mesh;

	MyMesh createMesh() { return createCube(); }

public:
	Sleigh(float x, float y, float z, float hAngle) {
		pos[0] = x; pos[1] = y; pos[2] = z; pos[3] = 0;
		this->hAngle = hAngle;

		float amb[4] = { 0.2f, 0.15f, 0.1f, 1.0f };
		float diff[4] = { 0.8f, 0.6f, 0.4f, 1.0f };
		float spec[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 100.0f;
		int texcount = 0;

		mesh = createMesh();
		memcpy(mesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh.mat.emissive, emissive, 4 * sizeof(float));
		mesh.mat.shininess = shininess;
		mesh.mat.texCount = texcount;
	}

	float *get_direction() {
		return dir;
	}

	struct update_info update(float deltaTime, struct update_info uInfo) {
		hAngle += uInfo.h_turning * 180.0f * deltaTime;
		uInfo.h_turning = 0.0f;

		dir[0] = cos(vAngle * 3.14f / 180) * sin(hAngle * 3.14f / 180);
		dir[1] = sin(vAngle * 3.14f / 180);
		dir[2] = cos(vAngle * 3.14f / 180) * cos(hAngle * 3.14f / 180);

		speed += uInfo.accelerating * 5.0f * deltaTime;
		uInfo.accelerating = -1.0f;
		if (speed < 0) speed = 0;
		else if (speed > 8.0f) speed = 8.0f;
		for (int i  = 0; i < 3; i++) pos[i] += speed * dir[i] * deltaTime;
		printf("%f %f %f %f\n", speed, pos[0], pos[1], pos[2]);

		return uInfo;
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

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

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