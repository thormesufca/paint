#include <GL/glut.h>
#include <stdio.h>
#include "formas.h"
#include "matriz.h"
#include "funcoes.h"
#include <vector>
#include <algorithm>

#define HEIGHT 600
#define WIDTH 800
#define SPEED 2.0

int choice = 1;
std::vector<Forma*> formas;

Ponto2D mouseAtual     = {-1.0f, -1.0f};
Ponto2D linhaPrimeiroP = {-1.0f, -1.0f};
Poligono* poligonoAtual       = nullptr;
bool      poligonoPrimeiroPonto = true;
bool  arrastando     = false;
float xArrastaInicio = 0.0f;
float yArrastaInicio = 0.0f;
Forma* formaSelecionada = nullptr;



int init(){
    glClearColor(1.0, 1.0, 1.0, 1.0);
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
        case '4':
            choice = 4;
            break;
        case 'R':
        case 'r':
            if(formaSelecionada) formaSelecionada->rotacionar(-45.0f);
            break;
        case 'E':
        case 'e':
            if(formaSelecionada) formaSelecionada->rotacionar(45.0f);
            break;
        case 'z':
            if (formaSelecionada) {
                auto it = std::find(formas.begin(), formas.end(), formaSelecionada);
                if(it != formas.end()) formas.erase(it);
                delete formaSelecionada;
                formaSelecionada = nullptr;
            }
            break;
        case 'c':
        case 'C':
            if(formaSelecionada) formaSelecionada->cisalharHorizontal(0.1f);
            break;
        case 'x':
        case 'X':
            if(formaSelecionada) formaSelecionada->cisalharHorizontal(-0.1f);
            break;
        case 'w':
        case 'W':
            if(formaSelecionada) formaSelecionada->transladar(0, 1.0f * SPEED);
            break;
        case 'a':
        case 'A':
            if(formaSelecionada) formaSelecionada->transladar(-1.0f * SPEED, 0);
            break;
        case 's':
        case 'S':
            if(formaSelecionada) formaSelecionada->transladar(0, -1.0f * SPEED);
            break;
        case 'd':
        case 'D':
            if(formaSelecionada) formaSelecionada->transladar(1.0f * SPEED, 0);
            break;
        }
    glutPostRedisplay();
}

