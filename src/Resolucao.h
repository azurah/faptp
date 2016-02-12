#ifndef RESOLUCAO_H
#define RESOLUCAO_H

#include <iostream>
#include <map>
#include <vector>

#include "Professor.h"
#include "Disciplina.h"
#include "ProfessorDisciplina.h"
#include "AlunoPerfil.h"
#include "Solucao.h"

#include "includes/json/json.h"

#define RESOLUCAO_GERAR_GRADE_TIPO_GULOSO          1
#define RESOLUCAO_GERAR_GRADE_TIPO_GRASP           2
#define RESOLUCAO_GERAR_GRADE_TIPO_COMBINATORIO    3

#define RESOLUCAO_GRASP_TEMPO_CONSTRUCAO_FATOR_DEFAULT    .5
#define RESOLUCAO_GRASP_ITERACAO_VIZINHOS_DEDAULT         5

#define RESOLUCAO_GRASP_VIZINHOS_ALEATORIOS   1
#define RESOLUCAO_GRASP_VIZINHOS_CRESCENTE    2

class Resolucao {
public:
    Resolucao(int pBlocosTamanho, int pCamadasTamanho, int pPerfisTamanho, string arquivoEntrada = "res/input.json");
    virtual ~Resolucao();

    double start();
    double start(bool input);

    Solucao* gerarHorarioAG();

    double gerarGrade();

    void showResult();

    Solucao* getSolucao();

    /*
     Par�metros da execu��o da solu��o
     */
    // Hor�rio popula��o inicial
    int horarioPopulacaoInicial;
    int horarioProfessorColisaoMax;

    // Hor�rio torneio
    double horarioTorneioPares;
    double horarioTorneioPopulacao;

    // Hor�rio cruzamento
    int horarioCruzamentoFilhos;
    int horarioCruzamentoDias;
    double horarioCruzamentoCamadas;
    int horarioCruzamentoTentativasMax;

    int horarioIteracao;

    // Hor�rio muta��o
    double horarioMutacaoProbabilidade;
    int horarioMutacaoTentativas;

    // Grade tipo de constru��o
    int gradeTipoConstrucao;
    double gradeAlfa;

    // Grade GRASP
    int gradeGraspVizinhanca;
    int gradeGraspVizinhos;
    double gradeGraspTempoConstrucao;
private:
    int blocosTamanho;
    int camadasTamanho;
    int perfisTamanho;
    string arquivoEntrada;

    map<string, Professor*> professores;

    map<string, int> disciplinasIndex;
    vector<Disciplina*> disciplinas;

    map< string, vector<Disciplina*> > periodoXdisciplina;

    map<string, AlunoPerfil*> alunoPerfis;

    map<string, ProfessorDisciplina*> professorDisciplinas;

    Solucao* solucao;

    Json::Value jsonRoot;

    void initDefault();

    void carregarDados();

    void carregarDadosProfessores();
    void carregarDadosDisciplinas();

    void carregarAlunoPerfis();

    void carregarDadosProfessorDisciplinas();

    void carregarSolucao();

    vector<Disciplina*> ordenarDisciplinas();
    vector<Disciplina*> ordenarDisciplinas(vector<Disciplina*> pDisciplina);
    void atualizarDisciplinasIndex();

    vector<Solucao*> gerarHorarioAGPopulacaoInicial();
    vector<Solucao*> gerarHorarioAGTorneioPar(vector<Solucao*> solucoesPopulacao);
    Solucao* gerarHorarioAGTorneio(vector<Solucao*> solucoesPopulacao);
    Solucao* gerarHorarioAGTorneio2(vector<Solucao*> solucoesPopulacao);
    vector<Solucao*> gerarHorarioAGCruzamentoAleatorio(Solucao *solucaoPai1, Solucao *solucaoPai2);
    bool gerarHorarioAGCruzamentoAleatorioReparoBloco(Solucao *&solucaoFilho, int diaG, int blocoG, int camadaG);
    bool gerarHorarioAGCruzamentoAleatorioReparo(Solucao *&solucaoFilho, int diaG, int blocoG, int camadaG);
    void gerarHorarioAGSobrevivenciaElitismo(vector<Solucao*> &populacao);
    void gerarHorarioAGSobrevivenciaElitismo(vector<Solucao*> &populacao, int populacaoMax);
    vector<Solucao*> gerarHorarioAGMutacao(vector<Solucao*> filhos);
    Solucao* gerarHorarioAGMutacao(Solucao* pSolucao);

    double gerarGrade(Solucao *&pSolucao);

    double gerarGradeTipoGuloso(Solucao *&pSolucao);

    Grade* gerarGradeTipoCombinacaoConstrutiva(Grade* pGrade, vector<Disciplina*> disciplinasRestantes, int maxDeep, int deep, int current);
    Grade* gerarGradeTipoCombinacaoConstrutiva(Grade* pGrade, vector<Disciplina*> disciplinasRestantes, int maxDeep);
    double gerarGradeTipoCombinacaoConstrutiva(Solucao *&pSolucao);

    void gerarGradeTipoGraspConstrucao(Solucao *pSolucao);

    void gerarGradeTipoGraspConstrucao(Grade* pGrade);
    void gerarGradeTipoGraspConstrucao(Grade* pGrade, vector<ProfessorDisciplina*> professorDisciplinasIgnorar);
    Solucao* gerarGradeTipoGraspRefinamentoAleatorio(Solucao *pSolucao);
    Solucao* gerarGradeTipoGraspRefinamentoCrescente(Solucao *pSolucao);
    double gerarGradeTipoGrasp();
    double gerarGradeTipoGrasp(Solucao *&pSolucao);

    double gerarGradeTipoGraspClear(Solucao *&pSolucao);

    vector<Disciplina*>::iterator getLimiteIntervaloGrasp(vector<Disciplina*> pApRestante);
    int getIntervaloAlfaGrasp(vector<Disciplina*> pApRestante) const;

    void showResult(Solucao *pSolucao);
};

#endif /* RESOLUCAO_H */