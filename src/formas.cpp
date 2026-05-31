#include "Formas.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

// ====================
// Forma
// ====================

Forma::Forma() {
    cor[0] = 1.0f;
    cor[1] = 1.0f;
    cor[2] = 1.0f;

    lineWidth = 1.0f;
}

void Forma::setCor(float r, float g, float b) {
    cor[0] = r;
    cor[1] = g;
    cor[2] = b;
}

void Forma::setLineWidth(float width) {
    lineWidth = width;
}

// ====================
// Ponto
// ====================

Ponto::Ponto(float x, float y) {
    ponto.x = x;
    ponto.y = y;
}

void Ponto::setPonto(float x, float y) {
    ponto.x = x;
    ponto.y = y;
}

void Ponto::draw() const {
    glColor3fv(cor);
    glPointSize(lineWidth);

    glBegin(GL_POINTS);
        glVertex2f(ponto.x, ponto.y);
    glEnd();
}

// ====================
// Linha
// ====================

Linha::Linha(float x1, float y1, float x2, float y2) {
    p1.x = x1;
    p1.y = y1;

    p2.x = x2;
    p2.y = y2;
}

void Linha::setPontos(float x1, float y1, float x2, float y2) {
    p1.x = x1;
    p1.y = y1;
    p2.x = x2;
    p2.y = y2;
}

void Linha::draw() const {
    glColor3fv(cor);
    glLineWidth(lineWidth);

    glBegin(GL_LINES);
        glVertex2f(p1.x, p1.y);
        glVertex2f(p2.x, p2.y);
    glEnd();
}

// ====================
// Poligono
// ====================

void Poligono::adicionarVertice(float x, float y) {
    vertices.push_back({x, y});
}

void Poligono::draw() const {
    if (vertices.size() < 3) return; // Um polígono precisa de pelo menos 3 vértices
    glColor3fv(cor);
    glLineWidth(lineWidth);

    glBegin(GL_POLYGON);
        for (const auto& v : vertices) {
            glVertex2f(v.x, v.y);
        }
    glEnd();
}