#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include "formas.h"
#include <vector>

#define HEIGHT 300
#define WIDTH 400

int choice = 1;
std::vector<Forma*> formas;

int init(){
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
    return 0;
}

void keyboard(unsigned char key, int x, int y){
    switch(key){
        case '1':
            choice = 1;
            break;
        case '2':
            choice = 2;
            break;
        case '3':
            choice = 3;
            break;
        case 'z':
            if (!formas.empty()) {
                delete formas.back();
                formas.pop_back();
            }
            break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y){
    static Ponto2D primeiroPonto;
    static Ponto2D segundoPonto;
    static bool desenhandoPoligono = false;
    static Poligono* poligonoAtual = nullptr;

    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        if(choice == 1){
            Ponto* p = new Ponto(x, HEIGHT - y);
            p->setCor(1.0, 0.0, 0.0);
            p->setLineWidth(10.0);
            formas.push_back(p);
        }
        if(choice == 2){
            if (primeiroPonto.x == -1.0f && primeiroPonto.y == -1.0f) {
                primeiroPonto = {(float)x, (float)(HEIGHT - y)};
            } else {
                segundoPonto = {(float)x, (float)(HEIGHT - y)};
                Linha* l = new Linha(primeiroPonto.x, primeiroPonto.y, segundoPonto.x, segundoPonto.y);
                l->setCor(0.0, 1.0, 0.0);
                l->setLineWidth(2.0);
                formas.push_back(l);
                primeiroPonto = {-1.0f, -1.0f}; // Reset para o próximo par de pontos
            }
        }
        if(choice == 3){
            if (!desenhandoPoligono) {
                poligonoAtual = new Poligono();
                poligonoAtual->setCor(0.0, 0.0, 1.0);
                poligonoAtual->setLineWidth(2.0);
                poligonoAtual->adicionarVertice(x, HEIGHT - y);
                desenhandoPoligono = true;
                formas.push_back(poligonoAtual);
            } else {
            formas.pop_back(); // Remove o polígono anterior para atualizar com o novo vértice
            poligonoAtual->adicionarVertice(x, HEIGHT - y);
            formas.push_back(poligonoAtual); // Adiciona o polígono atualizado
            }
        }    
    }
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && desenhandoPoligono) {
        poligonoAtual = nullptr;
        desenhandoPoligono = false;
    }
    glutPostRedisplay();
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& forma : formas) {
        forma->draw();
    }
    glutSwapBuffers();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 300);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenGL Example");

    printf("Pressione 1 para desenhar pontos, 2 para desenhar linhas e 3 para desenhar poligonos.\nClique com o mouse para desenhar. Clique com o botao direito para finalizar um poligono.\n");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}