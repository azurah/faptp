#include <fstream>
#include <numeric>

#include <modelo-grade/arquivos.h>
#include <modelo-grade/modelo_solver.h>

#include "parametros.h"
#include "Resolucao.h"
#include "Algorithms.h"
#include "Util.h"
#include "Semana.h"
#include "Aleatorio.h"

Resolucao::Resolucao(const Configuracao& c)
	: horarioPopulacaoInicial(c.popInicial_)
	  , horarioTorneioPares(c.numTorneioPares_)
	  , horarioTorneioPopulacao(c.numTorneioPop_)
	  , horarioIteracao(c.numIter_)
	  , horarioMutacaoProbabilidade(c.mutProb_)
	  , horarioMutacaoTentativas(c.mutTentativas_)
	  , gradeTipoConstrucao(c.tipoConstr_)
	  , gradeAlfa(c.graspAlfa_)
	  , gradeGraspVizinhanca(c.tipoVizinhanca_)
	  , gradeGraspVizinhos(c.numVizinhos_)
	  , gradeGraspTempoConstrucao(c.graspTempo_)
	  , blocosTamanho(c.blocoTam_)
	  , camadasTamanho(c.camadaTam_)
	  , perfisTamanho(c.perfilTam_)
	  , arquivoEntrada(c.filename_)
	  , professores()
	  , disciplinas()
	  , periodoXdisciplina()
	  , alunoPerfis()
	  , professorDisciplinas()
	  , solucao(nullptr)
	  , jsonRoot()
	  , util()
	  , curso(nullptr)
	  , alunos()
{
	carregarDados();
}

Resolucao::Resolucao(int pBlocosTamanho, int pCamadasTamanho, int pPerfisTamanho,
                     Configuracao::TipoGrade pTipoConstrucao, std::string pArquivoEntrada)
	: gradeTipoConstrucao(pTipoConstrucao)
	  , blocosTamanho(pBlocosTamanho)
	  , camadasTamanho(pCamadasTamanho)
	  , perfisTamanho(pPerfisTamanho)
	  , arquivoEntrada(pArquivoEntrada)
	  , curso(nullptr)
	  , alunos()
{
	carregarDados();
}

Resolucao::~Resolucao()
{
	delete solucao;

	while (!disciplinas.empty()) {
		delete disciplinas.back();
		disciplinas.pop_back();
	}

	for (auto& par : professores) {
		delete par.second;
	}

	for (auto& par : alunoPerfis) {
		delete par.second;
	}

	for (auto& par : professorDisciplinas) {
		delete par.second;
	}
}

fagoc::Curso& Resolucao::getCurso()
{
	return *curso;
}

const std::vector<fagoc::Aluno>& Resolucao::getAlunos() const
{
	return alunos;
}

void Resolucao::carregarDados()
{
#if defined(_WIN32)
	std::string folder = "res\\";
#else 
	std::string folder = "res/";
#endif
	std::ifstream myfile(folder + arquivoEntrada);

	if (myfile.is_open()) {
		myfile >> jsonRoot;
		myfile.close();

		carregarDadosDisciplinas();
		carregarDadosProfessores();
		carregarAlunoPerfis();

		if (gradeTipoConstrucao == Configuracao::TipoGrade::modelo) {
			auto p = fagoc::ler_json(folder + arquivoEntrada);
			curso.reset(new fagoc::Curso(std::move(p.first)));
			alunos = move(p.second);
		}
	} else {
		std::cerr << "We had a problem reading file (" << arquivoEntrada << ")\n";
		throw 1;
	}
}

void Resolucao::carregarDadosProfessores()
{
	const auto& jsonProfessores = jsonRoot["professores"];

	for (auto i = 0u; i < jsonProfessores.size(); i++) {
		std::string id = jsonProfessores[i]["id"].asString();
		std::string nome = jsonProfessores[i]["nome"].asString();

		professores[id] = new Professor(nome, id);
		auto& diasDisponiveis = professores[id]->diasDisponiveis;
		auto& numDisp = professores[id]->numDisponibilidade;

		if (jsonProfessores[i].isMember("creditoMaximo") == 1) {
			professores[id]->setCreditoMaximo(jsonProfessores[i]["creditoMaximo"].asInt());
		}

		if (jsonProfessores[i].isMember("disponibilidade") == 1) {
			const auto& disponibilidade = jsonProfessores[i]["disponibilidade"];
			diasDisponiveis.resize(disponibilidade.size());
			for (auto i = 0u; i < disponibilidade.size(); i++) {
				diasDisponiveis[i].resize(disponibilidade[i].size());
				for (auto j = 0u; j < disponibilidade[i].size(); j++) {
					diasDisponiveis[i][j] = disponibilidade[i][j].asBool();
				}

				numDisp += accumulate(begin(diasDisponiveis[i]), end(diasDisponiveis[i]), 0);
			}
		}

		const auto& competencias = jsonProfessores[i]["competencias"];
		for (auto j = 0u; j < competencias.size(); j++) {

			std::vector<Disciplina*>::iterator it;
			std::string disciplinaId = competencias[j].asString();

			it = find_if(disciplinas.begin(), disciplinas.end(),
			             DisciplinaFindDisciplinaId(disciplinaId));
			(*it)->addProfessorCapacitado(professores[id]);
		}
	}
}

void Resolucao::carregarDadosDisciplinas()
{
	const auto& jsonDisciplinas = jsonRoot["disciplinas"];

	for (auto i = 0u; i < jsonDisciplinas.size(); i++) {
		const auto id = jsonDisciplinas[i]["id"].asString();
		const auto nome = jsonDisciplinas[i]["nome"].asString();
		const auto curso = jsonDisciplinas[i]["curso"].asString();
		const auto capacidade = jsonDisciplinas[i]["capacidade"].asInt();

		const auto cargahoraria = jsonDisciplinas[i]["carga"].asInt();
		const auto turma = jsonDisciplinas[i]["turma"].asString();

		const auto periodo = jsonDisciplinas[i]["periodo"].asString();
		const auto periodoMinimo = jsonDisciplinas[i]["periodominimo"].asString();

		Disciplina* disciplina = new Disciplina(nome, cargahoraria, periodo, curso, id, turma, capacidade, periodoMinimo);

		const auto& corequisitos = jsonDisciplinas[i]["corequisitos"];
		for (auto j = 0u; j < corequisitos.size(); j++) {
			disciplina->coRequisitos.push_back(corequisitos[j].asString());
		}

		const auto& prerequisitos = jsonDisciplinas[i]["prerequisitos"];
		for (auto j = 0u; j < prerequisitos.size(); j++) {
			disciplina->preRequisitos.push_back(prerequisitos[j].asString());
		}

		const auto& equivalentes = jsonDisciplinas[i]["equivalentes"];
		for (auto j = 0u; j < equivalentes.size(); j++) {
			disciplina->equivalentes.push_back(equivalentes[j].asString());
		}
		disciplina->equivalentes.push_back(nome);

		disciplinas.push_back(disciplina);

		periodoXdisciplina[curso + periodo].push_back(disciplina);
	}

	disciplinas = ordenarDisciplinas();
}

