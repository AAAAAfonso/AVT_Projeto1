#version 430

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 d_l_pos;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria

out Data {
	vec3 normal;
	vec3 eye;
	vec3 dirLightDir;
} DataOut;

void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.eye = vec3(-pos);

	DataOut.dirLightDir = vec3(-d_l_pos);

	gl_Position = m_pvm * position;	
}