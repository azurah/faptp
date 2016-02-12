/* 
 * File:   Output.cpp
 * Author: pedromazala
 * 
 * Created on January 1, 2016, 8:20 PM
 */

#include "Output.h"

Output::Output() {
}

Output::Output(const Output& orig) {
}

Output::~Output() {
}

std::string Output::getDir() {
    time_t current_time = time(NULL);
    std::string c_time_string = ctime(&current_time);
    c_time_string.erase(std::remove(c_time_string.begin(), c_time_string.end(), ' '), c_time_string.end());
    c_time_string.erase(std::remove(c_time_string.begin(), c_time_string.end(), '\n'), c_time_string.end());
    std::string dir = "output/" + c_time_string;

    return dir;
}

void Output::write(Solucao *pSolucao) {
    write(pSolucao, getDir());
}

void Output::write(Solucao *pSolucao, std::string savePath) {
    std::string command = "mkdir -p " + savePath;

    // Criando o diret�rio de sa�da
    int result = system(command.c_str());

    std::ostringstream saida{};
    const std::string diasDaSemana[] = {"Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado", "Domingo"};

    saida << std::nounitbuf;
    saida << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<head>\n"
            << "<style type=\"text/css\">\n"
            << "  table {\n"
            << "    width: 100%;\n"
            << "    border-collapse: collapse;\n"
            << "    margin: 10px 0 15px 0px;\n"
            << "  }\n"
            << "  table, td, th {\n"
            << "    border: 1px solid black;\n"
            << "  }\n"
            << "</style>\n"
            << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
            << "</head>\n"
            << "<body>\n";

    saida << "<hr /> <h3>Horario</h3>\n";
    for (int i = 0; i < pSolucao->camadasTamanho; i++) {
        saida << "<table align='center' class='horario'>\n";

        saida << "<tr>";
        for (int j = 0; j < SEMANA; j++) {
            saida << "<th>" << diasDaSemana[j] << "</th>";
        }
        saida << "</tr>";


        for (int j = 0; j < pSolucao->blocosTamanho; j++) {
            saida << "<tr>";
            for (int k = 0; k < SEMANA; k++) {
                auto pd = pSolucao->horario->at(k, j, i);
                std::string pds = "-";
                if (pd != NULL) {
                    pds = pd->getDisciplina()->getNome();
                }
                saida << "<td>" << pds << "</td>";
            }
            saida << "</tr>";
        }
        saida << "</table>\n";
    }

    saida << "<hr /> <h3>Grades</h3>\n";
    for (const auto& par : pSolucao->grades) {
        const auto gradeAtual = par.second;
        
        double fo = gradeAtual->getObjectiveFunction();
        
        if (fo == 0) {
            continue;
        }

        saida << "<table align='center' class='grade'>\n";

        saida << "<tr><th colspan=\"" << SEMANA << "\">" << gradeAtual->alunoPerfil->id << " (" << fo << ")</th></tr>\n";

        saida << "<tr>";
        for (int j = 0; j < SEMANA; j++) {
            saida << "<th>" << diasDaSemana[j] << "</th>";
        }
        saida << "</tr>";


        for (int j = 0; j < pSolucao->blocosTamanho; j++) {
            saida << "<tr>";
            for (int k = 0; k < SEMANA; k++) {
                auto pd = gradeAtual->at(k, j, 0);
                std::string pds = "-";
                if (pd != NULL) {
                    pds = pd->getDisciplina()->getNome();
                }
                saida << "<td>" << pds << "</td>";
            }
            saida << "</tr>";
        }

        /*
        const auto& discEscolhidas = gradeAtual->disciplinasAdicionadas;
        for (const auto disc : discEscolhidas) {
            std::cout << disc->nome << "; ";
        }
        */

        saida << "</table>\n";
    }

    saida << "</body>\n"
            << "</html>\n";

    std::ofstream arquivoSaida(savePath + "/horario.html");
    arquivoSaida << std::nounitbuf << saida.str() << std::endl;
    arquivoSaida.close();
}

