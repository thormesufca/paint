#ifndef FORMAS_H
#define FORMAS_H

#include <vector>

struct Ponto2D {
    float x=-1.0f;
    float y=-1.0f;
};

class Forma {
protected:
    float cor[3];
    float lineWidth;
    
public:
    Forma();
    virtual ~Forma() = default;

    bool selecionada = false;
    void setCor(float r, float g, float b);
    void setLineWidth(float width);

    virtual void draw() const = 0;
};

class Ponto : public Forma {
private:
    Ponto2D ponto;

public:
    Ponto(float x, float y);

    void setPonto(float x, float y);
    Ponto2D getPonto() const { return ponto; }

    void draw() const override;
};

class Linha : public Forma {
private:
    Ponto2D p1;
    Ponto2D p2;

public:
    Linha(float x1, float y1, float x2, float y2);

    void setPontos(float x, float y, float x2, float y2);
    std::pair<Ponto2D, Ponto2D> getPontos() const { return {p1, p2}; }

    void draw() const override;
};

class Poligono : public Forma {
private:
    std::vector<Ponto2D> vertices;

public:
    void adicionarVertice(float x, float y);

    int getNumVertices() const { return vertices.size(); }
    std::vector<Ponto2D> getPontos() const { return vertices; }

    void draw() const override;
};

#endif