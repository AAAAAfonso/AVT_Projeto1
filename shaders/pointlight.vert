#version 430

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 d_l_pos;

uniform vec4 p_l_pos0;
uniform vec4 p_l_pos1;
uniform vec4 p_l_pos2;
uniform vec4 p_l_pos3;
uniform vec4 p_l_pos4;
uniform vec4 p_l_pos5;

uniform vec4 s_l_pos0;
uniform vec4 s_l_pos1;
uniform vec4 s_l_spot;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 dirLightDir;
	vec3 pointLightDir[6];
	vec3 spotLightDir[2];
	vec3 spotLightSpot;
	vec2 tex_coord;
} DataOut;

out vec4 pos;


void main () {

	pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.eye = vec3(-pos);

	DataOut.dirLightDir = vec3(-d_l_pos);

	DataOut.pointLightDir[0] = vec3(p_l_pos0 - pos);
	DataOut.pointLightDir[1] = vec3(p_l_pos1 - pos);
	DataOut.pointLightDir[2] = vec3(p_l_pos2 - pos);
	DataOut.pointLightDir[3] = vec3(p_l_pos3 - pos);
	DataOut.pointLightDir[4] = vec3(p_l_pos4 - pos);
	DataOut.pointLightDir[5] = vec3(p_l_pos5 - pos);

	DataOut.spotLightDir[0] = vec3(s_l_pos0 - pos);
	DataOut.spotLightDir[1] = vec3(s_l_pos1 - pos);
	DataOut.spotLightSpot = vec3(-s_l_spot);

	DataOut.tex_coord = texCoord.st;

	gl_Position = m_pvm * position;	
}