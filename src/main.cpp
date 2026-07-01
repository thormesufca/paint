#include <GL/glut.h>
#include <stdio.h>
#include "formas.h"
#include "matriz.h"
#include "funcoes.h"
#include "arquivo.h"
#include <vector>
#include <algorithm>
#include <fstream>

#define HEIGHT 900
#define WIDTH 1200
#define SPEED 2.0
#define TOOLBAR_WIDTH 80
#define MARGEM_LATERAL 5
#define ALTURA_ELEMENTO 35
#define MARGEM_TOPO 15
#define SLOT_BOTAO (ALTURA_ELEMENTO + 10) // Margem 10 pixels
#define MARGEM_BASE 15
#define SLOT_CORES (ALTURA_ELEMENTO + 10) // Margem 10 pixels
#define TEXTO_PAD_H 7
#define TEXTO_PAD_V 12
#define FECHAR_TOL 12 // distancia (px) para fechar poligono clicando no 1o vertice
#define BARRA_TOPO 30 // altura da barra de status superior

int winWidth = WIDTH;
int winHeight = HEIGHT;

int choice = 1;
std::vector<Forma *> formas;

bool animando = false;
std::vector<Ponto2D> velocidades;

std::string mensagemStatus = "";
bool exibirStatus = false;
bool statusSucesso = false;

bool modoTexto = false;
int acaoTexto = 0; // 1 = salvar, 2 = carregar
std::string nomeDigitado = "";

Ponto2D mouseAtual = {-1.0f, -1.0f};
Ponto2D linhaPrimeiroP = {-1.0f, -1.0f};
bool poligonoPrimeiroPonto = true;
std::vector<Ponto2D> verticesPoligonoTemp;
std::vector<Linha *> linhasPoligonoTemp;
bool arrastando = false;
bool esperandoArrasto = false;
float xArrastaInicio = 0.0f;
float yArrastaInicio = 0.0f;
Forma *formaSelecionada = nullptr;

float corAtual[3] = {1.0f, 1.0f, 1.0f};
int corSelecionada = 6;

bool mostrarAjuda = false;

const float CORES[7][3] = {
    {0.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
};

int init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    return 0;
}

