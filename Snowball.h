#pragma once

#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include <random>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class SnowBall {
private:
	float pos[4];
	float dir[4];
	float speed;
	float radius; float spawn_radius;
	bool alive;
	float respawn_time;
	float spawn_angle;
	MyMesh mesh;


	void createMesh() { 
		float amb[4] = { 0.2f, 0.2f, 0.21f, 1.0f };
		float diff[4] = { 0.4f, 0.4f, 0.42f, 1.0f };
		float spec[4] = { 0.12f, 0.12f, 0.14f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 30.0f;
		int texcount = 0;

		mesh = mesh = createSphere(this->radius, 20);
		memcpy(mesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh.mat.emissive, emissive, 4 * sizeof(float));
		mesh.mat.shininess = shininess;
		mesh.mat.texCount = texcount;
	}

	

	void CalcultateDir() {
		this->dir[0] = (rand() % 361);
		this->dir[1] = 0.0;
		this->dir[2] = (rand() % 361);
		this->dir[3] = 0.0;

		float size = sqrt(dir[0] * dir[0] + dir[2] * dir[2]);

		for (unsigned int i = 0; i < 3;i++) dir[i] = dir[i] / size;
		if (dir[0] * pos[0] + dir[2] * pos[2] >= 0) {
			if (dir[0] * pos[0] > 0) {
				dir[0] = -dir[0];
			} 
			if (dir[2] * pos[2] > 0) {
				dir[2] = -dir[2];
			}
		}
	}

public:

	SnowBall(float radius, float spawn_angle, float spawn_radius) {
		this->radius = radius; this->alive = true;
		this->spawn_angle = spawn_angle; this->spawn_radius = spawn_radius;
		this->speed = (rand() % 300) / 100 + 0.5f; this->respawn_time = (rand() % 1000) / 100;

		this->pos[0] = cos(spawn_angle * 3.14/180) * spawn_radius;
		this->pos[1] = this->radius; 
		this->pos[2] = sin(spawn_angle * 3.14 / 180) * spawn_radius;
		this->pos[3] = 0;

		createMesh();

		CalcultateDir();
	}

	void render(struct render_info rInfo) {
		if (this->alive) {
			GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, mesh.mat.ambient);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, mesh.mat.diffuse);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, mesh.mat.specular);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, mesh.mat.shininess);

			pushMatrix(MODEL);
			translate(MODEL, this->pos[0], this->pos[1], this->pos[2]);

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
	}

	void updateSnowBallSpeed() {
		this->speed += (rand() % 200) / 100;
	}

	void respawnBall(){
		this->spawn_angle = (rand() % 361);

		this->pos[0] = cos(spawn_angle * 3.14 / 180) * spawn_radius;
		this->pos[2] = sin(spawn_angle * 3.14 / 180) * spawn_radius;

		CalcultateDir();

		this->alive = true;

	}


	void updateSnowBallPosition(float deltatime) {
		static float time_passed = 0;

		if(this->alive){ //if ball is alive
			pos[0] += dir[0] * this->speed * deltatime;
			pos[2] += dir[2] * this->speed * deltatime;

			if (sqrt(this->pos[0] * this->pos[0] + this->pos[2] * this->pos[2]) > this->spawn_radius) {
				this->alive = false;
			}
		} else {
			if (time_passed < this->respawn_time) {
				time_passed += deltatime;
			}
			else {
				respawnBall();
				time_passed = 0;
			}
		}
	}

	void KillBall() {
		this->alive = false;
	}

	float* getPosition() {
		return pos;
	}

	float getRadius() {
		return this->radius;
	}
};