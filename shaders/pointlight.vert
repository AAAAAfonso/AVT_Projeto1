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

uniform int text_mode;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec4 texCoord;
in vec4 tangent;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 dirLightDir;
	vec3 pointLightDir[6];
	vec3 spotLightDir[2];
	vec3 spotLightSpot;
	vec2 tex_coord;
	vec2 sphere_coord;
} DataOut;

out vec4 pos;


void main () {
	vec3 n, t, b;
	vec3 eye, d_l, p_l0, p_l1, p_l2, p_l3, p_l4, p_l5, s_l0, s_l1, s_d;
	vec3 aux;

	pos = m_viewModel * position;

	n = normalize(m_normal * normal.xyz);
	eye = vec3(-pos);

	d_l = vec3(-d_l_pos);

	p_l0 = vec3(p_l_pos0 - pos);
	p_l1 = vec3(p_l_pos1 - pos);
	p_l2 = vec3(p_l_pos2 - pos);
	p_l3 = vec3(p_l_pos3 - pos);
	p_l4 = vec3(p_l_pos4 - pos);
	p_l5 = vec3(p_l_pos5 - pos);

	s_l0 = vec3(s_l_pos0 - pos);
	s_l1 = vec3(s_l_pos1 - pos);
	s_d = vec3(-s_l_spot);

	/*if (text_mode == 4) {
		t = normalize(m_normal * tangent.xyz);  
		b = tangent.w * cross(n,t);
	
		aux.x = dot(eye, t);
		aux.y = dot(eye, b);
		aux.z = dot(eye, n);
		eye = normalize(aux);

		aux.x = dot(d_l, t);
		aux.y = dot(d_l, b);
		aux.z = dot(d_l, n);
		d_l = normalize(aux);

		aux.x = dot(p_l0, t);
		aux.y = dot(p_l0, b);
		aux.z = dot(p_l0, n);
		p_l0 = normalize(aux);
		aux.x = dot(p_l1, t);
		aux.y = dot(p_l1, b);
		aux.z = dot(p_l1, n);
		p_l1 = normalize(aux);
		aux.x = dot(p_l2, t);
		aux.y = dot(p_l2, b);
		aux.z = dot(p_l2, n);
		p_l2 = normalize(aux);
		aux.x = dot(p_l3, t);
		aux.y = dot(p_l3, b);
		aux.z = dot(p_l3, n);
		p_l3 = normalize(aux);
		aux.x = dot(p_l4, t);
		aux.y = dot(p_l4, b);
		aux.z = dot(p_l4, n);
		p_l4 = normalize(aux);
		aux.x = dot(p_l5, t);
		aux.y = dot(p_l5, b);
		aux.z = dot(p_l5, n);
		p_l5 = normalize(aux);

		aux.x = dot(s_l0, t);
		aux.y = dot(s_l0, b);
		aux.z = dot(s_l0, n);
		s_l0 = normalize(aux);
		aux.x = dot(s_l1, t);
		aux.y = dot(s_l1, b);
		aux.z = dot(s_l1, n);
		s_l1 = normalize(aux);
		aux.x = dot(s_d, t);
		aux.y = dot(s_d, b);
		aux.z = dot(s_d, n);
		s_d = normalize(aux);
	}*/

	DataOut.normal = n;
	DataOut.eye = eye;

	DataOut.dirLightDir = d_l;

	DataOut.pointLightDir[0] = p_l0;
	DataOut.pointLightDir[1] = p_l1;
	DataOut.pointLightDir[2] = p_l2;
	DataOut.pointLightDir[3] = p_l3;
	DataOut.pointLightDir[4] = p_l4;
	DataOut.pointLightDir[5] = p_l5;

	DataOut.spotLightDir[0] = s_l0;
	DataOut.spotLightDir[1] = s_l1;
	DataOut.spotLightSpot = s_d;

	DataOut.tex_coord = texCoord.st;

	gl_Position = m_pvm * position;

	if(text_mode == 6) {
		vec3 r = reflect(-DataOut.eye, DataOut.normal);
		float m = 2.0 * sqrt( r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));
		DataOut.sphere_coord.s = r.x/m + 0.5;
		DataOut.sphere_coord.t = r.y/m + 0.5; 
	}
}