// Distancia ao quadrado entre dois pontos (evita sqrt)
float distancia2(Ponto2D a, Ponto2D b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// Translada a forma limitando o deslocamento para que ela inteira (bounding box)
// permaneca dentro da area de desenho (fora da toolbar e da barra superior)
void transladarLimitado(Forma *forma, float dx, float dy)
{
    auto pontos = forma->getPontos();
    if (pontos.empty()) return;

    float minX = pontos[0].x, maxX = pontos[0].x;
    float minY = pontos[0].y, maxY = pontos[0].y;
    for (auto &p : pontos)
    {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    float xMin = TOOLBAR_WIDTH, xMax = winWidth;
    float yMin = 0, yMax = winHeight - BARRA_TOPO;

    if (minX + dx < xMin) dx = xMin - minX;
    if (maxX + dx > xMax) dx = xMax - maxX;
    if (minY + dy < yMin) dy = yMin - minY;
    if (maxY + dy > yMax) dy = yMax - maxY;

    forma->transladar(dx, dy);
}

void reshape(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, w, 0.0, h);
    glMatrixMode(GL_MODELVIEW);
}

void atualizarAnimacao(int)
{
    if (!animando) return;

    for (int i = 0; i < formas.size(); i++)
    {
        auto pontos = formas[i]->getPontos();
        float minX = pontos[0].x, maxX = pontos[0].x;
        float minY = pontos[0].y, maxY = pontos[0].y;
        for (auto &p : pontos)
        {
            if (p.x < minX) minX = p.x;
            if (p.x > maxX) maxX = p.x;
            if (p.y < minY) minY = p.y;
            if (p.y > maxY) maxY = p.y;
        }

        float &vx = velocidades[i].x;
        float &vy = velocidades[i].y;

        if (minX + vx < TOOLBAR_WIDTH) vx =  std::abs(vx);
        if (maxX + vx > winWidth)      vx = -std::abs(vx);
        if (minY + vy < 0)             vy =  std::abs(vy);
        if (maxY + vy > winHeight)     vy = -std::abs(vy);

        formas[i]->transladar(vx, vy);
        std::string tipo = formas[i]->getTipo();
        if(tipo != "ponto"){
            formas[i]->rotacionar(-5);
        }
    }

    glutPostRedisplay();
    glutTimerFunc(8, atualizarAnimacao, 0);
}

bool temPreview()
{
    return (choice == 2 && linhaPrimeiroP.x != -1.0f) || (choice == 3 && !poligonoPrimeiroPonto);
}

void desselecionar()
{
    for (auto &forma : formas)
        forma->selecionada = false;
    formaSelecionada = nullptr;
}

void mostrarStatus(const std::string &msg, bool sucesso)
{
    mensagemStatus = msg;
    statusSucesso = sucesso;
    exibirStatus = true;
    glutTimerFunc(3000, [](int)
                  { exibirStatus = false; glutPostRedisplay(); }, 0);
}

void salvarComNome(const std::string &nome)
{
    std::string arq = nome + ".json";
    try
    {
        salvarArquivo(formas, arq);
        mostrarStatus("Salvo em " + arq, true);
    }
    catch (const std::exception &e)
    {
        mostrarStatus(e.what(), false);
    }
}

void carregarComNome(const std::string &nome)
{
    std::string arq = nome + ".json";

    // Procura o arquivo no mesmo diretorio
    std::ifstream teste(arq.c_str());
    if (!teste.good())
    {
        mostrarStatus(arq + " nao existe", false);
        return;
    }
    teste.close();

    try
    {
        std::vector<Forma *> carregadas = carregarArquivo(arq);
        for (auto f : formas)
            delete f;
        formas = carregadas;
        formaSelecionada = nullptr;
        mostrarStatus("Carregado de " + arq, true);
    }
    catch (const std::exception &e)
    {
        mostrarStatus(e.what(), false);
    }
}

void keyboard(unsigned char key, int x, int y)
{
    // Modo de digitacao do nome do arquivo (Salvar/Carregar)
    if (modoTexto)
    {
        if (key == 13) // Enter confirma
        {
            if (!nomeDigitado.empty())
            {
                if (acaoTexto == 1)
                    salvarComNome(nomeDigitado);
                else
                    carregarComNome(nomeDigitado);
            }
            modoTexto = false;
            acaoTexto = 0;
            nomeDigitado.clear();
        }
        else if (key == 27) // Esc cancela
        {
            modoTexto = false;
            acaoTexto = 0;
            nomeDigitado.clear();
        }
        else if (key == 8 || key == 127) // Backspace
        {
            if (!nomeDigitado.empty())
                nomeDigitado.pop_back();
        }
        else
        {
            char c = (char)key;
            bool valido = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                          (c >= '0' && c <= '9') || c == '-' || c == '_';
            if (valido && nomeDigitado.size() < 40)
                nomeDigitado.push_back(c);
        }
        glutPostRedisplay();
        return;
    }

    switch (key)
    {
    case '?': // abre/fecha o painel de atalhos
        mostrarAjuda = !mostrarAjuda;
        break;
    case 32: // Espaço — toggle animação bounce
        animando = !animando;
        if (animando)
        {
            velocidades.clear();
            for (int i = 0; i < (int)formas.size(); i++)
            {
                float vx = (i % 3 + 1) * 3.5 * (i % 2 == 0 ? 1 : -1);
                float vy = (i % 4 + 1) * 3.0 * (i % 3 == 0 ? 1 : -1);
                velocidades.push_back({vx, vy});
            }
            glutTimerFunc(8, atualizarAnimacao, 0);
        }
        break;
    case '1':
        choice = 1;
        desselecionar();
        break;
    case '2':
        choice = 2;
        desselecionar();
        break;
    case '3':
        choice = 3;
        desselecionar();
        break;
    case '4':
        choice = 4;
        break;
    case 'H':
    case 'h':
        if (formaSelecionada)
            formaSelecionada->refletirHorizontal();
        break;
    case 'V':
    case 'v':
        if (formaSelecionada)
            formaSelecionada->refletirVertical();
        break;
    case 'Q':
    case 'q':
        if (formaSelecionada)
            formaSelecionada->rotacionar(15.0f);
        break;
    case 'E':
    case 'e':
        if (formaSelecionada)
            formaSelecionada->rotacionar(-15.0f);
        break;
    case 'C':
    case 'c':
        if (Poligono *poli = dynamic_cast<Poligono *>(formaSelecionada))
            poli->tornarConvexo();
        break;
    case 'z':
        if (formaSelecionada)
        {
            auto it = std::find(formas.begin(), formas.end(), formaSelecionada);
            if (it != formas.end())
                formas.erase(it);
            delete formaSelecionada;
            formaSelecionada = nullptr;
        }
        break;
    case 'I':
    case 'i':
        if (formaSelecionada)
            formaSelecionada->cisalharVertical(0.01f);
        break;
    case 'J':
    case 'j':
        if (formaSelecionada)
            formaSelecionada->cisalharHorizontal(-0.01f);
        break;
    case 'K':
    case 'k':
        if (formaSelecionada)
            formaSelecionada->cisalharVertical(-0.01f);
        break;
    case 'L':
    case 'l':
        if (formaSelecionada)
            formaSelecionada->cisalharHorizontal(0.01f);
        break;
    case 'w':
    case 'W':
        if (formaSelecionada)
            transladarLimitado(formaSelecionada, 0, 1.0f * SPEED);
        break;
    case 'a':
    case 'A':
        if (formaSelecionada)
            transladarLimitado(formaSelecionada, -1.0f * SPEED, 0);
        break;
    case 's':
    case 'S':
        if (formaSelecionada)
            transladarLimitado(formaSelecionada, 0, -1.0f * SPEED);
        break;
    case 'd':
    case 'D':
        if (formaSelecionada)
            transladarLimitado(formaSelecionada, 1.0f * SPEED, 0);
        break;
    case 127: // DELETE
        if (formaSelecionada)
        {
            auto it = std::find(formas.begin(), formas.end(), formaSelecionada);
            if (it != formas.end())
                formas.erase(it);
            delete formaSelecionada;
            formaSelecionada = nullptr;
        }
        break;
    case 27: // ESC
        desselecionar();
        break;
    case 43: // + Escalar para cima
        if (formaSelecionada)
        {
            formaSelecionada->escalar(1.05, 1.05);
        }
        break;
    case 45: // - Escalar para baixo
        if (formaSelecionada)
        {
            formaSelecionada->escalar(0.95, 0.95);
        }
        break;
    }
    glutPostRedisplay();
}

void teclasEspeciais(int key, int x, int y)
{
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
    if (formaSelecionada)
        transladarLimitado(formaSelecionada, dx, dy);
    glutPostRedisplay();
}

void addPonto(float x, float y)
{
    Ponto *p = new Ponto(x, y);
    p->setCor(corAtual[0], corAtual[1], corAtual[2]);
    p->setLineWidth(10.0);
    formas.push_back(p);
}

void addLinha(float x, float y, bool ativo)
{
    if (!ativo)
    {
        linhaPrimeiroP = {-1.0, -1.0};
        return;
    }

    if (linhaPrimeiroP.x == -1.0)
    {
        linhaPrimeiroP = {x, y};
    }
    else
    {
        Linha *l = new Linha(linhaPrimeiroP.x, linhaPrimeiroP.y, x, y);
        l->setCor(corAtual[0], corAtual[1], corAtual[2]);
        l->setLineWidth(2.0);
        formas.push_back(l);
        linhaPrimeiroP = {-1.0, -1.0};
    }
}

void addPoligono(float x, float y, bool ativo)
{
    if (!ativo)
    {
        if (verticesPoligonoTemp.size() >= 3)
        {
            for (Linha *l : linhasPoligonoTemp)
            {
                auto it = std::find(formas.begin(), formas.end(), (Forma *)l);
                if (it != formas.end())
                    formas.erase(it);
                delete l;
            }
            Poligono *p = new Poligono();
            p->setCor(corAtual[0], corAtual[1], corAtual[2]);
            p->setLineWidth(2.0);
            for (auto &v : verticesPoligonoTemp)
                p->adicionarVertice(v.x, v.y);
            formas.push_back(p);
        }
        else
        {
            for (Linha *l : linhasPoligonoTemp)
            {
                auto it = std::find(formas.begin(), formas.end(), (Forma *)l);
                if (it != formas.end())
                    formas.erase(it);
                delete l;
            }
        }
        linhasPoligonoTemp.clear();
        verticesPoligonoTemp.clear();
        poligonoPrimeiroPonto = true;
        return;
    }

    if (poligonoPrimeiroPonto)
    {
        verticesPoligonoTemp.push_back({x, y});
        poligonoPrimeiroPonto = false;
    }
    else
    {
        Ponto2D last = verticesPoligonoTemp.back();
        Linha *l = new Linha(last.x, last.y, x, y);
        l->setCor(corAtual[0], corAtual[1], corAtual[2]);
        l->setLineWidth(2.0);
        formas.push_back(l);
        linhasPoligonoTemp.push_back(l);
        verticesPoligonoTemp.push_back({x, y});
    }
}

void motionPassiva(int x, int y)
{
    mouseAtual = {(float)x, (float)(winHeight - y)};
    if (temPreview())
        glutPostRedisplay();
}

void motion(int x, int y)
{
    float fx = (float)x;
    float fy = (float)(winHeight - y);
    mouseAtual = {fx, fy};

    if (arrastando)
    {
        float dx = fx - xArrastaInicio;
        float dy = fy - yArrastaInicio;
        transladarLimitado(formaSelecionada, dx, dy);
        xArrastaInicio = fx;
        yArrastaInicio = fy;
        glutPostRedisplay();
    }
    if (temPreview())
    glutPostRedisplay();
}

void verificarArrasto(int)
{
    if (esperandoArrasto) {
        esperandoArrasto = false;
        arrastando = true;
        // Sincroniza início do arrasto com posição atual do mouse
        xArrastaInicio = mouseAtual.x;
        yArrastaInicio = mouseAtual.y;
    }
}

void mouse(int button, int state, int x, int y)
{
    if (animando) return;
    if (modoTexto) return;
    float fx = (float)x;
    float fy = (float)(winHeight - y);

    if (fx < TOOLBAR_WIDTH)
    { // Verifica se o clique foi na barra de ferramentas
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            // Botões de ferramenta
            for (int i = 0; i < 4; i++)
            {
                float yTop = winHeight - MARGEM_TOPO - i * SLOT_BOTAO;
                float yBot = yTop - ALTURA_ELEMENTO;
                if (fy >= yBot && fy <= yTop)
                {
                    addLinha(0, 0, false);
                    addPoligono(0, 0, false);
                    choice = i + 1;
                    if (choice != 4)
                        desselecionar();
                    glutPostRedisplay();
                    return;
                }
            }
            // Botão Salvar
            {
                float yTop = winHeight - MARGEM_TOPO - 4 * SLOT_BOTAO;
                float yBot = yTop - ALTURA_ELEMENTO;
                if (fy >= yBot && fy <= yTop)
                {
                    modoTexto = true;
                    acaoTexto = 1; // salvar
                    nomeDigitado.clear();
                    glutPostRedisplay();
                    return;
                }
            }
            // Botão Carregar
            {
                float yTop = winHeight - MARGEM_TOPO - 5 * SLOT_BOTAO;
                float yBot = yTop - ALTURA_ELEMENTO;
                if (fy >= yBot && fy <= yTop)
                {
                    modoTexto = true;
                    acaoTexto = 2; // carregar
                    nomeDigitado.clear();
                    glutPostRedisplay();
                    return;
                }
            }
            // Quadros de cor
            for (int i = 0; i < 7; i++)
            {
                float yBot = MARGEM_BASE + i * SLOT_CORES;
                float yTop = yBot + ALTURA_ELEMENTO;
                if (fy >= yBot && fy <= yTop)
                {
                    corSelecionada = i;
                    corAtual[0] = CORES[i][0];
                    corAtual[1] = CORES[i][1];
                    corAtual[2] = CORES[i][2];
                    glutPostRedisplay();
                    return;
                }
            }
        }
        return;
    }

    // Ignora cliques na barra de status superior
    if (fy > winHeight - BARRA_TOPO)
        return;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            bool temSelecionada = formaSelecionada != nullptr;

            if (temSelecionada)
            {
                // Aguarda 200ms antes de ativar o arrasto
                esperandoArrasto = true;
                xArrastaInicio = fx;
                yArrastaInicio = fy;
                glutTimerFunc(200, verificarArrasto, 0);
            }
            else
            {
                switch (choice)
                {
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
                    // Se ja ha >=3 vertices e o clique e perto do 1o, fecha o poligono
                    if (!poligonoPrimeiroPonto && verticesPoligonoTemp.size() >= 3 &&
                        distancia2(verticesPoligonoTemp[0], {fx, fy}) <= FECHAR_TOL * FECHAR_TOL)
                        addPoligono(0, 0, false);
                    else
                        addPoligono(fx, fy, true);
                    break;
                case 4:
                    for (auto &forma : formas)
                        forma->selecionada = false;
                    formaSelecionada = nullptr;
                    for (auto &forma : formas)
                    {
                        if (forma->selecionar(fx, fy))
                        {
                            formaSelecionada = forma;
                            break;
                        }
                    }
                }
            }
        }
        else if (state == GLUT_UP)
        {
            if (esperandoArrasto)
            {
                // Click curto (< 200ms): tratar como click normal
                esperandoArrasto = false;
                switch (choice)
                {
                case 1:
                    addLinha(0, 0, false);
                    addPoligono(0, 0, false);
                    addPonto(xArrastaInicio, yArrastaInicio);
                    break;
                case 2:
                    addPoligono(0, 0, false);
                    addLinha(xArrastaInicio, yArrastaInicio, true);
                    break;
                case 3:
                    addLinha(0, 0, false);
                    if (!poligonoPrimeiroPonto && verticesPoligonoTemp.size() >= 3 &&
                        distancia2(verticesPoligonoTemp[0], {xArrastaInicio, yArrastaInicio}) <= FECHAR_TOL * FECHAR_TOL)
                        addPoligono(0, 0, false);
                    else
                        addPoligono(xArrastaInicio, yArrastaInicio, true);
                    break;
                case 4:
                    for (auto &forma : formas)
                        forma->selecionada = false;
                    formaSelecionada = nullptr;
                    for (auto &forma : formas)
                    {
                        if (forma->selecionar(xArrastaInicio, yArrastaInicio))
                        {
                            formaSelecionada = forma;
                            break;
                        }
                    }
                    break;
                }
            }
            arrastando = false;
        }
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        addLinha(0, 0, false);
        addPoligono(0, 0, false);
        if (formaSelecionada)
        {
            for (auto &forma : formas)
                forma->selecionada = false;
            formaSelecionada = nullptr;
        }
    }
    glutPostRedisplay();
}

