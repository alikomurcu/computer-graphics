#version 460 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them
// here for simplicity.
uniform vec4[5] lightPos; // light position in world coordinates
uniform vec4[5] lightColor; // light color

vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 kd = vec3(0.8, 0.8, 0.8);     // diffuse reflectance coefficient
vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
vec3 ks = vec3(0.8, 0.8, 0.8);   // specular reflectance coefficient

uniform vec3 eyePos;

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

void main(void)
{
	// Compute lighting. We assume lightPos and eyePos are in world
	// coordinates. fragWorldPos and fragWorldNor are the interpolated
	// coordinates by the rasterizer.

	vec3 V = normalize(eyePos - vec3(fragWorldPos));
	vec3 N = normalize(fragWorldNor);

	vec3 diffuseColor = {0.0f,0.0f,0.0f};
	vec3 specularColor = {0.0f,0.0f,0.0f};
	vec3 ambientColor = Iamb * ka;
	float NdotL = 0.0f;
	float NdotH = 0.0f;
	for(int i=0; i<5; i++)
	{
		if(lightPos[i].w == 0.0f)		// if light is off,
			continue;
		vec3 LPos = vec3(lightPos[i]);
		vec3 LColor = vec3(lightColor[i]);

		vec3 L = normalize(LPos - vec3(fragWorldPos));
		vec3 H = normalize(L + V);
		NdotL = dot(N, L); // for diffuse component
		NdotH = dot(N, H); // for specular component

		vec3 I = LColor / pow(length(vec3(fragWorldPos - lightPos[i])), 2);		// inverse square law
		diffuseColor += I * kd * max(0, NdotL);
		specularColor += I * ks * pow(max(0, NdotH), 400);
	}
	fragColor = vec4(diffuseColor + specularColor + ambientColor, 1);
}
