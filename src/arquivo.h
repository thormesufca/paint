#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "formas.h"
#include <string>
#include <vector>

void salvarArquivo(const std::vector<Forma*>& formas, const std::string& nomeArquivo);
std::vector<Forma*> carregarArquivo(const std::string& nomeArquivo);

#endif
