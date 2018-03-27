#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int v1, v2, v3;

    glutInit(&argc, argv);
    glutCreateWindow("glstats");

    v1=v2=v3=0;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,&v1);
    glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT,&v2);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,&v3);
    printf("uniforms\n");
    printf("  vertex:   %d components\n", v1);
    printf("  geometry: %d components\n", v2);
    printf("  fragment: %d components\n", v3);

    v1=0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &v1);
    printf("per-vertex attribs\n");
    printf("  %d 4-element vectors\n", v1);

    v1=v2=0;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &v1);
    glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT, &v2);
    printf("geometry output\n");
    printf("  %d vertices\n", v1);
    printf("  %d total components\n", v2);

    v1=v2=v3=0;
    glGetIntegerv(GL_MAX_VERTEX_VARYING_COMPONENTS_EXT, &v1);
    glGetIntegerv(GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT, &v2);
    glGetIntegerv(GL_MAX_VARYING_FLOATS, &v3);
    printf("varying\n");
    printf("  %d components vertex to geometry\n", v1);
    printf("  %d components geometry to fragment\n", v2);
    printf("  %d components vertex to fragment\n", v3);

    v1=v2=v3=0;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &v1);
    glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT, &v2);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &v3);
    printf("texture units\n");
    printf("  vertex:   %d textures\n", v1);
    printf("  geometry: %d textures\n", v2);
    printf("  fragment: %d textures\n", v3);

    v1=v2=v3=0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &v1);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &v2);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &v3);
    printf("max texture sizes\n");
    printf("  2D: %d x %d\n", v1,v1);
    printf("  3D: %d x %d x %d\n", v2,v2,v2);
    printf("  %d texture array layers\n", v3);
    return 0;
}