void Resolucao::carregarDadosProfessorDisciplinas()
{
	const auto& jsonProfessorDisciplinas = jsonRoot["professordisciplinas"];

	for (auto i = 0u; i < jsonProfessorDisciplinas.size(); i++) {
		const auto id = jsonProfessorDisciplinas[i]["id"].asString();
		const auto professor = jsonProfessorDisciplinas[i]["professor"].asString();
		const auto disciplina = jsonProfessorDisciplinas[i]["disciplina"].asString();

		ProfessorDisciplina* professorDisciplina = new ProfessorDisciplina(professores[professor], disciplinas[disciplinasIndex[disciplina]], id);

		/**
		 * TODO: recuperar peso do v�nculo do professor com a disciplina do arquivo de entrada
		 */
		double competenciaPeso = 1.0;
		professorDisciplina->professor->addCompetencia(disciplina, competenciaPeso);

		professorDisciplinas[id] = professorDisciplina;
	}
}

void Resolucao::carregarAlunoPerfis()
{
	const auto& jsonAlunoPerfis = jsonRoot["alunoperfis"];

	for (auto i = 0u; i < jsonAlunoPerfis.size(); i++) {
		const auto id = jsonAlunoPerfis[i]["id"].asString();
		const auto peso = jsonAlunoPerfis[i]["peso"].asDouble();
		const auto turma = jsonAlunoPerfis[i]["turma"].asString();
		const auto periodo = jsonAlunoPerfis[i]["periodo"].asString();

		AlunoPerfil* alunoPerfil = new AlunoPerfil(peso, id, turma, periodo);

		const auto& jsonRestantes = jsonAlunoPerfis[i]["restantes"];
		for (auto j = 0u; j < jsonRestantes.size(); j++) {
			Disciplina* disciplina = disciplinas[disciplinasIndex[jsonRestantes[j].asString()]];
			alunoPerfil->addRestante(disciplina);
		}
		alunoPerfil->restante = ordenarDisciplinas(alunoPerfil->restante);

		const auto& jsonCursadas = jsonAlunoPerfis[i]["cursadas"];
		for (auto j = 0u; j < jsonCursadas.size(); j++) {
			std::string cursada = jsonCursadas[j].asString();
			alunoPerfil->addCursada(cursada);
		}

		std::vector<Disciplina*> aprovadas;
		const auto& restantes = alunoPerfil->restante;
	/*	set_difference(disciplinas.begin(), disciplinas.end(),
		               alunoPerfil->restante.begin(), alunoPerfil->restante.end(),
		               inserter(aprovadas, aprovadas.begin()), DisciplinaCargaHorariaDesc());
*/
		std::remove_copy_if(begin(disciplinas), end(disciplinas), back_inserter(aprovadas),
							[&restantes](Disciplina* d)
		{
			return find_if(begin(restantes), end(restantes), [&d](Disciplina* x)
			{
				return x->id == d->id;
			}) != end(restantes);
		});

		auto& aprovadasNomes = alunoPerfil->aprovadas;

		for (const auto& aprovada : aprovadas) {
			aprovadasNomes.push_back(aprovada->nome);
		}

		alunoPerfis[id] = alunoPerfil;
	}
}

double Resolucao::start()
{
	return start(true);
}

double Resolucao::start(bool input)
{
	if (input) {
		carregarSolucao();
		return gerarGrade();
	}

	return gerarHorarioAG()->getObjectiveFunction();
}

void Resolucao::carregarSolucao()
{
	const auto& jsonHorario = jsonRoot["horario"];

	carregarDadosProfessorDisciplinas();

	Solucao* solucaoLeitura = new Solucao(blocosTamanho, camadasTamanho, perfisTamanho);
	int bloco, dia, camada;

	for (auto i = 0u; i < jsonHorario.size(); i++) {
		bloco = jsonHorario[i]["horario"].asInt();
		dia = jsonHorario[i]["semana"].asInt();
		camada = jsonHorario[i]["camada"].asInt();

		ProfessorDisciplina* professorDisciplina = professorDisciplinas[jsonHorario[i]["professordisciplina"].asString()];
		if (verbose)
			std::cout << "D:" << dia << " - B:" << bloco << " - C:" << camada << " - PDSP:" << professorDisciplina->disciplina->nome << "  - P:";
		solucaoLeitura->horario->insert(dia, bloco, camada, professorDisciplina);
	}

	if (verbose)
		std::cout << "-----------------------------------------" << std::endl;
	solucao = solucaoLeitura;
}

std::vector<Disciplina*> Resolucao::ordenarDisciplinas()
{
	disciplinas = ordenarDisciplinas(disciplinas);

	atualizarDisciplinasIndex();

	return disciplinas;
}

std::vector<Disciplina*> Resolucao::ordenarDisciplinas(std::vector<Disciplina*> pDisciplinas)
{
	std::vector<Disciplina*>::iterator dIter = pDisciplinas.begin();
	std::vector<Disciplina*>::iterator dIterEnd = pDisciplinas.end();

	sort(dIter, dIterEnd, DisciplinaCargaHorariaDesc());

	return pDisciplinas;
}

void Resolucao::atualizarDisciplinasIndex()
{
	Disciplina* disciplina;

	std::vector<Disciplina*>::iterator dIter = disciplinas.begin();
	std::vector<Disciplina*>::iterator dIterEnd = disciplinas.end();

	disciplinasIndex.clear();
	for (int i = 0; dIter != dIterEnd; ++dIter , i++) {
		disciplina = *dIter;
		disciplinasIndex[disciplina->id] = i;
	}
}