void teclasEspeciais(int key, int x, int y){
    float dx = 0;
    float dy = 0;
    switch (key)
    {
    case GLUT_KEY_LEFT:
        dx = -1.0 * SPEED;
        break;
    case GLUT_KEY_RIGHT:
        dx = 1.0 * SPEED;
        break;
    case GLUT_KEY_DOWN:
        dy = -1.0 * SPEED;
        break;
    case GLUT_KEY_UP:
        dy = 1.0 * SPEED;
        break;
    default:
        break;
    }
    for(auto &forma: formas){
        forma->transladar(dx, dy);
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
    if (!ativo) {
        linhaPrimeiroP = {-1.0f, -1.0f};
        return;
    }

    if (linhaPrimeiroP.x == -1.0f) {
        linhaPrimeiroP = {x, y};
    } else {
        Linha* l = new Linha(linhaPrimeiroP.x, linhaPrimeiroP.y, x, y);
        l->setCor(0.0, 1.0, 0.0);
        l->setLineWidth(2.0);
        formas.push_back(l);
        linhaPrimeiroP = {-1.0f, -1.0f};
    }
}

void addPoligono(float x, float y, bool ativo) {
    if (!ativo) {
        poligonoPrimeiroPonto = true;

        if (poligonoAtual && poligonoAtual->getNumVertices() < 3) {
            formas.pop_back();
            delete poligonoAtual;
        }
        poligonoAtual = nullptr;
        return;
    }

    if (poligonoPrimeiroPonto) {
        poligonoAtual = new Poligono();
        poligonoAtual->setCor(0.0, 0.0, 1.0);
        poligonoAtual->setLineWidth(2.0);
        poligonoAtual->adicionarVertice(x, y);
        poligonoPrimeiroPonto = false;
        formas.push_back(poligonoAtual);
    } else {
        poligonoAtual->adicionarVertice(x, y);
    }
}

void motionPassiva(int x, int y) {
    mouseAtual = {(float)x, (float)(HEIGHT - y)};
    bool temPreview = (choice == 2 && linhaPrimeiroP.x != -1.0f) ||
                      (choice == 3 && !poligonoPrimeiroPonto);
    if (temPreview) glutPostRedisplay();
}

void motion(int x, int y) {
    float fx = (float)x;
    float fy = (float)(HEIGHT - y);
    mouseAtual = {fx, fy};

    // if (arrastando) {
    //     float dx = fx - xArrastaInicio;
    //     float dy = fy - yArrastaInicio;
    //     for (auto& forma : formas) {
    //         if (forma->selecionada) forma->transladar(dx, dy);
    //     }
    //     xArrastaInicio = fx;
    //     yArrastaInicio = fy;
    // }

    bool temPreview = (choice == 2 && linhaPrimeiroP.x != -1.0f) ||
                      (choice == 3 && !poligonoPrimeiroPonto);
    if (temPreview) glutPostRedisplay();
}

void mouse(int button, int state, int x, int y){
    float fx = (float)x;
    float fy = (float)(HEIGHT - y);

    if(button == GLUT_LEFT_BUTTON){
        if(state == GLUT_DOWN){
            bool temSelecionada = false;
            //for (auto& forma : formas)
                // if (forma->selecionada) { temSelecionada = true; break; }

            // if (temSelecionada) {
            //     arrastando = true;
            //     xArrastaInicio = fx;
            //     yArrastaInicio = fy;
            // } else
            if (!temSelecionada) {
                switch(choice) {
                case 1:
                    addLinha(0, 0, false);
                    addPoligono(0, 0, false);
                    addPonto(fx, fy);
                    break;
                case 2:
                    addPoligono(0, 0, false);
                    addLinha(fx, fy, true);
                    break;
                case 3:
                    addLinha(0, 0, false);
                    addPoligono(fx, fy, true);
                    break;
                case 4:
                    for(auto &forma: formas)
                        forma->selecionada = false;
                    formaSelecionada = nullptr;
                    for(auto &forma: formas){
                        if(forma->selecionar(fx, fy)){
                            formaSelecionada = forma;
                            break;
                        }
                    }
                }
            }
        } else if(state == GLUT_UP){
            // arrastando = false;
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
        if (forma->selecionada) {
            forma->draw();
        }
    }
    // preview rubber-band
    if (mouseAtual.x != -1.0f) {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0xAAAA);
        glLineWidth(1.0f);

        if (choice == 2 && linhaPrimeiroP.x != -1.0f) {
            glColor3f(0.0f, 1.0f, 0.0f);
            glBegin(GL_LINES);
                glVertex2f(linhaPrimeiroP.x, linhaPrimeiroP.y);
                glVertex2f(mouseAtual.x, mouseAtual.y);
            glEnd();
        }

        if (choice == 3 && !poligonoPrimeiroPonto && poligonoAtual) {
            auto pts = poligonoAtual->getPontos();

            // draw() ignora polígonos com < 3 vértices; desenha as arestas existentes manualmente
            if (pts.size() == 2) {
                glDisable(GL_LINE_STIPPLE);
                glColor3f(0.0f, 0.0f, 1.0f);
                glLineWidth(2.0f);
                glBegin(GL_LINES);
                    glVertex2f(pts[0].x, pts[0].y);
                    glVertex2f(pts[1].x, pts[1].y);
                glEnd();
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0xAAAA);
                glLineWidth(1.0f);
            }

            if (!pts.empty()) {
                glColor3f(0.0f, 0.0f, 1.0f);
                glBegin(GL_LINES);
                    glVertex2f(pts.back().x, pts.back().y);
                    glVertex2f(mouseAtual.x, mouseAtual.y);
                glEnd();
            }
        }

        glDisable(GL_LINE_STIPPLE);
    }

    escrever(10, 30, "Pressione 1 para desenhar pontos, 2 para desenhar linhas e 3 para desenhar poligonos.");
    escrever(10, 15, "Clique com o mouse para desenhar. Clique com o botao direito para finalizar um poligono.");

    const char* ferramentas[] = {"", "Ponto", "Linha", "Poligono", "Selecao"};
    escrever(WIDTH - 40, 10, ferramentas[choice]);

    glutSwapBuffers();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenGL Example");

    printf("Pressione 1 para desenhar pontos, 2 para desenhar linhas e 3 para desenhar poligonos.\nClique com o mouse para desenhar. Clique com o botao direito para finalizar um poligono.\n");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(teclasEspeciais);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(motionPassiva);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}
