#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ios>
#include <string>
#include <chrono>
#include <boost/format.hpp>
#include <faptp/Resolucao.h>
#include <faptp/Output.h>
#include <faptp/Configuracao.h>
#include <faptp/Util.h>
#include <faptp/Timer.h>

/*
 * PAR�METROS DE CONFIGURA��O *
 * input.all.json ->
 *      camadasTamanho : 33
 *      perfilTamanho : 1392
 * input.json (impreciso, mas funciona) ->
 *      camadasTamaho : 4
 *      perfilTamanho : 10
 */

struct Entrada
{
    int camadasTamanho;
    int perfilTamanho;
};

constexpr Entrada input_all_json{33, 1392};
constexpr Entrada input_json{4, 10};

void semArgumentos()
{
    Resolucao r {Configuracao()
            .arquivoEntrada(Util::join_path({"entradas"}, "input.all.json"))
            .populacaoInicial(20)
            .porcentagemCruzamentos(30) // %
            .numMaximoIteracoesSemEvolucaoGRASP(15)
            .numMaximoIteracoesSemEvolucaoAG(20)
            .tipoCruzamento(Configuracao::TipoCruzamento::ciclo)
            .tipoMutacao(Configuracao::TipoMutacao::substiui_disciplina)
            .mutacaoProbabilidade(15) // %
            .graspNumVizinhos(2)
            .graspAlfa(20) // %
            .camadaTamanho(input_all_json.camadasTamanho)
            .perfilTamanho(input_all_json.perfilTamanho)
            .numTorneioPopulacao(4)
            .tentativasMutacao(4)
            .graspVizinhanca(Configuracao::TipoVizinhos::aleatorios)
            .tipoConstrucao(Configuracao::TipoGrade::grasp)
            };

    std::cout << "Montando horarios [AG + Modelo]...\n";

    auto inicio = Util::now();
    r.gerarHorarioAG();
    auto fim = Util::now();

    std::cout << "Tempo do horario: " << Util::chronoDiff(fim, inicio) << "ms\n\n";

    auto fo = r.getSolucao()->getFO();
    std::cout << "\nResultado:" << fo << "\n";
    //r.showResult();

    auto savePath = Util::join_path({"teste", "fo" + std::to_string(fo)});
    Output::write(r.getSolucao(), savePath);

    std::ostringstream oss;
    oss << "solucaoAlvo: " << r.foAlvo << "\n";
    oss << "hashAlvo: " << r.hashAlvo << "\n";
    oss << "iteracoes: " << r.iteracaoAlvo << "\n";
    oss << "tempoAlvo: " << r.tempoAlvo << "\n";
    oss << "ultima iteracao: " << r.ultimaIteracao << "\n\n";
    std::cout << oss.str();

    std::ofstream out {savePath + "log.txt"};
    //r.logExperimentos();
    out << oss.str();
    out << r.getLog();
}

std::pair<long long, int>
novo_experimento(const std::string& input, const std::string& id,
                 int n_indiv, int taxa_mut, int p_cruz,
                 const std::string& oper_cruz, int grasp_iter, 
                 int grasp_nviz, int grasp_alfa, int n_tour, int n_mut,
                 int exec_i)
{
    // n�mero de itera��es grande para o algoritmo se encerrar por tempo
    const auto inf = gsl::narrow_cast<int>(1e9); 

    auto cruzamento = [&] {
        if (oper_cruz == "PMX") {
            return Configuracao::TipoCruzamento::pmx;
        } else if (oper_cruz == "CX") {
            return Configuracao::TipoCruzamento::ciclo;
        } else if (oper_cruz == "OX") {
            return Configuracao::TipoCruzamento::ordem;
        }
    }();

    Resolucao r {Configuracao()
            .arquivoEntrada(input)
            .populacaoInicial(n_indiv)
            .porcentagemCruzamentos(p_cruz) // %
            .numMaximoIteracoesSemEvolucaoGRASP(grasp_iter)
            .numMaximoIteracoesSemEvolucaoAG(inf)
            .tipoCruzamento(cruzamento)
            .mutacaoProbabilidade(taxa_mut) // %
            .graspNumVizinhos(grasp_nviz)
            .graspAlfa(grasp_alfa) // %
            .camadaTamanho(input_all_json.camadasTamanho)
            .perfilTamanho(input_all_json.perfilTamanho)
            .numTorneioPopulacao(n_tour)
            .tentativasMutacao(n_mut)
    };

    Timer t;
    r.gerarHorarioAG();
    auto fo = r.getSolucao()->getFO();
    auto tempo = t.elapsed();

    return {tempo, fo};
}