Solucao* Resolucao::gerarHorarioAG()
{
	Solucao* solucaoAG;
	std::vector<Solucao*> populacao = gerarHorarioAGPopulacaoInicial();

	std::vector<Solucao*> parVencedor;
	std::vector<Solucao*> filhos1;
	std::vector<Solucao*> filhos2;
	std::vector<Solucao*> geneX;

	int iMax = horarioIteracao; //(populacao.size() * horarioTorneioPopulacao) * horarioTorneioPares;

	for (int i = 0; i < iMax; i++) {
		std::cout << "i: " << i << "\n";

		//sort(populacao.begin(), populacao.end(), std::greater<Solucao*>());

		parVencedor = gerarHorarioAGTorneioPar(populacao);

		filhos1 = gerarHorarioAGCruzamentoAleatorio(parVencedor[0], parVencedor[1]);
		filhos2 = gerarHorarioAGCruzamentoAleatorio(parVencedor[1], parVencedor[0]);

		//populacao.insert(populacao.end(), filhos1.begin(), filhos1.end());
		Util::insert_sorted(populacao, begin(filhos1), end(filhos1),
		                    SolucaoComparaMaior());
		// Adiciona o segundo grupo de filhos no primeiro vetor
		//filhos1.insert(filhos1.end(), filhos2.begin(), filhos2.end());
		Util::insert_sorted(populacao, begin(filhos2), end(filhos2),
		                    SolucaoComparaMaior());

		geneX = gerarHorarioAGMutacao(filhos1);

		//populacao.insert(populacao.end(), geneX.begin(), geneX.end());
		Util::insert_sorted(populacao, begin(geneX), end(geneX),
		                    SolucaoComparaMaior());

		gerarHorarioAGSobrevivenciaElitismo(populacao);
	}

	//sort(populacao.begin(), populacao.end(), std::greater<Solucao*>());
	gerarHorarioAGSobrevivenciaElitismo(populacao, 1);
	solucaoAG = populacao[0];

	solucao = solucaoAG;

	return solucaoAG;
}

std::vector<Solucao*> Resolucao::gerarHorarioAGPopulacaoInicial()
{
	std::vector<Solucao*> solucoesAG;

	Disciplina* disciplinaAleatoria;
	Professor* professorSelecionado;

	std::string dId;
	std::string pId;
	std::string pdId;

	int randInt;
	bool inserted = false;
	bool professorPossuiCreditos;

	std::map<std::string, int> creditosUtilizadosProfessor;
	std::map<std::string, int> colisaoProfessor;

	/**
	 * TODO: ordenar os professores com menos blocos dispon�veis para dar aula
	 * Priorizar esses professores para que eles tenham as aulas montadas
	 * antes dos outros professores
	 */

	while (static_cast<int>(solucoesAG.size()) != horarioPopulacaoInicial) {
		Solucao* solucaoLocal = new Solucao(blocosTamanho, camadasTamanho, perfisTamanho);
		int i = 0;

		creditosUtilizadosProfessor.clear();
		colisaoProfessor.clear();

		for (const auto& par : periodoXdisciplina) {

			std::vector<Disciplina*> disciplinas = par.second;
			std::map<std::string, int> creditosUtilizadosDisciplina;
			std::map<std::string, ProfessorDisciplina*> disciplinaXprofessorDisciplina;
			int dia = 0, bloco = 0;

			bool finding = false;

			std::map<std::string, int> infactiveis;

			colisaoProfessor.clear();

			while (disciplinas.size() != 0) {
				randInt = 0;

				/**
				 * TODO: implementar aleatoriedade uniforme
				 */
				switch (0) {
				case 0:
					randInt = aleatorio::randomInt() % disciplinas.size();
					break;
				case 1:
					break;
				}

				disciplinaAleatoria = disciplinas[randInt];
				dId = disciplinaAleatoria->getId();

				// Se a disciplina ainda n�o foi adicionada a lista de creditos utilizados
				if (creditosUtilizadosDisciplina.count(dId) == 0) {
					creditosUtilizadosDisciplina[dId] = 0;
				}

				// Se a disciplina ainda n�o tem professor alocado
				if (disciplinaXprofessorDisciplina.count(dId) == 0) {

					do {
						randInt = aleatorio::randomInt() % disciplinaAleatoria->professoresCapacitados.size();

						professorSelecionado = disciplinaAleatoria->professoresCapacitados[randInt];
						pId = professorSelecionado->getId();

						if (creditosUtilizadosProfessor.count(pId) == 0) {
							creditosUtilizadosProfessor[pId] = 0;
						}
						professorPossuiCreditos = (professorSelecionado->creditoMaximo != 0 && professorSelecionado->creditoMaximo < (creditosUtilizadosProfessor[pId] + disciplinaAleatoria->getCreditos()));
					} while (professorPossuiCreditos);

					int profNum = randInt; //0;

					/*for (; profNum < disciplinaAleatoria->professoresCapacitados.size(); profNum++) {
					    const auto& professor = disciplinaAleatoria->professoresCapacitados[profNum];
					    pId = professor->getId();

					    if (creditosUtilizadosProfessor.count(pId) == 0) {
					        creditosUtilizadosProfessor[pId] = 0;
					    }

					    if (professor->creditoMaximo != 0
					            && professor->creditoMaximo < (creditosUtilizadosProfessor[pId]
					            + disciplinaAleatoria->getCreditos())) {
					        continue;
					    } else break;
					}*/

					creditosUtilizadosProfessor[pId] += disciplinaAleatoria->getCreditos();

					pdId = "pr" + pId + "di" + dId;
					if (professorDisciplinas.count(pdId) == 0) {
						professorDisciplinas[pdId] = new ProfessorDisciplina(disciplinaAleatoria->professoresCapacitados[profNum], disciplinaAleatoria, pdId);
					}

					disciplinaXprofessorDisciplina[dId] = professorDisciplinas[pdId];
				}

				if (dia > (SEMANA - 1)) {
					finding = true;
				}

				if (finding) {
					std::vector<int> empties = solucaoLocal->horario->getAllEmpty(i);
					int positions[3];

					for (std::vector<int>::iterator it = empties.begin(); it != empties.end(); ++it) {

						solucaoLocal->horario->get3DMatrix((*it), positions);

						dia = positions[1];
						bloco = positions[0];

						inserted = solucaoLocal->horario->insert(dia, bloco, i, disciplinaXprofessorDisciplina[dId]);
						if (inserted) {
							break;
						}

						//std::cout << i << " " << par.first << " (" << (*it) << " - " << dia << " - " << bloco << ") [" << disciplinaAleatoria->nome << "]" << std::endl;
					}

					if (infactiveis.count(dId) == 0) {
						infactiveis[dId] = 0;
					}
					infactiveis[dId]++;

					//std::cerr << infactiveis[dId] << ") Deu merda! " << par.first << " [" << disciplinaAleatoria->nome << "]" << std::endl;
					if (infactiveis[dId] > 4) {

						solucaoLocal->horario->clearDisciplina(disciplinaXprofessorDisciplina[dId], i);

						disciplinas.erase(find_if(disciplinas.begin()
						                          , disciplinas.end()
						                          , DisciplinaFindDisciplina(disciplinaAleatoria)));

						continue;
					}
				} else {

					inserted = solucaoLocal->horario->insert(dia, bloco, i, disciplinaXprofessorDisciplina[dId]);
				}

				if (inserted) {

					bloco++;
					creditosUtilizadosDisciplina[dId]++;
					if (!finding && ((bloco % 2) == 1 && disciplinaAleatoria->getCreditos() > creditosUtilizadosDisciplina[dId])) {
						inserted = solucaoLocal->horario->insert(dia, bloco, i, disciplinaXprofessorDisciplina[dId]);
						if (inserted) {
							bloco++;
							creditosUtilizadosDisciplina[dId]++;
						}
					}
				} else {
					if (colisaoProfessor.count(dId) == 0) {
						colisaoProfessor[dId] = 0;
					}

					colisaoProfessor[dId]++;

					if (colisaoProfessor[dId] == horarioProfessorColisaoMax) {

						if (bloco % 2 == 1) {
							bloco += 1;
						} else {
							bloco += 2;
						}

						colisaoProfessor[dId] = 0;
					}
				}

				if (bloco >= blocosTamanho) {
					bloco = 0;
					dia++;
				}

				//std::cout << "\n" << disciplinaAleatoria->getCreditos() << " == " << creditosUtilizadosDisciplina[dId] << std::endl;
				if (disciplinaAleatoria->getCreditos() == creditosUtilizadosDisciplina[dId]) {

					disciplinas.erase(find_if(disciplinas.begin()
					                          , disciplinas.end()
					                          , DisciplinaFindDisciplina(disciplinaAleatoria)));
				}
			}

			i++;
		}

		gerarGrade(solucaoLocal);
		//gerarGradeTipoGrasp(solucaoLocal, false);
		//solucoesAG.push_back(solucaoLocal);
		Util::insert_sorted(solucoesAG, solucaoLocal, SolucaoComparaMaior());
	}

	return solucoesAG;
}