void desenharToolbar()
{
    // Fundo
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(TOOLBAR_WIDTH, 0);
    glVertex2f(TOOLBAR_WIDTH, winHeight);
    glVertex2f(0, winHeight);
    glEnd();

    // Separador
    glColor3f(1, 1, 1);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2f(TOOLBAR_WIDTH, 0);
    glVertex2f(TOOLBAR_WIDTH, winHeight);
    glEnd();

    // Botões de ferramenta
    const char *nomes[] = {"Ponto", "Linha", "Poligono", "Selecionar"};
    for (int i = 0; i < 4; i++)
    {
        float yTop = winHeight - MARGEM_TOPO - i * SLOT_BOTAO;
        float yBot = yTop - ALTURA_ELEMENTO;
        if (choice == i + 1)
            glColor3f(0.3, 0.3, 0.75);
        else
            glColor3f(0.28, 0.28, 0.28);
        glBegin(GL_QUADS);
        glVertex2f(MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yTop);
        glVertex2f(MARGEM_LATERAL, yTop);
        glEnd();
        escrever(MARGEM_LATERAL + TEXTO_PAD_H, (int)(yBot + TEXTO_PAD_V), nomes[i]);
    }

    // Botão Salvar
    {
        float yTop = winHeight - MARGEM_TOPO - 4 * SLOT_BOTAO;
        float yBot = yTop - ALTURA_ELEMENTO;
        glColor3f(0.2, 0.45, 0.2);
        glBegin(GL_QUADS);
        glVertex2f(MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yTop);
        glVertex2f(MARGEM_LATERAL, yTop);
        glEnd();
        escrever(MARGEM_LATERAL + TEXTO_PAD_H, (int)(yBot + TEXTO_PAD_V), "Salvar");
    }

    // Botão Carregar
    {
        float yTop = winHeight - MARGEM_TOPO - 5 * SLOT_BOTAO;
        float yBot = yTop - ALTURA_ELEMENTO;
        glColor3f(0.2, 0.35, 0.5);
        glBegin(GL_QUADS);
        glVertex2f(MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yTop);
        glVertex2f(MARGEM_LATERAL, yTop);
        glEnd();
        escrever(MARGEM_LATERAL + TEXTO_PAD_H, (int)(yBot + TEXTO_PAD_V), "Carregar");
    }

    // Quadros de cor
    for (int i = 0; i < 7; i++)
    {
        float yBot = MARGEM_BASE + i * SLOT_CORES;
        float yTop = yBot + ALTURA_ELEMENTO;
        glColor3fv(CORES[i]);
        glBegin(GL_QUADS);
        glVertex2f(MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yBot);
        glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yTop);
        glVertex2f(MARGEM_LATERAL, yTop);
        glEnd();
        if (corSelecionada == i)
        {
            glColor3f(0.5, 0.5, 0.5);
            glLineWidth(2.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(MARGEM_LATERAL, yBot);
            glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yBot);
            glVertex2f(TOOLBAR_WIDTH - MARGEM_LATERAL, yTop);
            glVertex2f(MARGEM_LATERAL, yTop);
            glEnd();
        }
    }
}

