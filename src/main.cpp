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

void addPonto(float x, float y) {
    Ponto* p = new Ponto(x, y);
    p->setCor(1.0, 0.0, 0.0);
    p->setLineWidth(10.0);
    formas.push_back(p);
}

void addLinha(float x, float y, bool ativo) {
    static Ponto2D primeiroPonto = {-1.0f, -1.0f};

    if (!ativo) {
        primeiroPonto = {-1.0f, -1.0f}; // Reset para o próximo par de pontos
        return;
    }

    if (primeiroPonto.x == -1.0f) {
        primeiroPonto = {(float)x, (float)y};
    } else {
        Linha* l = new Linha(primeiroPonto.x, primeiroPonto.y, (float)x, (float)y);
        l->setCor(0.0, 1.0, 0.0);
        l->setLineWidth(2.0);
        formas.push_back(l);
        primeiroPonto = {-1.0f, -1.0f}; // Reset para o próximo par de pontos
    }
}

void addPoligono(float x, float y, bool ativo) {
    static bool primeiroPonto = true;
    static Poligono* poligonoAtual = nullptr;

    if (!ativo) {
        primeiroPonto = true;

        if (poligonoAtual && poligonoAtual->getNumVertices() < 3) {
            // Polígono inválido, remove da lista
            formas.pop_back();
            delete poligonoAtual;
        }
        poligonoAtual = nullptr;
        return;
    }

    if (primeiroPonto) {
        poligonoAtual = new Poligono();
        poligonoAtual->setCor(0.0, 0.0, 1.0);
        poligonoAtual->setLineWidth(2.0);
        poligonoAtual->adicionarVertice(x, y);
        primeiroPonto = false;
        formas.push_back(poligonoAtual);
    } else {
        poligonoAtual->adicionarVertice(x, y);
    }
}

void mouse(int button, int state, int x, int y){

    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        switch(choice) {
        case 1:
            addLinha(0, 0, false);      // Reseta demais funções para evitar conflitos
            addPoligono(0, 0, false);

            addPonto(x, HEIGHT - y);
            break;
        case 2:
            addPoligono(0, 0, false);

            addLinha(x, HEIGHT - y, true);
            break;
        case 3:
            addLinha(0, 0, false);

            addPoligono(x, HEIGHT - y, true);
            break;
        }
    }
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        addLinha(0, 0, false);
        addPoligono(0, 0, false);
    }
    glutPostRedisplay();
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& forma : formas) {
        if (!forma->selecionada) forma->draw();
    }

    for (const auto& forma : formas) {
        if (forma->selecionada) { // Separando formas selecionadas para transformações futuras
            forma->draw();
        }
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