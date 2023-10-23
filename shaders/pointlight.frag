#version 430

out vec4 colorOut;

uniform bool dir_l_toggled;
uniform bool point_l_toggled;
uniform bool spot_l_toggled;
uniform bool fog_toggled;

uniform float spot_l_threshold;

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform int text_mode;


struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

in vec4 pos;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 dirLightDir;
	vec3 pointLightDir[6];
	vec3 spotLightDir[2];
	vec3 spotLightSpot;
	vec2 tex_coord;
} DataIn;

void main() {

	vec4 spec = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	vec3 fogColor = vec3(0.5,0.6,0.7);
	float dist = length(pos); 

	float fogAmount = exp( -dist*0.06 );
	colorOut = vec4(0.0);

	//calculations for the point light
	if (point_l_toggled) {
		for (int i = 0; i < 6; i++) {
			vec3 p_l = normalize(DataIn.pointLightDir[i]);
			float intensity = max(dot(n,p_l), 0.0);
			if (intensity > 0.0) {
				vec3 h = normalize(p_l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess);
			}
			colorOut += intensity * mat.diffuse + spec;
		}
	}

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

	// calculations for the spotlight
	if(spot_l_toggled)  {  //Scene iluminated by a spotlight
		vec3 s_d = normalize(DataIn.spotLightSpot);
		for (int i = 0; i < 2; i++) {
			vec3 s_l = normalize(DataIn.spotLightDir[i]);
			if (dot(s_l, s_d) > spot_l_threshold) {
				float intensity = max(dot(n,s_l), 0.0);
				if (intensity > 0.0) {
					vec3 h = normalize(s_l + e);
					float intSpec = max(dot(h,n), 0.0);
					spec = mat.specular * pow(intSpec, mat.shininess);
				}
				colorOut += intensity * mat.diffuse + spec;
			}
		}
	}
	
	if (text_mode == 2) {
		vec4 texel;
		texel = texture(texmap2, DataIn.tex_coord);
		if (texel.a == 0) discard;
		colorOut = min(texel*colorOut, 1.0f);
	} else if (text_mode == 1) {
		vec4 texel, texel1;
		texel = texture(texmap, DataIn.tex_coord);
		texel1 = texture(texmap1, DataIn.tex_coord);
		colorOut = min(texel*texel1*colorOut, 1.0f);
	} else if (text_mode == 0) {
		colorOut = min(colorOut + mat.ambient, 1.0f);
	}

	if (fog_toggled) {
		vec3 final_color = mix(fogColor, vec3(colorOut), fogAmount );
		colorOut = vec4(final_color, 1);
	}
}