void desenharBarraSuperior()
{
    float x0 = TOOLBAR_WIDTH;
    float y0 = winHeight - BARRA_TOPO;

    // Fundo da barra
    glColor3f(0.13f, 0.13f, 0.13f);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(winWidth, y0);
    glVertex2f(winWidth, winHeight);
    glVertex2f(x0, winHeight);
    glEnd();

    // Linha separadora inferior
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(winWidth, y0);
    glEnd();

    int yTexto = (int)(y0 + 10);

    // Ferramenta atual
    const char *nomes[] = {"Ponto", "Linha", "Poligono", "Selecionar"};
    std::string ferramenta = std::string("Ferramenta: ") + nomes[choice - 1];
    escrever((int)x0 + 12, yTexto, ferramenta.c_str());

    // Indicador de cor atual
    float corX = x0 + 220;
    escrever((int)corX, yTexto, "Cor:");
    float sx = corX + 35, sw = 22, sh = 16, sy = y0 + 7;
    glColor3fv(corAtual);
    glBegin(GL_QUADS);
    glVertex2f(sx, sy);
    glVertex2f(sx + sw, sy);
    glVertex2f(sx + sw, sy + sh);
    glVertex2f(sx, sy + sh);
    glEnd();
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(sx, sy);
    glVertex2f(sx + sw, sy);
    glVertex2f(sx + sw, sy + sh);
    glVertex2f(sx, sy + sh);
    glEnd();

    // Dica contextual conforme a ferramenta
    const char *dica;
    switch (choice)
    {
    case 1: dica = "Clique para criar um ponto"; break;
    case 2: dica = "Clique em 2 pontos para criar uma linha"; break;
    case 3: dica = "Clique p/ adicionar vertices | dir. ou 1o vertice = fechar"; break;
    default: dica = "Clique p/ selecionar | arraste p/ mover | Del exclui"; break;
    }
    escrever((int)(sx + sw + 25), yTexto, dica);

    // Atalho de ajuda no canto direito
    escrever(winWidth - 95, yTexto, "?  =  ajuda");
}

