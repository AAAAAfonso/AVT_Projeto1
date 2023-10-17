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
	int lives = 5;

	MyMesh board;

	void createMesh() {
		float amb[4] = { 0.15f, 0.1f, 0.06f, 1.0f };
		float diff[4] = { 0.2f, 0.15f, 0.09f, 1.0f };
		float spec[4] = { 0.03f, 0.02f, 0.01f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 100.0f;
		int texcount = 0;

		board = createCube();

		memcpy(board.mat.ambient, amb, 4 * sizeof(float));
		memcpy(board.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(board.mat.specular, spec, 4 * sizeof(float));
		memcpy(board.mat.emissive, emissive, 4 * sizeof(float));
		board.mat.shininess = shininess;
		board.mat.texCount = texcount;
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
		if (lives == 0) reset_lives();
	}

	void reset_lives() {
		lives = 5;
	}

	int get_lives() {
		return lives;
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
		return col_detected;
	}
	void updateAABB() {

		//float angle_0 = 0.5 * cos(0);
		//float angle_x = 0.5 * cos(this->vAngle * 3.14f / 180);

		//float small_dif = (angle_0 - angle_x) *cos(this->hAngle * 3.14f / 180);

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
		this->hAngle = 90.0;
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

	void render(struct render_info rInfo) {
		GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, board.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, board.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, board.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, board.mat.shininess);

		glUniform1i(rInfo.textured_uniformId, false);

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1] + 0.2f, pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		scale(MODEL, 0.6f, 0.1f, 1.0f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(board.vao);

		glDrawElements(board.type, board.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, board.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, board.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, board.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, board.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		translate(MODEL, 0.25f, 0.0f, 0.0f);
		scale(MODEL, 0.1f, 0.2f, 1.0f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(board.vao);

		glDrawElements(board.type, board.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, board.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, board.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, board.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, board.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		translate(MODEL, -0.25f, 0.0f, 0.0f);
		scale(MODEL, 0.1f, 0.2f, 1.0f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(board.vao);

		glDrawElements(board.type, board.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, board.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, board.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, board.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, board.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		translate(MODEL, 0.0f, 0.3f, -0.45f);
		scale(MODEL, 0.6f, 0.3f, 0.1f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(board.vao);

		glDrawElements(board.type, board.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, board.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, board.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, board.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, board.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		translate(MODEL, 0.0f, 0.3f, 0.45f);
		scale(MODEL, 0.6f, 0.2f, 0.1f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(board.vao);

		glDrawElements(board.type, board.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, board.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, board.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, board.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, board.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		translate(MODEL, -0.25f, 0.3f, -0.2f);
		scale(MODEL, 0.1f, 0.3f, 0.4f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(board.vao);

		glDrawElements(board.type, board.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, board.mat.ambient);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, board.mat.diffuse);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, board.mat.specular);
		loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, board.mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, pos[0], pos[1], pos[2]);
		rotate(MODEL, hAngle, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, vAngle, 1.0f, 0.0f, 0.0f);
		translate(MODEL, 0.25f, 0.3f, -0.2f);
		scale(MODEL, 0.1f, 0.3f, 0.4f);
		translate(MODEL, -0.5f, 0.0f, -0.5f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(board.vao);

		glDrawElements(board.type, board.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
};