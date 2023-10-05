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


	MyMesh createHouse() { return createCube(); }
	MyMesh createRooftop() { return createCone(0.5f, 0.75f, 4); }


public:
	House( float x, float z) {

		this->x = x; this->z = z;
		float amb[4] = { 0.2f, 0.15f, 0.1f, 1.0f };
		float diff[4] = { 0.8f, 0.6f, 0.4f, 1.0f };
		float spec[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
		float emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float shininess = 100.0f;
		int texcount = 0;

		aabb_max[0] = this->x + 1; aabb_max[1] = 1.0f; aabb_max[2] = this->z + 1.0;
		aabb_min[0] = this->x; aabb_min[1] = 0.0f; aabb_min[2] = this->z;


		house = createHouse();
		rooftop = createRooftop();
		memcpy(house.mat.ambient, amb, 4 * sizeof(float));
		memcpy(house.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(house.mat.specular, spec, 4 * sizeof(float));
		memcpy(house.mat.emissive, emissive, 4 * sizeof(float));
		house.mat.shininess = shininess;
		house.mat.texCount = texcount;

		memcpy(rooftop.mat.ambient, amb, 4 * sizeof(float));
		memcpy(rooftop.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(rooftop.mat.specular, spec, 4 * sizeof(float));
		memcpy(rooftop.mat.emissive, emissive, 4 * sizeof(float));
		rooftop.mat.shininess = shininess;
		rooftop.mat.texCount = texcount;
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
			translate(MODEL, this->x, 0.0f, this->z);

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
			translate(MODEL,  this->x + 0.5f, 1.0f, this->z + 0.5f);
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