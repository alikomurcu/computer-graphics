#version 460 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform float startingVal;
uniform vec3[576] CP;
uniform int samples;
uniform int m, n;

layout(location=0) in vec3 inVertex;
out vec4 fragWorldPos;
out vec3 fragWorldNor;


void setIndex(out int index)
{
	int i = int(((gl_VertexID) / (samples * samples)) / (m * samples));
	int j = int(((gl_VertexID) % int(samples*samples*n)) / (samples*samples));
	index = (i*m + j) * 16;
}

void Bernstein(out float[4] b, out float[4] db, float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	b[0] = (1 - t) * (1 - t) * (1 - t);
	b[1] = 3 * t * (1 - t) * (1 - t);
	b[2] = 3 * t2 * (1 - t);
	b[3] = t3;

	// bezier surface derivative		TODO: check derivatives
	db[0] = -3 * (1 - t) * (1 - t);
	db[1] = 3 * (1 - t) * (1 - t) - 6 * t * (1 - t);
	db[2] = 6 * t * (1 - t) - 3 * t2;
	db[3] = 3 * t2;
}

void main(void)
{
	int index;
	setIndex(index);
	// arrange u-v into [0,1]
	float u = inVertex.x, v = inVertex.y;

	float mm = m/4, nn = n/4;

	float rangeY = 2 * abs(startingVal);
	float rangeX = 2 * abs(startingVal);

	if(mm < nn)
	{
		rangeY *= (mm/nn);
	}
	else if(mm >= nn)
	{
		rangeX *= (nn/mm);
	}

	float sampleStartingValY = startingVal + (rangeY/mm) * int((index/16) / int(mm));
	float sampleStartingValX = startingVal + (rangeX/nn) * int((index/16) % int(mm));

	float sampleRangeY = abs(rangeY/mm);
	float sampleRangeX = abs(rangeX/nn);

	u = (u-sampleStartingValY) / sampleRangeY;
	v = (v-sampleStartingValX) / sampleRangeX;

	float bu[4], dbu[4], bv[4], dbv[4];
	Bernstein(bu, dbu, u);
	Bernstein(bv, dbv, v);

	vec3 vertexPosition =
	CP[index+0] * bu[0] * bv[0] + CP[index+1] * bu[0] * bv[1] + CP[index+2] * bu[0] * bv[2] + CP[index+3] * bu[0] * bv[3] +
	CP[index+4] * bu[1] * bv[0] + CP[index+5] * bu[1] * bv[1] + CP[index+6] * bu[1] * bv[2] + CP[index+7] * bu[1] * bv[3] +
	CP[index+8] * bu[2] * bv[0] + CP[index+9] * bu[2] * bv[1] + CP[index+10] * bu[2] * bv[2] + CP[index+11] * bu[2] * bv[3] +
	CP[index+12] * bu[3] * bv[0] + CP[index+13] * bu[3] * bv[1] + CP[index+14] * bu[3] * bv[2] + CP[index+15] * bu[3] * bv[3];

	vec3 du = vec3(
	CP[index+0]*dbu[0]*bv[0] + CP[index+1]*dbu[0]*bv[1] + CP[index+2]*dbu[0]*bv[2] + CP[index+3]*dbu[0]*bv[3] +
	CP[index+4]*dbu[1]*bv[0] + CP[index+5]*dbu[1]*bv[1] + CP[index+6]*dbu[1]*bv[2] + CP[index+7]*dbu[1]*bv[3] +
	CP[index+8]*dbu[2]*bv[0] + CP[index+9]*dbu[2]*bv[1] + CP[index+10]*dbu[2]*bv[2] + CP[index+11]*dbu[2]*bv[3] +
	CP[index+12]*dbu[3]*bv[0] + CP[index+13]*dbu[3]*bv[1] + CP[index+14]*dbu[3]*bv[2] + CP[index+15]*dbu[3]*bv[3]);

	vec3 dv = vec3(
	CP[index+0]*bu[0]*dbv[0] + CP[index+1]*bu[0]*dbv[1] + CP[index+2]*bu[0]*dbv[2] + CP[index+3]*bu[0]*dbv[3] +
	CP[index+4]*bu[1]*dbv[0] + CP[index+5]*bu[1]*dbv[1] + CP[index+6]*bu[1]*dbv[2] + CP[index+7]*bu[1]*dbv[3] +
	CP[index+8]*bu[2]*dbv[0] + CP[index+9]*bu[2]*dbv[1] + CP[index+10]*bu[2]*dbv[2] + CP[index+11]*bu[2]*dbv[3] +
	CP[index+12]*bu[3]*dbv[0] + CP[index+13]*bu[3]*dbv[1] + CP[index+14]*bu[3]*dbv[2] + CP[index+15]*bu[3]*dbv[3]);

	// Compute the world coordinates of the vertex and its normal.
	// These coordinates will be interpolated during the rasterization
	// stage and the fragment shader will receive the interpolated
	// coordinates.

	fragWorldPos = modelingMatrix * vec4(inVertex.x, inVertex.y, vertexPosition.z, 1);
	fragWorldNor = inverse(transpose(mat3x3(modelingMatrix))) * normalize(cross(du, dv));

	gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex.x, inVertex.y, vertexPosition.z, 1);
}
