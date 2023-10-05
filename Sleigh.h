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
	float aabb_max[4];
	float aabb_min[4];

	Sleigh(float x, float y, float z, float hAngle) {
		pos[0] = x; pos[1] = y; pos[2] = z; pos[3] = 0;
		this->hAngle = hAngle;

		dir[0] = cos(vAngle * 3.14f / 180) * sin(hAngle * 3.14f / 180);
		dir[1] = -sin(vAngle * 3.14f / 180);
		dir[2] = cos(vAngle * 3.14f / 180) * cos(hAngle * 3.14f / 180);

		float amb[4] = { 0.2f, 0.15f, 0.1f, 1.0f };
		float diff[4] = { 0.8f, 0.6f, 0.4f, 1.0f };
		float spec[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 100.0f;
		int texcount = 0;

		aabb_max[0] = 1.0f + x; aabb_max[1] = 1.0f + y;	aabb_max[2] = 1.0f + z;
		aabb_min[0] = x; aabb_max[1] = y;	aabb_max[2] = z;


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

	bool detectColisionAABBbox(struct update_info* uInfo) {
		bool col_detected = false;
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
		return col_detected;
	}
	void updateAABB() {
		this->aabb_max[0] = this->pos[0] + (0.5f * std::fabs(cos(this->hAngle * 3.14f / 180)) + 0.5f * std::fabs(sin(this->hAngle * 3.14f / 180))) * cos(this->vAngle * 3.14f / 180)
										 + std::fmax(1 * sin(this->vAngle * 3.14 / 180) * sin(this->hAngle * 3.14 / 180), 0);;
		
		this->aabb_max[1] = this->pos[1] + 1 * std::fabs(cos(this->vAngle * 3.14f / 180)) + 0.5f * std::fabs(sin(this->vAngle * 3.14f / 180));
		this->aabb_max[2] = this->pos[2] + 0.5f * (std::fabs(sin(this->hAngle * 3.14f / 180)) + std::fabs(cos(this->hAngle * 3.14f / 180))) * cos(this->vAngle * 3.14f / 180) +
							std::fmax(1*sin(this->vAngle * 3.14 /180)*cos(this->hAngle*3.14 / 180), 0);

		this->aabb_min[0] = this->pos[0] - 0.5f * std::fabs(cos(this->hAngle * 3.14f / 180)) - 0.5f * std::fabs(sin(this->hAngle * 3.14f / 180))
										+ std::fmin(1 * sin(this->vAngle * 3.14 / 180) * sin(this->hAngle * 3.14 / 180), 0);
		;
		this->aabb_min[1] = this->pos[1] - 0.5f * std::fabs(sin(this->vAngle));
		this->aabb_min[2] = this->pos[2] - 0.5f * (std::fabs(sin(this->hAngle * 3.14f / 180)) + std::fabs(cos(this->hAngle * 3.14f / 180))) * cos(this->vAngle * 3.14f / 180)
							+ std::fmin(1 * sin(this->vAngle * 3.14 / 180) * cos(this->hAngle * 3.14 / 180), 0);
		;

	}

	void detectColisionSphere(struct update_info* uInfo) {
		for (int i = 0; i < uInfo->snowballs->size(); i++) {
			float pos[3];
			pos[0] = (*(uInfo->snowballs))[i].getPosition()[0];
			pos[1] = (*(uInfo->snowballs))[i].getPosition()[1];
			pos[2] = (*(uInfo->snowballs))[i].getPosition()[2];
			int x = std::max(this->aabb_min[0], std::min(pos[0], this->aabb_max[0]));
			int y = std::max(this->aabb_min[1], std::min(pos[1], this->aabb_max[1]));
			int z = std::max(this->aabb_min[2], std::min(pos[2], this->aabb_max[2]));

			int distance = std::sqrt(
				(x - pos[0]) * (x - pos[0]) +
				(y - pos[1]) * (y - pos[1]) +
				(z - pos[2]) * (z - pos[2]));

			if (distance < (*(uInfo->snowballs))[i].getRadius() ) {
				(*(uInfo->snowballs))[i].KillBall();
			}
		}
	}

	float *get_pos() {
		return pos;
	}

	void update(float deltaTime, struct update_info *uInfo) {
		static float hAngle_prev = 0.0f; static float vAngle_prev = 0.0f;
		hAngle += uInfo->h_turning * 180.0f * deltaTime;
		uInfo->h_turning = 0.0f;

		vAngle += uInfo->v_turning * 180.0f * deltaTime;
		uInfo->v_turning = 0.0f;
		if (vAngle < -30.0f) vAngle = -30.0f;
		else if (vAngle > 30.0f) vAngle = 30.0f;


		speed += uInfo->accelerating * 10.0f * deltaTime;
		uInfo->accelerating = -1.0f;
		if (speed < 0) speed = 0;
		else if (speed > 10.0f) speed = 10.0f;


		updateAABB();
		detectColisionSphere(uInfo);
		if (!detectColisionAABBbox(uInfo)) {
			dir[0] = cos(vAngle * 3.14f / 180) * sin(hAngle * 3.14f / 180);
			dir[1] = -sin(vAngle * 3.14f / 180);
			dir[2] = cos(vAngle * 3.14f / 180) * cos(hAngle * 3.14f / 180);
			for (int i = 0; i < 3; i++) pos[i] += speed * dir[i] * deltaTime;
			vAngle_prev = vAngle;
			hAngle_prev = hAngle;
		}
		else {
			hAngle = hAngle_prev;
			vAngle = vAngle_prev;
			speed = 0;
		}

		if (pos[1] < 0) pos[1] = 0.0f; 
		if (pos[1] > 10.0f) pos[1] = 10.f;
		if (std::fabs(pos[0]) > 11) pos[0] = 11.0f*(pos[0] < 0? -1 : 1);
		if (std::fabs(pos[2]) > 11) pos[2] = 11.0f*(pos[2] < 0 ? -1 : 1);

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

		glUniform1i(rInfo.textured_uniformId, false);

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