std::vector<Solucao*> Resolucao::gerarHorarioAGTorneioPar(std::vector<Solucao*> solucoesPopulacao)
{
	std::vector<Solucao*> torneioPar;

	torneioPar.push_back(gerarHorarioAGTorneio2(solucoesPopulacao));
	torneioPar.push_back(gerarHorarioAGTorneio2(solucoesPopulacao));

	return torneioPar;
}

Solucao* Resolucao::gerarHorarioAGTorneio(std::vector<Solucao*> solucoesPopulacao) const
{
	std::vector<Solucao*> torneioCandidatos;
	Solucao* vencedor = nullptr;
	double vencedorFO = 0;
	double randomFO;
	int randInt;

	auto populacaoTorneioMax = size_t(horarioTorneioPopulacao) * solucoesPopulacao.size();

	while (torneioCandidatos.size() <= populacaoTorneioMax && solucoesPopulacao.size() != 0) {
		randInt = aleatorio::randomInt() % solucoesPopulacao.size();

		randomFO = solucoesPopulacao[randInt]->getObjectiveFunction();
		if (vencedorFO < randomFO) {
			vencedor = solucoesPopulacao[randInt];
			vencedorFO = randomFO;
		}

		// Remove elemento sorteado
		solucoesPopulacao.erase(remove(solucoesPopulacao.begin(), solucoesPopulacao.end(), solucoesPopulacao[randInt]), solucoesPopulacao.end());
	}

	return vencedor;
}

Solucao* Resolucao::gerarHorarioAGTorneio2(std::vector<Solucao*> solucoesPopulacao)
{
	Util util;
	int primeiro, segundo;

	auto param = std::min(int(solucoesPopulacao.size()) - 1, 15);
	primeiro = util.randomBetween(0, param);
	segundo = util.randomBetween(param, solucoesPopulacao.size());

	return (solucoesPopulacao[primeiro]->getObjectiveFunction() > solucoesPopulacao[segundo]->getObjectiveFunction()) ? solucoesPopulacao[primeiro] : solucoesPopulacao[segundo];
}

std::vector<Solucao*> Resolucao::gerarHorarioAGCruzamentoAleatorio(Solucao* solucaoPai1, Solucao* solucaoPai2)
{
	std::vector<Solucao*> filhos;
	std::vector<ProfessorDisciplina*> matrizBackup;

	auto camadasMax = int(horarioCruzamentoCamadas * camadasTamanho);
	int numFilhos = 0;

	int camadaPeriodo, diaSemana, blocoHorario;
	int posicao, posicaoX;
	bool success;

	ProfessorDisciplina* e;
	ProfessorDisciplina* g;

	Solucao* filho;

	do {

		filho = new Solucao(*solucaoPai1);

		for (int i = 0; i < camadasMax; i++) {

			camadaPeriodo = aleatorio::randomInt() % camadasTamanho;

			for (int j = 0, tentativas = 0; j < horarioCruzamentoDias;) {
				success = false;

				// De segunda a s�bado
				diaSemana = aleatorio::randomInt() % 6;
				blocoHorario = aleatorio::randomInt() % blocosTamanho;

				posicao = filho->horario->getPosition(diaSemana, blocoHorario, camadaPeriodo);

				e = solucaoPai2->horario->at(posicao);
				g = filho->horario->at(posicao);

				// Backup
				matrizBackup = filho->horario->matriz;

				if (e == g) {
					// nada a fazer
					success = true;
				} else if (e == NULL) {
					// Remove G
					posicaoX = filho->horario->getPosition(diaSemana, blocoHorario, camadaPeriodo);
					filho->horario->matriz[posicaoX] = NULL;

					success = gerarHorarioAGCruzamentoAleatorioReparo(filho, diaSemana, blocoHorario, camadaPeriodo);
					if (!success) {
						filho->horario->matriz = matrizBackup;
					}
				} else if (g == NULL) {
					// Recupera primeira ocorrencia da disciplina E
					posicaoX = filho->horario->getFirstDisciplina(e->disciplina);
					// Se o professor da primeira ocorrencia for o mesmo professor da disciplina E
					if (filho->horario->matriz[posicaoX]->professor == e->professor) {
						// Remove primeira ocorrencia de E
						filho->horario->matriz[posicaoX] = NULL;

						success = filho->horario->insert(diaSemana, blocoHorario, camadaPeriodo, e);
						if (!success) {
							filho->horario->matriz = matrizBackup;
						}
					}
				} else {
					// Recupera primeira ocorrencia da disciplina E
					posicaoX = filho->horario->getFirstDisciplina(e->disciplina);
					// Se o professor da primeira ocorrencia for o mesmo professor da disciplina E
					if (filho->horario->matriz[posicaoX]->professor == e->professor) {

						// Remove primeira ocorrencia de E
						filho->horario->matriz[posicaoX] = NULL;

						// insere E
						success = filho->horario->insert(diaSemana, blocoHorario, camadaPeriodo, e);
						if (!success) {
							success = gerarHorarioAGCruzamentoAleatorioReparo(filho, diaSemana, blocoHorario, camadaPeriodo);
							if (!success) {
								filho->horario->matriz = matrizBackup;
							}
						}
					}
				}

				if (success || tentativas >= horarioCruzamentoTentativasMax) {
					j++;
					tentativas = 0;
				}
			}
		}
		gerarGradeTipoGraspClear(filho);
		filhos.push_back(filho);
		numFilhos++;
	} while (numFilhos <= horarioCruzamentoFilhos);


	return filhos;
}

