#version 430

out vec4 colorOut;

uniform bool dir_l_toggled;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;

	vec3 dirLightDir;
} DataIn;

void main() {

	vec4 spec = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	colorOut = vec4(0.0);

	// calculations for the directional light
	if (dir_l_toggled) {
		vec3 d_l = normalize(DataIn.dirLightDir);
		float intensity = max(dot(n,d_l), 0.0);
		if (intensity > 0.0) {
			vec3 h = normalize(d_l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}
		colorOut += intensity * mat.diffuse + spec;
	}
	
	colorOut = min(colorOut + mat.ambient, 1.0f);
}