#pragma once
#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include <random>

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

class House {
private:
	
	float x, z;
	float aabb_max[4];
	float aabb_min[4];

	MyMesh house;
	MyMesh rooftop;

	void createMesh() {
		float house_amb[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
		float house_diff[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		float house_spec[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float house_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float house_shininess = 100.0f;
		int house_texcount = 0;

		house = createCube();
		memcpy(house.mat.ambient, house_amb, 4 * sizeof(float));
		memcpy(house.mat.diffuse, house_diff, 4 * sizeof(float));
		memcpy(house.mat.specular, house_spec, 4 * sizeof(float));
		memcpy(house.mat.emissive, house_emissive, 4 * sizeof(float));
		house.mat.shininess = house_shininess;
		house.mat.texCount = house_texcount;

		float rooftop_amb[4] = { 0.2f, 0.1f, 0.07f, 1.0f };
		float rooftop_diff[4] = { 0.4f, 0.2f, 0.15f, 1.0f };
		float rooftop_spec[4] = { 0.1f, 0.07f, 0.05f, 1.0f };
		float rooftop_emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float rooftop_shininess = 100.0f;
		int rooftop_texcount = 0;

		rooftop = createCone(1.0f, 1.42f, 4);
		memcpy(rooftop.mat.ambient, rooftop_amb, 4 * sizeof(float));
		memcpy(rooftop.mat.diffuse, rooftop_diff, 4 * sizeof(float));
		memcpy(rooftop.mat.specular, rooftop_spec, 4 * sizeof(float));
		memcpy(rooftop.mat.emissive, rooftop_emissive, 4 * sizeof(float));
		rooftop.mat.shininess = rooftop_shininess;
		rooftop.mat.texCount = rooftop_texcount;
	}

public:
	House( float x, float z) {

		this->x = x; this->z = z;

		aabb_max[0] = this->x + 1.0f; aabb_max[1] = 2.5f; aabb_max[2] = this->z + 1.0f;
		aabb_min[0] = this->x - 1.0f; aabb_min[1] = 0.0f; aabb_min[2] = this->z - 1.0f;

		createMesh();
	}

	void render(struct render_info rInfo) {
			GLint loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, house.mat.ambient);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, house.mat.diffuse);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, house.mat.specular);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, house.mat.shininess);

			pushMatrix(MODEL);
			translate(MODEL, this->x - 1.0f, 0.0f, this->z - 1.0f);
			scale(MODEL, 2.0f, 1.5f, 2.0f);

			// send matrices to OGL
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// Render mesh
			glBindVertexArray(house.vao);

			glDrawElements(house.type, house.numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			popMatrix(MODEL);

			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, rooftop.mat.ambient);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, rooftop.mat.diffuse);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, rooftop.mat.specular);
			loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, rooftop.mat.shininess);

			pushMatrix(MODEL);
			translate(MODEL,  this->x, 1.5f, this->z);
			rotate(MODEL, -45, 0.0f, 1.0f, 0.0f);
			// send matrices to OGL
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// Render mesh
			glBindVertexArray(rooftop.vao);

			glDrawElements(rooftop.type, rooftop.numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			popMatrix(MODEL);
	}



	void updateHouse(float deltatime) {
		
	}
};