bool Resolucao::gerarHorarioAGCruzamentoAleatorioReparoBloco(Solucao*& solucaoFilho, int diaG, int blocoG, int camadaG)
{
	bool success = false;
	ProfessorDisciplina* g = solucaoFilho->horario->at(diaG, blocoG, camadaG);

	for (int blocoReparo = 0; blocoReparo < blocosTamanho; blocoReparo++) {
		if (solucaoFilho->horario->at(diaG, blocoReparo, camadaG) == NULL) {
			success = solucaoFilho->horario->insert(diaG, blocoReparo, camadaG, g);
			if (success) {
				return true;
			}
		}
	}

	return false;
}

bool Resolucao::gerarHorarioAGCruzamentoAleatorioReparo(Solucao*& solucaoFilho, int diaG, int blocoG, int camadaG)
{
	if (gerarHorarioAGCruzamentoAleatorioReparoBloco(solucaoFilho, diaG, blocoG, camadaG)) {
		return true;
	}

	for (int diaReparo = 0; diaReparo < 6; diaReparo++) {
		if (diaReparo == diaG) {
			continue;
		}

		if (gerarHorarioAGCruzamentoAleatorioReparoBloco(solucaoFilho, diaReparo, blocoG, camadaG)) {
			return true;
		}
	}

	return false;
}

void Resolucao::gerarHorarioAGSobrevivenciaElitismo(std::vector<Solucao*>& populacao)
{
	gerarHorarioAGSobrevivenciaElitismo(populacao, horarioPopulacaoInicial);
}

void Resolucao::gerarHorarioAGSobrevivenciaElitismo(std::vector<Solucao*>& populacao, int populacaoMax)
{
	//std::sort(populacao.begin(), populacao.end(), greater<Solucao*>());

	for (size_t i = populacaoMax; i < populacao.size(); i++) {
		delete populacao[i];
	}

	populacao.resize(populacaoMax);
}

std::vector<Solucao*> Resolucao::gerarHorarioAGMutacao(std::vector<Solucao*> filhos)
{
	std::vector<Solucao*> genesX;
	Solucao* solucaoTemp;

	double porcentagem;

	// Muta��o dos filhos
	for (auto j = 0u; j < filhos.size(); j++) {
		porcentagem = ((aleatorio::randomInt() % 100) / 100);

		if (porcentagem <= horarioMutacaoProbabilidade) {
			solucaoTemp = gerarHorarioAGMutacao(filhos[j]);

			if (solucaoTemp != NULL) {
				gerarGradeTipoGraspClear(solucaoTemp);
				genesX.push_back(solucaoTemp);
			}
		}
	}

	return genesX;
}

Solucao* Resolucao::gerarHorarioAGMutacao(Solucao* pSolucao)
{
	Solucao* currentSolucao = new Solucao(*pSolucao);
	bool success = false;

	int camadaX;
	int diaX1, diaX2;
	int blocoX1, blocoX2;
	int x1, x2;

	ProfessorDisciplina *pdX1, *pdX2;
	std::vector<ProfessorDisciplina*> backup;

	for (int i = 0; i < horarioMutacaoTentativas; i++) {
		camadaX = aleatorio::randomInt() % camadasTamanho;

		diaX1 = aleatorio::randomInt() % (SEMANA);
		diaX2 = aleatorio::randomInt() % (SEMANA);

		blocoX1 = aleatorio::randomInt() % blocosTamanho;
		blocoX2 = aleatorio::randomInt() % blocosTamanho;

		x1 = currentSolucao->horario->getPosition(diaX1, blocoX1, camadaX);
		x2 = currentSolucao->horario->getPosition(diaX2, blocoX2, camadaX);

		backup = currentSolucao->horario->matriz;

		pdX1 = currentSolucao->horario->at(x1);
		pdX2 = currentSolucao->horario->at(x2);

		if (pdX1 == pdX2) {
			// Nada a fazer
		} else if (pdX1 == NULL) {
			if (currentSolucao->horario->insert(diaX1, blocoX1, camadaX, pdX2)) {
				currentSolucao->horario->matriz[x2] = NULL;
				success = true;
			}
		} else if (pdX2 == NULL) {
			if (currentSolucao->horario->insert(diaX2, blocoX2, camadaX, pdX1)) {
				currentSolucao->horario->matriz[x1] = NULL;
				success = true;
			}
		} else {
			currentSolucao->horario->matriz[x1] = NULL;
			currentSolucao->horario->matriz[x2] = NULL;

			if (currentSolucao->horario->insert(diaX1, blocoX1, camadaX, pdX2)
				&& currentSolucao->horario->insert(diaX2, blocoX2, camadaX, pdX1)) {
				success = true;
			}
		}
	}

	if (success) {
		return currentSolucao;
	} else {
		delete currentSolucao;
		return nullptr;
	}
}

double Resolucao::gerarGrade()
{
	return gerarGrade(solucao);
}

double Resolucao::gerarGrade(Solucao*& pSolucao)
{
	switch (gradeTipoConstrucao) {
	case Configuracao::TipoGrade::guloso:
		return gerarGradeTipoGuloso(pSolucao);

	case Configuracao::TipoGrade::grasp:
		return gerarGradeTipoGrasp(pSolucao);

	case Configuracao::TipoGrade::combinatorio:
		return gerarGradeTipoCombinacaoConstrutiva(pSolucao);

	case Configuracao::TipoGrade::modelo:
		return gerarGradeTipoModelo(pSolucao);
	}

	return 0;
}