void desenharPainelAjuda()
{
    const char *linhas[] = {
        "ATALHOS DO TECLADO",
        "",
        "1 / 2 / 3 / 4    Ponto / Linha / Poligono / Selecionar",
        "Q / E            Rotacionar",
        "+ / -            Escalar",
        "H / V            Refletir horizontal / vertical",
        "I / J / K / L    Cisalhar",
        "C                Fecho convexo (poligono selecionado)",
        "W A S D / setas  Mover forma selecionada",
        "Delete / z       Excluir forma selecionada",
        "Espaco           Animacao",
        "Clique direito   Fechar poligono / desmarcar",
        "ESC              Desmarcar selecao",
        "",
        "?                Fecha esta ajuda",
    };
    int n = sizeof(linhas) / sizeof(linhas[0]);

    float lineH = 22, pad = 22, bw = 540;
    float bh = n * lineH + 2 * pad;
    float cx = (TOOLBAR_WIDTH + winWidth) / 2.0f;
    float cy = winHeight / 2.0f;
    float x0 = cx - bw / 2, y0 = cy - bh / 2;

    // Escurece o fundo para destacar o painel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(TOOLBAR_WIDTH, 0);
    glVertex2f(winWidth, 0);
    glVertex2f(winWidth, winHeight);
    glVertex2f(TOOLBAR_WIDTH, winHeight);
    glEnd();
    glDisable(GL_BLEND);

    // Caixa do painel
    glColor3f(0.1f, 0.1f, 0.16f);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x0 + bw, y0);
    glVertex2f(x0 + bw, y0 + bh);
    glVertex2f(x0, y0 + bh);
    glEnd();
    glColor3f(0.5f, 0.5f, 0.75f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0);
    glVertex2f(x0 + bw, y0);
    glVertex2f(x0 + bw, y0 + bh);
    glVertex2f(x0, y0 + bh);
    glEnd();

    // Linhas de texto (de cima para baixo)
    for (int i = 0; i < n; i++)
        escrever((int)(x0 + pad), (int)(y0 + bh - pad - 14 - i * lineH), linhas[i]);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    desenharToolbar();

    for (const auto &forma : formas)
    {
        if (!forma->selecionada)
            forma->draw();
    }

    for (const auto &forma : formas)
    {
        if (forma->selecionada)
        {
            forma->draw();
        }
    }

    // Barra de status superior (ferramenta/cor/dica) por cima das formas
    desenharBarraSuperior();

    // preview rubber-band
    if (mouseAtual.x != -1.0f)
    {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0xAAAA);
        glLineWidth(1.0f);

        if (choice == 2 && linhaPrimeiroP.x != -1.0f)
        {
            glColor3fv(corAtual);
            glBegin(GL_LINES);
            glVertex2f(linhaPrimeiroP.x, linhaPrimeiroP.y);
            glVertex2f(mouseAtual.x, mouseAtual.y);
            glEnd();
        }

        if (choice == 3 && !poligonoPrimeiroPonto && !verticesPoligonoTemp.empty())
        {
            glColor3fv(corAtual);
            glBegin(GL_LINES);
            glVertex2f(verticesPoligonoTemp.back().x, verticesPoligonoTemp.back().y);
            glVertex2f(mouseAtual.x, mouseAtual.y);
            glEnd();
        }

        glDisable(GL_LINE_STIPPLE);
    }

    // Marcador no 1o vertice do poligono (clique aqui para fechar)
    if (choice == 3 && verticesPoligonoTemp.size() >= 3)
    {
        Ponto2D pv = verticesPoligonoTemp[0];
        bool perto = mouseAtual.x != -1.0f &&
                     distancia2(pv, mouseAtual) <= FECHAR_TOL * FECHAR_TOL;
        float r = perto ? 8.0f : 5.0f;
        if (perto)
            glColor3f(0.2f, 1.0f, 0.4f); // verde: pronto para fechar
        else
            glColor3f(1.0f, 0.85f, 0.2f); // amarelo: ponto de fechamento
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(pv.x - r, pv.y - r);
        glVertex2f(pv.x + r, pv.y - r);
        glVertex2f(pv.x + r, pv.y + r);
        glVertex2f(pv.x - r, pv.y + r);
        glEnd();
    }

    if (exibirStatus)
    {
        float centerx = (TOOLBAR_WIDTH + winWidth) / 2.0;
        float by = 10.0, bh = 25.0, bw = 300.0;
        if (statusSucesso)
            glColor3f(0.1f, 0.45f, 0.1f);
        else
            glColor3f(0.5f, 0.1f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(centerx - bw / 2, by);
        glVertex2f(centerx + bw / 2, by);
        glVertex2f(centerx + bw / 2, by + bh);
        glVertex2f(centerx - bw / 2, by + bh);
        glEnd();
        escrever((int)(centerx - bw / 2 + 8), (int)(by + 8), mensagemStatus.c_str());
    }

    if (modoTexto)
    {
        float centerx = (TOOLBAR_WIDTH + winWidth) / 2.0;
        float bw = 460.0, bh = 50.0;
        float by = winHeight - 70.0;
        glColor3f(0.15f, 0.15f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(centerx - bw / 2, by);
        glVertex2f(centerx + bw / 2, by);
        glVertex2f(centerx + bw / 2, by + bh);
        glVertex2f(centerx - bw / 2, by + bh);
        glEnd();

        std::string label = (acaoTexto == 1) ? "Salvar como: " : "Carregar: ";
        std::string linha = label + nomeDigitado + ".json";
        escrever((int)(centerx - bw / 2 + 10), (int)(by + bh - 18), linha.c_str());
        escrever((int)(centerx - bw / 2 + 10), (int)(by + 10), "Enter = confirmar   Esc = cancelar");
    }

    if (mostrarAjuda)
        desenharPainelAjuda();

    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("UFCA - Computacao Grafica - Paint");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(teclasEspeciais);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(motionPassiva);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}
