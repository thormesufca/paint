#include "arquivo.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

static void validarExtensao(const std::string& nomeArquivo) {
    if (nomeArquivo.size() < 5 || nomeArquivo.substr(nomeArquivo.size() - 5) != ".json")
        throw std::invalid_argument("O sistema só lê arquivos .json");
}

void salvarArquivo(const std::vector<Forma*>& formas, const std::string& nomeArquivo) {
    validarExtensao(nomeArquivo);

    std::ofstream arquivo(nomeArquivo);
    if (!arquivo.is_open())
        throw std::runtime_error("Nao foi possivel abrir o arquivo para escrita: " + nomeArquivo);

    std::ostringstream json;
    json << "[";
    for (int i = 0; i<formas.size(); i++)
    {
        json << formas[i]->serializar();
        if( i + 1 < formas.size()){
            json << ",";
        }
    }
    json << "]";
    arquivo << json.str();

    arquivo.close();
}

std::vector<Forma*> carregarArquivo(const std::string& nomeArquivo) {
    validarExtensao(nomeArquivo);

    std::ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open())
        throw std::runtime_error("Nao foi possivel abrir o arquivo para leitura: " + nomeArquivo);

    std::string conteudo((std::istreambuf_iterator<char>(arquivo)),
                          std::istreambuf_iterator<char>());
    arquivo.close();

    std::vector<Forma*> formas;

    // Extrai cada objeto {...} pelo nível de chaves
    int profundidade = 0;
    size_t inicio = 0;
    for (size_t i = 0; i < conteudo.size(); i++) {
        if (conteudo[i] == '{') {
            if (profundidade == 0) inicio = i;
            profundidade++;
        } else if (conteudo[i] == '}') {
            profundidade--;
            if (profundidade == 0) {
                std::string obj = conteudo.substr(inicio, i - inicio + 1);
                if      (obj.find("\"tipo\":\"ponto\"")   != std::string::npos)
                    formas.push_back(Ponto::desserializar(obj));
                else if (obj.find("\"tipo\":\"linha\"")   != std::string::npos)
                    formas.push_back(Linha::desserializar(obj));
                else if (obj.find("\"tipo\":\"poligono\"") != std::string::npos)
                    formas.push_back(Poligono::desserializar(obj));
            }
        }
    }

    return formas;
}