double Resolucao::gerarGradeTipoGuloso(Solucao*& pSolucao)
{
	Horario* horario;
	Grade* apGrade;

	std::map<std::string, AlunoPerfil*>::iterator apIter = alunoPerfis.begin();
	std::map<std::string, AlunoPerfil*>::iterator apIterEnd = alunoPerfis.end();
	AlunoPerfil* alunoPerfil;

	std::vector<Disciplina*>::iterator dIter;
	std::vector<Disciplina*>::iterator dIterEnd;
	Disciplina* disciplina;
	std::vector<Disciplina*> apRestante;
	std::vector<std::string> apCursadas;

	horario = pSolucao->horario;

	for (; apIter != apIterEnd; ++apIter) {
		if (verbose)
			std::cout << apIter->first << std::endl;
		alunoPerfil = alunoPerfis[apIter->first];

		apGrade = new Grade(blocosTamanho, alunoPerfil, horario, disciplinas, disciplinasIndex);

		apRestante = alunoPerfil->restante;
		apCursadas = alunoPerfil->cursadas;

		dIter = apRestante.begin();
		dIterEnd = apRestante.end();

		for (; dIter != dIterEnd; ++dIter) {
			disciplina = *dIter;

			apGrade->insert(disciplina);
		}

		pSolucao->insertGrade(apGrade);
	}

	return pSolucao->getObjectiveFunction();
}

Grade* Resolucao::gerarGradeTipoCombinacaoConstrutiva(Grade* pGrade, std::vector<Disciplina*> disciplinasRestantes, int maxDeep, int deep, int current)
{
	Grade* bestGrade = new Grade(*pGrade);
	Grade* currentGrade;

	double bestFO, currentFO;

	bool viavel;

	for (size_t i = current; i < disciplinasRestantes.size(); i++) {
		currentGrade = new Grade(*pGrade);

		if (verbose)
			std::cout << "Nivel: " << (deep) << " Disciplina: " << i << " (" << disciplinasRestantes[i]->id << ")" << std::endl;

		viavel = currentGrade->insert(disciplinasRestantes[i]);
		if (viavel) {

			if (deep != maxDeep) {
				currentGrade = gerarGradeTipoCombinacaoConstrutiva(currentGrade, disciplinasRestantes, maxDeep, (deep + 1), (i + 1));
			}

			bestFO = bestGrade->getObjectiveFunction();
			currentFO = currentGrade->getObjectiveFunction();
			if (bestFO < currentFO) {
				bestGrade = currentGrade;
			}

			if (deep == 0) {
				//std::cout << "----------------------------" << std::endl;
			}
		} else {
			if (verbose)
				std::cout << "[inviavel]" << std::endl;
		}
	}
	if (deep == 0) {
		//std::cout << "############################" << std::endl;
	}

	return bestGrade;
}

Grade* Resolucao::gerarGradeTipoCombinacaoConstrutiva(Grade* pGrade, std::vector<Disciplina*> disciplinasRestantes, int maxDeep)
{
	return gerarGradeTipoCombinacaoConstrutiva(pGrade, disciplinasRestantes, maxDeep, 0, 0);
}

double Resolucao::gerarGradeTipoCombinacaoConstrutiva(Solucao*& pSolucao)
{
	Horario* horario;
	Grade* apGrade;

	std::map<std::string, AlunoPerfil*>::iterator apIter = alunoPerfis.begin();
	std::map<std::string, AlunoPerfil*>::iterator apIterEnd = alunoPerfis.end();
	AlunoPerfil* alunoPerfil;

	std::vector<Disciplina*> apRestante;

	horario = pSolucao->horario;

	for (; apIter != apIterEnd; ++apIter) {
		if (verbose)
			std::cout << "[" << apIter->first << "]" << std::endl;
		alunoPerfil = alunoPerfis[apIter->first];

		apRestante = alunoPerfil->restante;

		apGrade = gerarGradeTipoCombinacaoConstrutiva(new Grade(blocosTamanho, alunoPerfil, horario,
		                                                        disciplinas, disciplinasIndex), apRestante, apRestante.size());

		pSolucao->insertGrade(apGrade);

		const auto& escolhidas = apGrade->disciplinasAdicionadas;

		for (const auto disc : escolhidas) {
			disc->alocados++;
			if (disc->alocados >= disc->capacidade) {
				disc->ofertada = false;
			}
		}

	}

	return pSolucao->getObjectiveFunction();
}

void Resolucao::gerarGradeTipoGraspConstrucao(Grade* pGrade)
{
	std::vector<ProfessorDisciplina*> professorDisciplinasIgnorar;

	gerarGradeTipoGraspConstrucao(pGrade, professorDisciplinasIgnorar);
}

void Resolucao::gerarGradeTipoGraspConstrucao(Grade* pGrade, std::vector<ProfessorDisciplina*> professorDisciplinasIgnorar)
{
	AlunoPerfil* alunoPerfil;

	std::vector<std::string> apCursadas;
	std::vector<Disciplina*> apRestante;
	std::vector<Disciplina*>::iterator dIterStart;
	std::vector<Disciplina*>::iterator current;
	Disciplina* disciplina;

	Util util;

	int adicionados;
	int disponivel = (SEMANA - 2) * camadasTamanho;

	int rand;
	int distancia;

	alunoPerfil = pGrade->alunoPerfil;

	apCursadas = alunoPerfil->cursadas;
	apRestante = std::vector<Disciplina*>(alunoPerfil->restante.begin(), alunoPerfil->restante.end());

	dIterStart = apRestante.begin();

	adicionados = 0;
	while (apRestante.size() != 0 && dIterStart != apRestante.end() && disponivel != adicionados) {

		distancia = getIntervaloAlfaGrasp(apRestante);
		rand = util.randomBetween(0, distancia);
		current = dIterStart + rand;
		disciplina = *current;

		if (pGrade->insert(disciplina, professorDisciplinasIgnorar)) {

			adicionados++;
		}
		apRestante.erase(current);

		dIterStart = apRestante.begin();
	}
}

void Resolucao::gerarGradeTipoGraspConstrucao(Solucao* pSolucao)
{
	Horario* horario;
	Grade* apGrade;

	std::map<std::string, AlunoPerfil*>::iterator apIter = alunoPerfis.begin();
	std::map<std::string, AlunoPerfil*>::iterator apIterEnd = alunoPerfis.end();
	AlunoPerfil* alunoPerfil;

	horario = pSolucao->horario;

	for (; apIter != apIterEnd; ++apIter) {

		if (verbose)
			std::cout << apIter->first << std::endl;
		alunoPerfil = alunoPerfis[apIter->first];

		apGrade = new Grade(blocosTamanho, alunoPerfil, horario, disciplinas, disciplinasIndex);

		gerarGradeTipoGraspConstrucao(apGrade);

		pSolucao->insertGrade(apGrade);
	}
}

