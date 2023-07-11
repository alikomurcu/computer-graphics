#include <stdio.h>
#include <stdlib.h>

int nx,ny;
float dist,left,right,top,bottom;

typedef struct
{
    float x,y,z;
} vec3f;

vec3f e;
vec3f gaze;
vec3f u,v,w;

typedef struct
{
    int x,y,z;
} vec3i;

typedef struct
{
    vec3f o,d;
} ray;

vec3i **image;

vec3f multS(vec3f a,float s)
{
    vec3f result;
    result.x = a.x*s;
    result.y = a.y*s;
    result.z = a.z*s;
    return result;
}

vec3f add(vec3f a, vec3f b)
{
    vec3f result;
    result.x = a.x+b.x;
    result.y = a.y+b.y;
    result.z = a.z+b.z;
    return result;
}

ray generateRay(int i, int j)
{
    ray result;
    float su,sv;
    vec3f m,q,s;
    
    su = (i+0.5)*(right-left)/nx;
    sv = (j+0.5)*(top-bottom)/ny;
    
    m = add(e,multS(gaze,dist));
    
    q = add(m,add(multS(u,left),multS(v,top)));
    
    s = add(q,add(multS(u,su),multS(v,-sv)));
    
    result.o = e;
    result.d = add(s,multS(e,-1));
    
    return result;
}
void writePPM(vec3i **image,int nx,int ny)
{
    int i,j;
    FILE *fp;
    
    fp = fopen("output.ppm","w");
    
    fprintf(fp,"P3\n");
    fprintf(fp,"#output.ppm\n");
    fprintf(fp,"%d %d\n",nx,ny);
    fprintf(fp,"255\n");
    for (i=0;i<ny;i++)
    {
        for (j=0;j<nx;j++)
        {
            fprintf(fp,"%d %d %d\t",image[i][j].x,image[i][j].y,image[i][j].z);
        }
        fprintf(fp,"\n");
    }
}

int main()
{
    int i,j;

    scanf("%d %d",&nx,&ny);

    scanf("%f %f %f %f %f",&dist,&left,&right,&bottom,&top);
    
    scanf("%f %f %f",&(e.x),&(e.y),&(e.z));
    scanf("%f %f %f",&(gaze.x),&(gaze.y),&(gaze.z));
    scanf("%f %f %f",&(v.x),&(v.y),&(v.z));

    w = multS(gaze,-1);
    u.x = 1.0;
    u.y = u.z = 0.0;
    
    image = (vec3i **)malloc(sizeof(vec3i*)*ny);
    for (i=0;i<ny;i++)
        image[i] = (vec3i *)malloc(sizeof(vec3i)*nx);
    
    for (i=0;i<ny;i++)
        for (j=0;j<nx;j++)
            image[i][j].x = image[i][j].y = image[i][j].z = 0;

    /* main raytracing loop */
    for (i=0;i<ny;i++)
        for (j=0;j<nx;j++)
        {
            ray myray = generateRay(j,i);
            vec3f pixel;
            pixel = add(myray.o,myray.d);
            //intersectScene(myray);
            //printf("%.4f %.4f %.4f\n",myray.d.x,myray.d.y,myray.d.z);
            printf("%.4f %.4f %.4f\n",pixel.x,pixel.y,pixel.z);
        }
        
    writePPM(image,nx,ny);
}