void novo_experimento_cli(const std::string& input, const std::string& file)
{
    std::ifstream config {file};

    // coment�rio do topo
    // formato entrada:
    // ID TaxaMut NIndiv %Cruz CruzOper NMut NTour GRASPIter GRASPNVzi GRASPAlfa NExec
    // formato saida:
    // ID,NExec,Tempo,Fo

    std::string id, cruz_oper;
    int taxa_mut, n_indiv, p_cruz, n_mut, n_tour, grasp_iter;
    int grasp_nviz, grasp_alfa, n_exec;

    while (config >> id >> taxa_mut >> n_indiv >> p_cruz
        >> cruz_oper >> n_mut >> n_tour
        >> grasp_iter >> grasp_nviz >> grasp_alfa >> n_exec) {

        auto path = Util::join_path({"experimento"});
        Util::create_folder(path);

        auto filename = path + id + ".txt";

        std::cout << "\n\nID: " << id << "\n";

        for (auto i = 0; i < n_exec; i++) {
            long long tempo;
            int fo;
            std::tie(tempo, fo) = novo_experimento(
                input, id, n_indiv, taxa_mut, p_cruz, cruz_oper, grasp_iter,
                grasp_nviz, grasp_alfa, n_tour, n_mut, i);

            std::ofstream out {filename};
            out << "ID Algoritmo, Numero execucao, Tempo total, FO\n";
            Util::logprint(out, boost::format("%s,%d,%lld,%d\n") % id % i % tempo % fo);
        }
    }
}

template <typename F>
std::string teste_tempo_iter(int num_exec, F f)
{
    std::ostringstream oss;

    for (auto i = 0; i < num_exec; i++) {
        Resolucao r{Configuracao()
            .arquivoEntrada(Util::join_path({"entradas"}, "input.all.json"))
            .camadaTamanho(input_all_json.camadasTamanho)
            .perfilTamanho(input_all_json.perfilTamanho)
            .tentativasMutacao(4)
        };

        Util::logprint(oss, boost::format("i: %d") % (i+1));

        Timer t;
        auto s = f(r);

        Util::logprint(oss, boost::format(" - fo: %d - t: %lld\n") % s->getFO() % t.elapsed());
        Util::logprint(oss, 
            boost::format("\t fo_alvo: %d - tempo_alvo: %lld\n") % r.foAlvo % r.tempoAlvo);
    }

    Util::logprint(oss, "\n");

    return oss.str();
}

void teste_tempo()
{
    const auto timeout_sec = 120;
    const auto timeout_ms = timeout_sec * 1000;
    const auto num_exec = 10;

    std::ostringstream oss;
    oss << std::string(25, '=') << "\n";
    oss << "Data: " << Util::dateTime() << "\n";
    oss << "Tempo maximo: " << timeout_ms << "\n";
    oss << "Numero de execucoes: " << num_exec << "\n\n";

    Util::logprint(oss, "SA-ILS\n");
    oss << teste_tempo_iter(num_exec, [&](Resolucao& r) {
        return r.gerarHorarioSA_ILS(timeout_ms);
    });

    Util::logprint(oss, "HySST\n");
    oss << teste_tempo_iter(num_exec, [&](Resolucao& r) {
        return r.gerarHorarioHySST(timeout_ms, 100, 100);
    });

    Util::logprint(oss, "WDJU\n");
    oss << teste_tempo_iter(num_exec, [&](Resolucao& r) {
        return r.gerarHorarioWDJU(timeout_ms);
    });

    std::ofstream out;
    out.open((boost::format("resultados%d.txt") % timeout_sec).str(), 
             std::ios::out | std::ios::app);
    out << oss.str() << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc == 3) {
        // Primeiro argumento � a entrada, o segundo � o arquivo de configura��o
        //novo_experimento_cli(argv[1], argv[2]);
        semArgumentos();
    } else if (argc == 2) {
        std::string flag = argv[1];
        if (flag == "-h" || flag == "--help") {
            std::cout << "Primeiro argumento � a entrada, o segundo � o arquivo de configura��o\n";
            std::cout << "Formato da config:\n";
            std::cout << "ID AGIter NIndiv %Cruz NMut NTour GRASPIter GRASPNVzi GRASPAlfa NExec\n";
        }
    } else {
        //semArgumentos();

        teste_tempo();
    }

    throw;
}
