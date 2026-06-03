#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

void escrever(int x, int y, const char *string){
    glColor3f(0, 0, 0);
    glRasterPos2f(x, y);
    int len, i;
    len = (int)strlen(string);
    for(i = 0; i < len; i++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
    }
}