Solucao* Resolucao::gerarGradeTipoGraspRefinamentoAleatorio(Solucao* pSolucao)
{
	Solucao* bestSolucao = new Solucao(*pSolucao);
	Solucao* currentSolucao;

	std::map<std::string, AlunoPerfil*>::iterator apIter;
	std::map<std::string, AlunoPerfil*>::iterator apIterEnd;
	AlunoPerfil* alunoPerfil;

	Grade* grade;

	std::vector<ProfessorDisciplina*> professorDisciplinasIgnorar;

	int random;
	int disciplinasSize;
	int disciplinasRemoveMax;
	int disciplinasRemoveRand;
	double bestFO, currentFO;

	bestFO = bestSolucao->getObjectiveFunction();

	for (int i = 0; i < gradeGraspVizinhos; i++) {
		currentSolucao = new Solucao(*pSolucao);

		apIter = alunoPerfis.begin();
		apIterEnd = alunoPerfis.end();

		if (verbose)
			std::cout << "------NGH" << i << std::endl;

		for (; apIter != apIterEnd; ++apIter) {
			alunoPerfil = alunoPerfis[apIter->first];

			grade = currentSolucao->grades[alunoPerfil->id];

			disciplinasSize = grade->disciplinasAdicionadas.size();
			disciplinasRemoveMax = int(ceil(disciplinasSize * 1));
			disciplinasRemoveRand = util.randomBetween(1, disciplinasRemoveMax);

			for (int j = 0; j < disciplinasRemoveRand; j++) {
				ProfessorDisciplina* professorDisciplinaRemovido = NULL;

				disciplinasSize = grade->disciplinasAdicionadas.size();
				random = util.randomBetween(0, disciplinasSize);
				grade->remove(grade->disciplinasAdicionadas[random], professorDisciplinaRemovido);

				// Se houve uma remo��o
				if (professorDisciplinaRemovido != NULL) {
					professorDisciplinasIgnorar.push_back(professorDisciplinaRemovido);
				}
			}

			gerarGradeTipoGraspConstrucao(grade, professorDisciplinasIgnorar);
		}
		currentFO = currentSolucao->getObjectiveFunction();
		if (verbose)
			std::cout << std::endl << "------NGH" << i << ": L(" << bestFO << ") < C(" << currentFO << ")" << std::endl;

		if (bestFO < currentFO) {
			delete bestSolucao;
			bestSolucao = currentSolucao;
			bestFO = currentFO;

			if (verbose)
				std::cout << "------NGH new best: " << bestFO << std::endl;
			i = 0;
		} else {
			delete currentSolucao;
		}
	}

	return bestSolucao;
}

Solucao* Resolucao::gerarGradeTipoGraspRefinamentoCrescente(Solucao* pSolucao)
{
	Solucao* bestSolucao = new Solucao(*pSolucao);
	Solucao* currentSolucao;

	std::map<std::string, AlunoPerfil*>::iterator apIter;
	std::map<std::string, AlunoPerfil*>::iterator apIterEnd;
	AlunoPerfil* alunoPerfil;

	Disciplina* disciplinaRemovida;
	std::vector<Disciplina*> disciplinasRemovidas;
	std::vector<Disciplina*> disciplinasRestantes;
	std::vector<Disciplina*>::iterator drIter;
	std::vector<Disciplina*>::iterator drIterEnd;

	Grade *bestGrade, *currentGrade;

	Util util;

	int random;

	double bestFO, currentFO;

	apIter = alunoPerfis.begin();
	apIterEnd = alunoPerfis.end();

	for (; apIter != apIterEnd; ++apIter) {

		alunoPerfil = alunoPerfis[apIter->first];
		bestGrade = bestSolucao->grades[alunoPerfil->id];

		for (int i = 0; i < gradeGraspVizinhos; i++) {
			currentSolucao = new Solucao(*pSolucao);
			currentGrade = currentSolucao->grades[alunoPerfil->id];

			if (verbose)
				std::cout << std::endl << "------NGH" << i << std::endl;

			disciplinasRemovidas.clear();
			disciplinasRestantes = std::vector<Disciplina*>(alunoPerfil->restante.begin(), alunoPerfil->restante.end());

			for (int j = 0; j < (i + 1); j++) {
				random = util.randomBetween(0, currentGrade->disciplinasAdicionadas.size());
				if (random == -1) {
					break;
				}
				disciplinaRemovida = currentGrade->remove(currentGrade->disciplinasAdicionadas[random]);
				if (disciplinaRemovida != NULL) {
					disciplinasRemovidas.push_back(disciplinaRemovida);
				}
			}

			disciplinasRestantes.erase(remove_if(disciplinasRestantes.begin(), disciplinasRestantes.end(), DisciplinasRemoveDisciplinas(disciplinasRemovidas)), disciplinasRestantes.end());
			disciplinasRestantes.erase(remove_if(disciplinasRestantes.begin(), disciplinasRestantes.end(), DisciplinasRemoveDisciplinas(currentGrade->disciplinasAdicionadas)), disciplinasRestantes.end());

			gerarGradeTipoCombinacaoConstrutiva(currentGrade, disciplinasRestantes, i);

			bestFO = bestGrade->getObjectiveFunction();
			currentFO = currentGrade->getObjectiveFunction();
			if (verbose)
				std::cout << "------NGH" << i << ": L(" << bestFO << ") < C(" << currentFO << ")" << std::endl;
			if (bestFO < currentFO) {
				delete bestSolucao;
				bestSolucao = currentSolucao;
				bestFO = currentFO;

				if (verbose)
					std::cout << "------NGH new best: " << bestFO << std::endl;
				i = 0;
			} else {
				delete currentSolucao;
			}
		}
	}

	return bestSolucao;
}

double Resolucao::gerarGradeTipoGrasp()
{
	return gerarGradeTipoGrasp(solucao);
}

