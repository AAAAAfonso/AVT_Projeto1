#pragma once

#include <geometry.h>
#include <render_info.h>
#include <AVTmathLib.h>
#include <random>
#include "flare.h"

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];
std::string flarepath = "flare.txt";



class LensFlare {
private:
	
	GLuint FlareTextureArray[5];
	MyMesh FlareMesh;
	FLARE_DEF* AVTflare = (FLARE_DEF *) malloc(sizeof(FLARE_DEF));


	void createMesh(float size) {
		this->FlareMesh = createQuad(size, size);
	}


public:
	

	LensFlare(float size, char* filename) {
		
		glGenTextures(5, this->FlareTextureArray);
		Texture2D_Loader(this->FlareTextureArray, "crcl.tga", 0);
		Texture2D_Loader(this->FlareTextureArray, "flar.tga", 1);
		Texture2D_Loader(this->FlareTextureArray, "hxgn.tga", 2);
		Texture2D_Loader(this->FlareTextureArray, "ring.tga", 3);
		Texture2D_Loader(this->FlareTextureArray, "sun.tga", 4);

		loadFlareFile(filename);
		createMesh(size);
	}

	unsigned int getTextureId(char* name) {
		int i;

		for (i = 0; i < NTEXTURES; ++i)
		{
			if (strncmp(name, flareTextureNames[i], strlen(name)) == 0)
				return i;
		}
		return -1;
	}


	void loadFlareFile(char* filename)
	{
		int     n = 0;
		FILE* f;
		char    buf[256];
		int fields;

		memset(this->AVTflare, 0, sizeof(FLARE_DEF));

		f = fopen(flarepath.c_str(), "r");
		if (f)
		{
			fgets(buf, sizeof(buf), f);
			sscanf(buf, "%f %f", &(this->AVTflare)->fScale, &(this->AVTflare)->fMaxSize);

			while (!feof(f))
			{
				char            name[8] = { '\0', };
				double          dDist = 0.0, dSize = 0.0;
				float			color[4];
				int				id;

				fgets(buf, sizeof(buf), f);
				fields = sscanf(buf, "%4s %lf %lf ( %f %f %f %f )", name, &dDist, &dSize, &color[3], &color[0], &color[1], &color[2]);
				if (fields == 7)
				{
					for (int i = 0; i < 4; ++i) color[i] = clamp(color[i] / 255.0f, 0.0f, 1.0f);
					id = getTextureId(name);
					if (id < 0) printf("Texture name not recognized\n");
					else
						this->AVTflare->element[n].textureId = id;
					this->AVTflare->element[n].fDistance = (float)dDist;
					this->AVTflare->element[n].fSize = (float)dSize;
					memcpy(this->AVTflare->element[n].matDiffuse, color, 4 * sizeof(float));
					++n;
				}
			}

			this->AVTflare->nPieces = n;
			fclose(f);
		}
		else printf("Flare file opening error\n");
	}

	void render(struct render_info rInfo,int lx, int ly, int* m_viewport) {  //lx, ly represent the projected position of light on viewport

			int     dx, dy;          // Screen coordinates of "destination"
			int     px, py;          // Screen coordinates of flare element
			int		cx, cy;
			float    maxflaredist, flaredist, flaremaxsize, flarescale, scaleDistance;
			int     width, height, alpha;    // Piece parameters;
			int     i;
			float	diffuse[4];

			GLint loc;

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			int screenMaxCoordX = m_viewport[0] + m_viewport[2] - 1;
			int screenMaxCoordY = m_viewport[1] + m_viewport[3] - 1;

			//viewport center
			cx = m_viewport[0] + (int)(0.5f * (float)m_viewport[2]) - 1;
			cy = m_viewport[1] + (int)(0.5f * (float)m_viewport[3]) - 1;

			// Compute how far off-center the flare source is.
			maxflaredist = sqrt(cx * cx + cy * cy);
			flaredist = sqrt((lx - cx) * (lx - cx) + (ly - cy) * (ly - cy));
			scaleDistance = (maxflaredist - flaredist) / maxflaredist;
			flaremaxsize = (int)(m_viewport[2] * this->AVTflare->fMaxSize);
			flarescale = (int)(m_viewport[2] * this->AVTflare->fScale);

			// Destination is opposite side of centre from source
			dx = clampi(cx + (cx - lx), m_viewport[0], screenMaxCoordX);
			dy = clampi(cy + (cy - ly), m_viewport[1], screenMaxCoordY);

			// Render each element. To be used Texture Unit 0
			GLint tex_loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "texmap");
			glUniform1i(rInfo.textMode_uniformId, 7); // draw modulated textured particles
			glUniform1i(tex_loc, 10);  //use TU 10

			for (i = 0; i < this->AVTflare->nPieces; ++i)
			{
				// Position is interpolated along line between start and destination.
				px = (int)((1.0f - this->AVTflare->element[i].fDistance) * lx + this->AVTflare->element[i].fDistance * dx);
				py = (int)((1.0f - this->AVTflare->element[i].fDistance) * ly + this->AVTflare->element[i].fDistance * dy);
				px = clampi(px, m_viewport[0], screenMaxCoordX);
				py = clampi(py, m_viewport[1], screenMaxCoordY);

				// Piece size are 0 to 1; flare size is proportion of screen width; scale by flaredist/maxflaredist.
				width = (int)(scaleDistance * flarescale * this->AVTflare->element[i].fSize);

				// Width gets clamped, to allows the off-axis flaresto keep a good size without letting the elements get big when centered.
				if (width > flaremaxsize)  width = flaremaxsize;

				height = (int)((float)m_viewport[3] / (float)m_viewport[2] * (float)width);
				memcpy(diffuse, this->AVTflare->element[i].matDiffuse, 4 * sizeof(float));
				diffuse[3] *= scaleDistance;   //scale the alpha channel

				if (width > 1)
				{
					// send the material - diffuse color modulated with texture
					loc = glGetUniformLocation(rInfo.shader.getProgramIndex(), "mat.diffuse");
					glUniform4fv(loc, 1, diffuse);

					glActiveTexture(GL_TEXTURE10);
					glBindTexture(GL_TEXTURE_2D, this->FlareTextureArray[this->AVTflare->element[i].textureId]);
					pushMatrix(MODEL);
					translate(MODEL, (float)(px - width * 0.0f), (float)(py - height * 0.0f), 0.0f);
					scale(MODEL, (float)width, (float)height, 1);
					computeDerivedMatrix(PROJ_VIEW_MODEL);
					glUniformMatrix4fv(rInfo.vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
					glUniformMatrix4fv(rInfo.pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
					computeNormalMatrix3x3();
					glUniformMatrix3fv(rInfo.normal_uniformId, 1, GL_FALSE, mNormal3x3);

					glBindVertexArray(FlareMesh.vao);
					glDrawElements(FlareMesh.type, FlareMesh.numIndexes, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
					popMatrix(MODEL);
				}
			}
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
		}
	};
