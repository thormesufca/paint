#include "formas.h"
#include "matriz.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <math.h>
#include <stdio.h>
#include <sstream>

#define COR_SELECAO 0.5, 0.5, 0.5
#define PI 3.14159265359

const int ESQ = 0b1000;
const int DIR = 0b0100;
const int CIMA = 0b0010;
const int BAIXO = 0b0001;

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

void Forma::aplicarTransformacaoComposta(Mat3 matrizTransformacao)
{
    auto pontos = getPontos();
    if (pontos.size() <= 1)
        return;

    Ponto2D c;
    c.x = 0;
    c.y = 0;
    for (const auto &pt : pontos)
    {
        c.x += pt.x;
        c.y += pt.y;
    }
    c.x /= pontos.size();
    c.y /= pontos.size();

    Mat3 T_ida = {{1, 0, -c.x},
                  {0, 1, -c.y},
                  {0, 0, 1}};

    Mat3 T_volta = {{1, 0, c.x},
                    {0, 1, c.y},
                    {0, 0, 1}};

    Mat3 temp, M;
    multiplicaMatriz(temp, matrizTransformacao, T_ida);
    multiplicaMatriz(M, T_volta, temp);

    for (auto &pt : pontos)
    {
        float x = M[0][0] * pt.x + M[0][1] * pt.y + M[0][2];
        float y = M[1][0] * pt.x + M[1][1] * pt.y + M[1][2];
        pt.x = x;
        pt.y = y;
    }
    setPontos(pontos);
}

void Forma::rotacionar(float angulo)
{

    float rad = angulo * PI / 180.0f;
    float cosA = cos(rad), sinA = sin(rad);

    Mat3 R = {{cosA, -sinA, 0},
              {sinA, cosA, 0},
              {0, 0, 1}};
    aplicarTransformacaoComposta(R);
}

void Forma::transladar(float dx, float dy)
{
    auto pontos = getPontos();
    for (auto &pt : pontos)
    {
        pt.x += dx;
        pt.y += dy;
    }
    setPontos(pontos);
}

void Forma::escalar(float sx, float sy)
{
    Mat3 S = {{sx, 0, 0},
              {0, sy, 0},
              {0, 0, 1}};

    aplicarTransformacaoComposta(S);
}


void Forma::refletirHorizontal()
{
    Mat3 R = {{-1, 0, 0},
              {0, 1, 0},
              {0, 0, 1}};
    aplicarTransformacaoComposta(R);
}

void Forma::refletirVertical()
{
    Mat3 R = {{1, 0, 0},
              {0, -1, 0},
              {0, 0, 1}};
    aplicarTransformacaoComposta(R);
}

void Forma::cisalharHorizontal(float shx)
{
    Mat3 C = {{1, shx, 0},
              {0, 1, 0},
              {0, 0, 1}};
    aplicarTransformacaoComposta(C);
}

