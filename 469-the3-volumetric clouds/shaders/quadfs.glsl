#version 460 core

in vec3 fragCoord;

out vec4 fragColor;
uniform float iTime;
uniform float ix, iy, iz;
uniform vec2 resolution;

#define PERLIN 0 // 1 for perlin noise and 0 for another noise

#define MARCH_COUNT 100
#define NUM_NOISE_OCTAVES 4

#define HEIGHT_OFFSET 1.25

#define WHITE_NOISE_GRID_SIZE 256.0

#define HASHSCALE 443.8975

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
    vec2(12.9898,78.233)))
    * 43758.5453123);
}


float f(float x)
{
    if (x<1)
        return 6*x*x*x*x*x - 15*x*x*x*x + abs(10*x*x*x) + 1;
    else return 0;
}

float perlin(vec3 pos)
{
    vec3 gradients[16] = {
    vec3(1, 1, 0),
    vec3(-1, 1, 0),
    vec3(1, -1, 0),
    vec3(-1, -1, 0),
    vec3(1, 0, 1),
    vec3(-1, 0, 1),
    vec3(1, 0, -1),
    vec3(-1, 0, -1),
    vec3(0, 1, 1),
    vec3(0, -1, 1),
    vec3(0, 1, -1),
    vec3(0, -1, -1),
    vec3(1, 1, 0),
    vec3(-1, 1, 0),
    vec3(0, -1, 1),
    vec3(0, -1, -1)
    };
    int table[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    int idx;

    float c = 0.0;
    for (int i_iter = 0; i_iter < 2; i_iter++)
    {
        for (int j_iter=0; j_iter < 2; j_iter++)
        {
            for (int k_iter=0; k_iter < 2; k_iter++)
            {
                float i = pos.x;
                float j = pos.y;
                float k = pos.z;

                float ii = i_iter == 0 ? floor(i): ceil(i);
                float jj = j_iter == 0 ? floor(j): ceil(j);
                float kk = k_iter == 0 ? floor(k): ceil(k);
                int inti = int(ii);
                int intj = int(jj);
                int intk = int(kk);
                idx = table[abs(intk) % 16];
                idx = table[abs(intj + idx) % 16];
                idx = table[abs(inti + idx) % 16];
                vec3 g = gradients[idx];
                float dotProduct = dot(vec3(i - ii, j - jj, k - kk), g);
                float fu = f(i - ii);
                float fv = f(j - jj);
                float fw = f(k - kk);
                float w = fu * fv * fw;
                c += w * dotProduct;
            }
        }
    }

    return (c+1)/2;
}

float hash12(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * HASHSCALE);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}


float bilinear_white_noise (vec2 uv)
{
    uv = fract(uv);

    vec2 uvPixels = uv * WHITE_NOISE_GRID_SIZE;

    vec2 uv_frac = uvPixels - floor(uvPixels);

    vec2 uv_floor = floor(uvPixels) / WHITE_NOISE_GRID_SIZE;
    vec2 uv_ceil = ceil(uvPixels) / WHITE_NOISE_GRID_SIZE;

    float noise00 = hash12(vec2(uv_floor.x, uv_floor.y));
    float noise01 = hash12(vec2(uv_floor.x, uv_ceil.y ));
    float noise10 = hash12(vec2(uv_ceil.x , uv_floor.y));
    float noise11 = hash12(vec2(uv_ceil.x , uv_ceil.y ));

    float noise0 = mix(noise00, noise01, uv_frac.y);
    float noise1 = mix(noise10, noise11, uv_frac.y);

    return mix(noise0, noise1, uv_frac.x);
}


float random_num (in vec3 pos)
{
    vec2 v = (pos.yz+ceil(pos.x))/float(MARCH_COUNT);
    return bilinear_white_noise(v);
}

vec4 march_rays(vec3 direction, vec4 pixel_color, vec4 sky_color)
{
    for (int ray_step = 0; ray_step < MARCH_COUNT; ++ray_step)
    {
        vec3 position = 0.05 * float(MARCH_COUNT - ray_step) * direction;

        position.xy+=iTime/25.0;

        position.x += iz/50.0;
        position.y += ix/50.0;
        position.z += iy/50.0;

        float noiseScale=0.5;
        #if PERLIN
        pixel_color += min(perlin(position), 1.0) * pixel_color;
        #else
        float cloud_distance = position.z + HEIGHT_OFFSET;

        for (int i = 0; i < NUM_NOISE_OCTAVES; ++i)
        {
            position *= 2.0;
            noiseScale *= 2.0;
            cloud_distance -= random_num(position) / noiseScale;
        }

        if (cloud_distance < 0.0)
        pixel_color = pixel_color + (pixel_color - 1.0 - cloud_distance * sky_color.zyxw)*cloud_distance*0.5;
        #endif
    }
    return pixel_color;
}

void main(void)
{
    vec3 direction = vec3(0.8, gl_FragCoord/resolution.y-0.65);
    direction.z = -direction.z;

    vec4 sky_color = vec4(0.6, 0.7, 0.8, 0.0);

    vec4 pixel_color = sky_color; // initialize the color to the sky color
    vec4 tmp = pixel_color;  // store the initial color to check if it's the same after the loop

    pixel_color = march_rays(direction, pixel_color, sky_color);
    if (tmp == pixel_color) // if the color didn't change, it means the ray didn't hit anything
        discard;

    fragColor = vec4(pixel_color);
}