double Resolucao::gerarGradeTipoGrasp(Solucao*& pSolucao)
{
	Solucao* currentSolucao;
	Solucao* temp;

	double bestFO, currentFO;

	Util util;

	clock_t t0;
	double diff = 0;

	//    int const RESOLUCAO_GRASP_TEMPO_CONSTRUCAO = ceil(gradeGraspTempoConstrucao * alunoPerfis.size());
	double const RESOLUCAO_GRASP_TEMPO_CONSTRUCAO = gradeGraspTempoConstrucao;
	//printf("c: %f\n", RESOLUCAO_GRASP_TEMPO_CONSTRUCAO);

	bestFO = 0;

	if (verbose)
		std::cout << "HORARIO (Solucao) :" << std::endl;

	int iteracoes = 0;
	while (diff <= RESOLUCAO_GRASP_TEMPO_CONSTRUCAO) {
		currentSolucao = new Solucao(*pSolucao);

		t0 = clock();
		gerarGradeTipoGraspConstrucao(currentSolucao);

		if (verbose)
			std::cout << "----FIT: " << currentSolucao->getObjectiveFunction() << std::endl;

		temp = currentSolucao;

		switch (gradeGraspVizinhanca) {
		case Configuracao::TipoVizinhos::aleatorios:
			currentSolucao = gerarGradeTipoGraspRefinamentoAleatorio(currentSolucao);
			break;
		case Configuracao::TipoVizinhos::crescente:
			currentSolucao = gerarGradeTipoGraspRefinamentoCrescente(currentSolucao);
			break;
		}
		if (temp->id != currentSolucao->id)
			delete temp;

		if (verbose)
			std::cout << "----FIT(NGH):" << currentSolucao->getObjectiveFunction() << std::endl;

		diff += util.timeDiff(clock(), t0);

		currentFO = currentSolucao->getObjectiveFunction();
		if (bestFO < currentFO) {
			delete pSolucao;
			pSolucao = currentSolucao;
			bestFO = currentFO;
			diff = 0;

			if (verbose)
				std::cout << "----NGH is the new best (gerarGradeTipoGrasp)" << std::endl;
		} else {
			delete currentSolucao;
		}
		if (verbose)
			std::cout << "-------------------------------------------------" << std::endl;

		iteracoes++;
	}
	//std::cout << iteracoes << std::endl;

	solucao = pSolucao;
	return pSolucao->getObjectiveFunction();
}

double Resolucao::gerarGradeTipoGraspClear(Solucao*& pSolucao)
{
	pSolucao->grades.clear();
	pSolucao->gradesLength = 0;

	return gerarGrade(pSolucao);
}

std::vector<Disciplina*>::iterator Resolucao::getLimiteIntervaloGrasp(std::vector<Disciplina*> pApRestante)
{
	std::vector<Disciplina*>::iterator dIter = pApRestante.begin();
	std::vector<Disciplina*>::iterator dIterEnd = pApRestante.end();
	std::vector<Disciplina*>::iterator dIterLimit = dIter;

	int bestFIT = (*dIter)->cargaHoraria;
	int worstFIT = (pApRestante.back())->cargaHoraria;

	int currentFIT = 0;
	auto acceptFIT = bestFIT - int(ceil((1 - gradeAlfa) * (bestFIT - worstFIT)));

	for (; dIter != dIterEnd; ++dIter) {
		currentFIT = (*dIter)->cargaHoraria;
		if (currentFIT < acceptFIT) {

			break;
		}
		dIterLimit = dIter;
	}

	return dIterLimit;
}

int Resolucao::getIntervaloAlfaGrasp(std::vector<Disciplina*> pApRestante) const
{
	std::vector<Disciplina*>::iterator dIter = pApRestante.begin();
	std::vector<Disciplina*>::iterator dIterEnd = pApRestante.end();

	int distancia = 0;

	int bestFIT = (*dIter)->cargaHoraria;
	int worstFIT = (pApRestante.back())->cargaHoraria;

	int currentFIT = 0;
	auto acceptFIT = bestFIT - int(ceil((1 - gradeAlfa) * (bestFIT - worstFIT)));

	for (; dIter != dIterEnd; ++dIter) {
		currentFIT = (*dIter)->cargaHoraria;
		if (currentFIT < acceptFIT) {
			break;
		}
		distancia++;
	}

	return distancia;
}

void Resolucao::showResult()
{
	showResult(solucao);
}

void Resolucao::showResult(Solucao* pSolucao)
{
	const auto& grades = pSolucao->grades;
	for (const auto& par : grades) {
		const auto gradeAtual = par.second;
		std::cout << gradeAtual->alunoPerfil->id << ":\n";
		const auto& discEscolhidas = gradeAtual->disciplinasAdicionadas;
		for (const auto disc : discEscolhidas) {

			std::cout << disc->nome << " ";
		}
		std::cout << " " << gradeAtual->getObjectiveFunction() << "\n";
	}
	std::cout << "\nFO da solucao: " << pSolucao->getObjectiveFunction() << std::endl;
}

Solucao* Resolucao::getSolucao()
{
	return solucao;
}

double Resolucao::gerarGradeTipoModelo(Solucao* pSolucao)
{
	auto horarioBin = converteHorario(pSolucao);
	auto total = 0.0;
	for (const auto& aluno : alunos) {
		auto currAluno = alunoPerfis[aluno.nome()];
		auto novaGrade = new Grade(blocosTamanho, currAluno, pSolucao->horario, disciplinas, disciplinasIndex);
		fagoc::Modelo_solver solver(*curso, aluno, horarioBin);

		solver.solve();
		auto solucao = solver.solucao();
		novaGrade->fo = solucao->funcao_objetivo;
		total += novaGrade->fo;

		auto& adicionadas = novaGrade->disciplinasAdicionadas;
		adicionadas.clear();
		for (const auto& disciplina : solucao->nomes_disciplinas) {
			auto index = disciplinasIndex[disciplina];
			adicionadas.push_back(disciplinas[index]);
		}

		pSolucao->insertGrade(novaGrade);
	}

	return total;
}

std::vector<std::vector<char>> Resolucao::converteHorario(Solucao* pSolucao)
{
	auto matriz = pSolucao->horario->matriz;
	auto numDisciplinas = disciplinas.size();
	auto numHorarios = SEMANA * blocosTamanho;
	std::vector<std::vector<char>> horarioBin(numHorarios, std::vector<char>(numDisciplinas, 0));
	int posicoes[3];

	for (auto i = 0u; i < matriz.size(); i++) {
		if (!matriz[i]) {
			continue;
		}
		auto index = curso->nome_to_indice()[matriz[i]->disciplina->id];
		pSolucao->horario->get3DMatrix(i, posicoes);
		auto dia = posicoes[1];
		auto bloco = posicoes[0];
		auto posicao = (dia * blocosTamanho) + bloco;

		horarioBin[posicao][index] = 1;
	}

	return horarioBin;
}