void Forma::cisalharVertical(float shy)
{
    Mat3 C = {{1, 0, 0},
              {shy, 1, 0},
              {0, 0, 1}};
    aplicarTransformacaoComposta(C);
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

void Ponto::setPontos(std::vector<Ponto2D> pts) {
    ponto = pts[0];
}

void Ponto::draw() const {
    if (selecionada) glColor3f(COR_SELECAO);
    else glColor3fv(cor);
    
    glPointSize(lineWidth);

    glBegin(GL_POINTS);
        glVertex2f(ponto.x, ponto.y);
    glEnd();
}

Ponto* Ponto::desserializar(const std::string& json) {
    float r, g, b, lw, x, y;
    sscanf(json.c_str(),
        "{\"tipo\":\"ponto\",\"cor\":[%f,%f,%f],\"lineWidth\":%f,\"x\":%f,\"y\":%f}",
        &r, &g, &b, &lw, &x, &y);
    Ponto* p = new Ponto(x, y);
    p->setCor(r, g, b);
    p->setLineWidth(lw);
    return p;
}

std::string Ponto::serializar(){
    std::ostringstream obj;
    obj << "{\"tipo\":\"ponto\",\"cor\":[";
    obj << cor[0];
    obj << ",";
    obj << cor[1];
    obj << ",";
    obj << cor[2];
    obj << "],\"lineWidth\":";
    obj << lineWidth;
    obj << ",\"x\":";
    obj << ponto.x;
    obj << ",\"y\":";
    obj << ponto.y;
    obj << "}";
    return obj.str();
}

void Ponto::rotacionar(float angulo){
    float rad = angulo * PI / 180.0f;
    float cosA = cos(rad), sinA = sin(rad);
    Ponto2D p = getPonto();

    Mat3 R = {{cosA, -sinA, 0},
              {sinA, cosA, 0},
              {0, 0, 1}};
    float x = R[0][0] * p.x + R[0][1] * p.y + R[0][2];
    float y = R[1][0] * p.x + R[1][1] * p.y + R[1][2];
    setPonto(x, y);
}

bool Ponto::selecionar(float x, float y) {
    int tolerancia = 5;
    if(x <= ponto.x + tolerancia && x >= ponto.x - tolerancia &&
       y <= ponto.y + tolerancia && y >= ponto.y - tolerancia){
        selecionada = true;
        return true;
    }
    return false;
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
    if (selecionada) glColor3f(COR_SELECAO);
    else glColor3fv(cor);
    
    glLineWidth(lineWidth);

    glBegin(GL_LINES);
        glVertex2f(p1.x, p1.y);
        glVertex2f(p2.x, p2.y);
    glEnd();
}

void Linha::setPontos(std::vector<Ponto2D> pts) {
    p1 = pts[0];
    p2 = pts[1];
}

bool Linha::selecionar(float x, float y){
    int tolerancia = 5;
    Ponto2D p1 = getPontos()[0];
    Ponto2D p2 = getPontos()[1];
    int codigoP1 = getCodigoPonto(p1, x, y, tolerancia);
    int codigoP2 = getCodigoPonto(p2, x, y, tolerancia);
    int resultado;
    while ((resultado = analisaCasosTriviais(codigoP1, codigoP2)) == 2){//Indefinido
        if(codigoP1 & ESQ){
            p1.y = p1.y + (p2.y - p1.y) * ((x - tolerancia) - p1.x) / (p2.x - p1.x);
            p1.x = x - tolerancia;
        }
        else if(codigoP1 & DIR){
            p1.y = p1.y + (p2.y - p1.y) * ((x + tolerancia) - p1.x) / (p2.x - p1.x);
            p1.x = x + tolerancia;
        }
        else if(codigoP1 & BAIXO){
            p1.x = p1.x + (p2.x - p1.x) * ((y - tolerancia) - p1.y) / (p2.y - p1.y);
            p1.y = y - tolerancia;
        }
        else { //CIMA
            p1.x = p1.x + (p2.x - p1.x) * ((y + tolerancia) - p1.y) / (p2.y - p1.y);
            p1.y = y + tolerancia;
        }
        codigoP1 = getCodigoPonto(p1, x, y, tolerancia);
    }
    if(resultado == 1){
        selecionada = true;
        return true;
    }
    return false;
}

// 0 - Trivial Negativo
// 1 - Trivial Positivo
// 2 - Indefinido
int Linha::analisaCasosTriviais(int codigo1, int codigo2){
    if (codigo1 == 0 || codigo2 == 0)
    { // 0000 em algum ponto - Caso trivial positivo
        return 1;
    }
    if ((codigo1 & codigo2) > 0)
    { // AND tem algum 1 - Caso trivial negativo
        return 0;
    }
    return 2;
}

int Linha::getCodigoPonto(Ponto2D ponto, float x, float y, int tolerancia){
    int codigo = 0;
    if (ponto.x < x - tolerancia)
        codigo |= ESQ;
    else if (ponto.x > x + tolerancia)
        codigo |= DIR;
    if (ponto.y < y - tolerancia)
        codigo |= BAIXO;
    else if (ponto.y > y + tolerancia)
        codigo |= CIMA;

    return codigo;
}

Linha* Linha::desserializar(const std::string& json) {
    float r, g, b, lw, x1, y1, x2, y2;
    sscanf(json.c_str(),
        "{\"tipo\":\"linha\",\"cor\":[%f,%f,%f],\"lineWidth\":%f,\"p1\":[%f,%f],\"p2\":[%f,%f]}",
        &r, &g, &b, &lw, &x1, &y1, &x2, &y2);
    Linha* l = new Linha(x1, y1, x2, y2);
    l->setCor(r, g, b);
    l->setLineWidth(lw);
    return l;
}

std::string Linha::serializar(){
    std::ostringstream obj;
    obj << "{\"tipo\":\"linha\",\"cor\":[";
    obj << cor[0];
    obj << ",";
    obj << cor[1];
    obj << ",";
    obj << cor[2];
    obj << "],\"lineWidth\":";
    obj << lineWidth;
    obj << ",\"p1\":[";
    obj << p1.x;
    obj << ",";
    obj << p1.y;
    obj << "],\"p2\":[";
    obj << p2.x;
    obj << ",";
    obj << p2.y;
    obj << "]";
    obj << "}";
    return obj.str();
}

// ====================
// Poligono
// ====================

void Poligono::adicionarVertice(float x, float y) {
    vertices.push_back({x, y});
}

void Poligono::draw() const {
    if (vertices.size() < 3) return; // Um polígono precisa de pelo menos 3 vértices

    if (selecionada) glColor3f(COR_SELECAO);
    else glColor3fv(cor);

    glLineWidth(lineWidth);

    glBegin(GL_LINE_LOOP);
        for (const auto& v : vertices) {
            glVertex2f(v.x, v.y);
        }
    glEnd();
}

void Poligono::setVertices(std::vector<Ponto2D> pontos){
    vertices.clear();
    for(auto &pt: pontos){
        vertices.push_back({pt.x, pt.y});
    }
}

bool Poligono::selecionar(float x, float y)
{
    //Lista todas as arestas
    std::vector<std::pair<Ponto2D, Ponto2D>> arestas;
    std::vector<std::pair<Ponto2D, Ponto2D>> arestasEspeciais;
    for(int i = 0; i < vertices.size(); i++){
        Ponto2D a = vertices[i];
        Ponto2D b = vertices[(i + 1) % vertices.size()];
        arestas.push_back({a, b});
    }
    int intersecoes = 0;
    //Analisar os casos triviais
    for (int i =0; i < arestas.size(); i++){
        Ponto2D a = arestas[i].first;
        Ponto2D b = arestas[i].second;
        int resultado = analisaCasosTriviais(a, b, x, y);
        if(resultado == 1) {
            intersecoes += 1;
        }
        if(resultado == 2){ // Indefinido, vai para casos especiais
            arestasEspeciais.push_back(arestas[i]);
        }
    }

    // Se ainda sobraram arestas, rodar casos especiais
    for (int i = 0; i < arestasEspeciais.size(); i++)
    {
        Ponto2D a = arestasEspeciais[i].first;
        Ponto2D b = arestasEspeciais[i].second;
        bool resultado2 = analisaCasosEspeciais(a, b, x, y);
        if (resultado2)
            intersecoes += 1;
        
    }

    if (intersecoes % 2 != 0)
    {
        selecionada = true;
        return true;
    }
    else
    {
        return false;
    }
}

// 0 - Não intercepta - Exclui
// 1 - Intercepta
// 2 - Indefinido
int Poligono::analisaCasosTriviais(Ponto2D p1, Ponto2D p2, float x, float y){
    if(p1.y > y && p2.y > y) // Acima
        return 0;
    
    if(p1.y < y && p2.y < y) // Abaixo
        return 0;

    if(p1.x < x && p2.x < x) //Esquerda
        return 0;
        // À Direita              P1 acima e P2 abaixo   ou   P1 abaixo e P2 Acima
    if((p1.x > x && p2.x > x) && ((p1.y > y && p2.y < y) || (p1.y < y && p2.y > y))) // À direita uma acima e outra abaixo
        return 1;

    return 2;
}

bool Poligono::analisaCasosEspeciais(Ponto2D p1, Ponto2D p2, float x, float y){
    if(p1.y == y && p2.y == y) // arestas alinhadas com o tiro, descarta
        return false;
    if(p1.y == y || p2.y == y){ // tiro passa por um dos vértices
        if((p1.y == y && p2.y > y) || (p2.y == y && p1.y > y)){// Um vértice no tiro e outro acima  - conta
            return true;
        }
        else{ // Senão - Um vértice no tiro e outro abaixo - não conta
            return false;
        }
    }
    
    float xi = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y); // Calcula x da interseção
    if(xi > x){
        return true;
    } else{
        return false;
    }
}



