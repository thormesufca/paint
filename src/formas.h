#ifndef FORMAS_H
#define FORMAS_H

#include "matriz.h"
#include <string>
#include <vector>

struct Ponto2D
{
    float x = -1.0f;
    float y = -1.0f;
};

class Forma
{
protected:
    float cor[3];
    float lineWidth;

public:
    Forma();
    virtual ~Forma() = default;
    virtual std::string getTipo() = 0;
    virtual std::string serializar() = 0;
    bool selecionada = false;
    void setCor(float r, float g, float b);
    void setLineWidth(float width);
    const float* getCor() const { return cor; };
    float getLineWidth() const { return lineWidth; }
    virtual void rotacionar(float angulo);
    void transladar(float dx, float dy);
    void escalar(float sx, float sy);
    void aplicarTransformacaoComposta(Mat3 transformacao);
    void refletirHorizontal();
    void refletirVertical();
    void cisalharHorizontal(float shx);
    void cisalharVertical(float shy);
    virtual bool selecionar(float x, float y) = 0;
    virtual void draw() const = 0;
    virtual std::vector<Ponto2D> getPontos() const = 0;
    virtual void setPontos(std::vector<Ponto2D>) = 0;
};

class Ponto : public Forma
{
private:
    Ponto2D ponto;

public:
    Ponto(float x, float y);

    void setPonto(float x, float y);
    Ponto2D getPonto() const { return ponto; }

    void draw() const override;
    std::vector<Ponto2D> getPontos() const override { return {ponto}; }
    void setPontos(std::vector<Ponto2D>) override;
    void rotacionar(float angulo) override;
    bool selecionar(float x, float y) override;
    std::string getTipo() override { return "ponto"; };
    std::string serializar() override;
    static Ponto* desserializar(const std::string& json);
};

class Linha : public Forma
{
private:
    Ponto2D p1;
    Ponto2D p2;
    int analisaCasosTriviais(int codigo1, int codigo2);
    int getCodigoPonto(Ponto2D ponto, float x, float y, int tolerancia);

public:
    Linha(float x1, float y1, float x2, float y2);

    void setPontos(float x1, float y1, float x2, float y2);
    void setPontos(std::vector<Ponto2D>) override;
    void draw() const override;
    std::vector<Ponto2D> getPontos() const override { return {p1, p2}; }
    bool selecionar(float x, float y) override;
    std::string getTipo() override { return "linha"; };
    std::string serializar() override;
    static Linha* desserializar(const std::string& json);
};

class Poligono : public Forma
{
private:
    std::vector<Ponto2D> vertices;
    int analisaCasosTriviais(Ponto2D p1, Ponto2D p2, float x, float y);
    bool analisaCasosEspeciais(Ponto2D p1, Ponto2D p2, float x, float y);


public:
    void adicionarVertice(float x, float y);

    int getNumVertices() const { return vertices.size(); }
    Ponto2D getCentroide() const;

    void draw() const override;
    std::vector<Ponto2D> getPontos() const override { return vertices; }
    void setVertices(std::vector<Ponto2D>);
    void setPontos(std::vector<Ponto2D> pts) override { setVertices(pts); }
    bool selecionar(float x, float y) override;
    std::string getTipo() override { return "poligono"; };
    std::string serializar() override;
    static Poligono* desserializar(const std::string& json);
};

#endif