Ponto2D Poligono::getCentroide() const {
    Ponto2D c;
    c.x = 0; c.y = 0;
    for (const auto& pt : vertices) {
        c.x += pt.x;
        c.y += pt.y;
    }
    c.x /= vertices.size();
    c.y /= vertices.size();
    return c;
}

Poligono* Poligono::desserializar(const std::string& json) {
    float r, g, b, lw;
    sscanf(json.c_str(),
        "{\"tipo\":\"poligono\",\"cor\":[%f,%f,%f],\"lineWidth\":%f",
        &r, &g, &b, &lw);
    Poligono* p = new Poligono();
    p->setCor(r, g, b);
    p->setLineWidth(lw);
    size_t pos = json.find("\"vertices\":[") + 12;
    while (pos < json.size() && json[pos] != ']') {
        if (json[pos] == '[') {
            float x, y;
            sscanf(json.c_str() + pos, "[%f,%f]", &x, &y);
            p->adicionarVertice(x, y);
            pos = json.find(']', pos) + 1;
        } else {
            pos++;
        }
    }
    return p;
}

std::string Poligono::serializar(){
    std::ostringstream obj;
    obj << "{\"tipo\":\"poligono\",\"cor\":[";
    obj << cor[0];
    obj << ",";
    obj << cor[1];
    obj << ",";
    obj << cor[2];
    obj << "],\"lineWidth\":";
    obj << lineWidth;
    obj << ",\"vertices\":[";
    for (int i = 0; i < vertices.size(); i++)
        {
            Ponto2D v = vertices[i];
            obj << "[";
            obj << v.x;
            obj << ",";
            obj << v.y;
            obj << "]";
            if (i + 1 < vertices.size())
            {
                obj << ",";
            }
        }
    obj << "]";
    obj << "}";
    return obj.str();
}

