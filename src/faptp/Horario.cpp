#include <iostream>
#include <sstream>

#include <faptp/Horario.h>
#include <faptp/Semana.h>

Horario::Horario(int pBlocosTamanho, int pCamadasTamanho) : Representacao(pBlocosTamanho, pCamadasTamanho), hash_(0) {}

Horario::~Horario() {}

Horario::Horario(const Horario& outro)
    : Representacao(outro), discCamada(outro.discCamada),
      creditos_alocados_(outro.creditos_alocados_), hash_(0) {}

Horario& Horario::operator=(const Horario& outro)
{
    Representacao::operator=(outro);
    creditos_alocados_ = outro.creditos_alocados_;
    hash_ = 0;
    discCamada = outro.discCamada;
    return *this;
}

bool Horario::colisaoProfessorAlocado(int pDia, int pBloco, std::string professorId) const
{
    int positionCamada {};

    for (int i = 0; i < camadasTamanho; i++) {

        positionCamada = getPosition(pDia, pBloco, i);
        if (matriz[positionCamada]
                && matriz[positionCamada]->professor->getId() == professorId) {
            return true;
        }
    }

    return false;
}

bool Horario::insert(int pDia, int pBloco, int pCamada, ProfessorDisciplina* pProfessorDisciplina)
{
    if (!pProfessorDisciplina) {
        return true;
    }
    return insert(pDia, pBloco, pCamada, pProfessorDisciplina, false);
}


bool Horario::isViable(int dia, int bloco, int camada, ProfessorDisciplina* pd) const
{
    if (!pd) {
        return true;
    }

    auto pos = getPosition(dia, bloco, camada);
    auto disc = pd->disciplina;

    auto creditos = creditos_alocados_.find(disc->id);
    if (creditos != end(creditos_alocados_) && creditos->second >= disc->cargaHoraria) {
        return false;
    }

    if (!matriz[pos]) {
        auto isProfAloc = colisaoProfessorAlocado(dia, bloco, pd->professor->getId());
        if (!isProfAloc) {
            return true;
        }
    }

    return false;
}

bool Horario::insert(int pDia, int pBloco, int pCamada, ProfessorDisciplina* pProfessorDisciplina, bool force)
{
    int position = getPosition(pDia, pBloco, pCamada);
    bool professorAlocado = false;
    auto disc = pProfessorDisciplina->disciplina;

    if (creditos_alocados_[disc->id] >= disc->cargaHoraria) {
        return false;
    }

    if (!matriz[position] || force) {
        professorAlocado = colisaoProfessorAlocado(pDia, pBloco,
                                                   pProfessorDisciplina->professor->getId());
        if (!professorAlocado || force) {
            creditos_alocados_[disc->id]++;
            return Representacao::insert(pDia, pBloco, pCamada, pProfessorDisciplina, force);
        }
    }
    return false;
}

void Horario::clearSlot(int pDia, int pBloco, int pCamada)
{
    auto pos = getPosition(pDia, pBloco, pCamada);
    auto profdisc = matriz[pos];
    if (!profdisc) {
        return;
    }
    creditos_alocados_[profdisc->disciplina->id]--;
    Representacao::clearSlot(pDia, pBloco, pCamada);
}

void Horario::clearCamada(int camada)
{
    for (auto d = 0; d < dias_semana_util; d++) {
        for (auto b = 0; b < blocosTamanho; b++) {
            clearSlot(d, b, camada);
        }
    }
}

std::size_t Horario::getHash()
{
    if (hash_ != 0) {
        return hash_;
    }
    auto oss = std::ostringstream {};

    for (const auto& pd : matriz) {
        if (pd) {
            oss << pd->disciplina->id + pd->professor->id;
        } else {
            oss << "00";
        }
    }
    hash_ = Util::hash_string(oss.str());
    return hash_;
}

int Horario::contaJanelasDia(int dia, int camada) const
{
    auto janelas = 0;
    auto contando = false;
    auto contador = 0;

    for (auto j = 0; j < blocosTamanho; j++) {
        if (at(dia, j, camada)) {
            if (!contando) {
                contando = true;
            }
            if (contando && contador > 0) {
                janelas++;
                contador = 0;
            }
        } else if (contando) {
            contador++;
        }
    }

    return contador;
}

int Horario::contaJanelasCamada(int camada) const
{
    auto janelas = 0;
    for (auto d = 0; d < dias_semana_util; d++) {
        janelas += contaJanelasDia(d, camada);
    }
    return janelas;
}

int Horario::contaJanelas() const
{
    auto janelas = 0;
    for (auto c = 0; c < camadasTamanho; c++) {
        janelas += contaJanelasCamada(c);
    }
    return janelas;
}

bool Horario::isProfDia(
    const std::string& professor,
    int dia
) const
{
    for (auto c = 0; c < camadasTamanho; c++) {
        for (auto b = 0; b < blocosTamanho; b++) {
            auto pd = at(dia, b, c);
            if (pd && pd->getProfessor()->getId() == professor) {
                return true;
            }
        }
    }
    return false;
}

 bool Horario::intervalosTrabalhoProf(const std::string& professor) const
{
     auto intervalos = 0;
     auto contando = false;
     auto contador = 0;

     for (auto j = 0; j < blocosTamanho; j++) {
         if (isProfDia(professor, j)) {
             if (!contando) {
                 contando = true;
             }
             if (contando && contador > 0) {
                 intervalos++;
                 contador = 0;
             }
         } else if (contando) {
             contador++;
         }
     }

     return contador;
}

int Horario::intervalosTrabalho(
    const std::unordered_map<std::string, Professor*>& professores
) const
{
    auto intervalos = 0;
    for (auto& p : professores) {
        intervalos += intervalosTrabalhoProf(p.second->getId());
    }
    return intervalos;
}

bool Horario::isAulaDia(int dia, int camada) const
{
    for (auto b = 0; b < blocosTamanho; b++) {
        if(at(dia, b, camada)) {
            return true;
        }
    }
    return false;
}

int Horario::numDiasAulaCamada(int camada) const
{
    auto num = 0;
    for (auto d = 0; d < dias_semana_util; d++) {
        num += isAulaDia(d, camada);
    }
    return num;
}

int Horario::numDiasAula() const
{
    auto num = 0;
    for (auto c = 0; c < camadasTamanho; c++) {
        num += numDiasAulaCamada(c);
    }
    return num;
}

int Horario::aulasSabadoCamada(int camada) const
{
    const auto sabado = dias_semana_util - 1;
    auto num = 0;
    for (auto b = 0; b < blocosTamanho; b++) {
        if (at(sabado, b, camada)) {
            num++;
        }
    }
    return num;
}

int Horario::aulasSabado() const
{
    auto aulas = 0;
    for (auto c = 0; c < camadasTamanho; c++) {
        aulas += aulasSabadoCamada(c);
    }
    return aulas;
}

int Horario::aulasSeguidasDiscDia(const std::string& disciplina, int dia) const
{
    auto num = 0;
    auto camada = discCamada.at(disciplina);

    for (auto b = 0; b < blocosTamanho; b++) {
        auto pd = at(dia, b, camada);
        if (pd && pd->getDisciplina()->getId() == disciplina) {
            num++;
        }
    }

    return std::max(num - 2, 0);
}

int Horario::aulasSeguidasDisc(const std::string& disciplina) const
{
    auto num = 0;
    for (auto d = 0; d < dias_semana_util; d++) {
        num += aulasSeguidasDiscDia(disciplina, d);
    }
    return num;
}

int Horario::aulasSeguidas(const std::vector<Disciplina*>& disciplinas) const
{
    auto num = 0;
    for (auto& disc : disciplinas) {
        if (disc->ofertada) {
            num += aulasSeguidasDisc(disc->getId());
        }
    }
    return num;
}

bool Horario::isDiscDificil(const std::string& disciplina) const
{
    return false;
}

int Horario::aulasSeguidasDificilDia(int dia, int camada) const
{
    auto num = 0;

    for (auto b = 0; b < blocosTamanho; b++) {
        auto pd = at(dia, b, camada);
        if (pd && isDiscDificil(pd->getDisciplina()->getId())) {
            num++;
        }
    }

    return std::max(num - 2, 0);
}

int Horario::aulasSeguidasDificilCamada(int camada) const
{
    auto num = 0;
    for (auto d = 0; d < dias_semana_util; d++) {
        num += aulasSeguidasDificilDia(d, camada);
    }
    return num;
}

int Horario::aulasSeguidasDificil() const
{
    auto num = 0;
    for (auto c = 0; c < camadasTamanho; c++) {
        num += aulasSeguidasDificilCamada(c);
    }
    return num;
}

bool Horario::aulaDificilUltimoHorarioDia(int dia, int camada) const
{
    const auto ultimo = blocosTamanho - 2;

    auto pd1 = at(dia, ultimo, camada);
    auto pd2 = at(dia, ultimo + 1, camada);

    auto dif1 = pd1 && isDiscDificil(pd1->getDisciplina()->getId());
    auto dif2 = pd2 && isDiscDificil(pd2->getDisciplina()->getId());

    return dif1 || dif2;
}

int Horario::aulaDificilUltimoHorarioCamada(int camada) const
{
    auto num = 0;
    for (auto d = 0; d < dias_semana_util; d++) {
        num += aulasSeguidasDificilDia(d, camada);
    }
    return num;
}

int Horario::aulaDificilUltimoHorario() const
{
    auto num = 0;
    for (auto c = 0; c < camadasTamanho; c++) {
        num += aulasSeguidasDificilCamada(c);
    }
    return num;
}

bool Horario::isProfPref(const std::string& prof, const std::string& disc) const
{
    return true;
}

int Horario::preferenciasProfessor(const std::string& professor) const
{
    auto num = 0;
    std::unordered_map<std::string, bool> percorrido;

    for (auto c = 0; c < camadasTamanho; c++) {
        for (auto d = 0; d < dias_semana_util; d++) {
            for (auto b = 0; b < blocosTamanho; b++) {
                auto pd = at(d, b, c);
                if (!pd || (pd && pd->getProfessor()->getId() != professor)) {
                    continue;
                }

                auto& disc = pd->getDisciplina()->getId();
                if (percorrido[disc]) {
                    continue;
                }

                percorrido[disc] = true;
                num += isProfPref(professor, disc);
            }
        }
    }

    return num;
}

int Horario::preferenciasProfessores(
    const std::unordered_map<std::string, Professor*>& professores
) const
{
    auto num = 0;
    for (auto& p : professores) {
        num += preferenciasProfessor(p.second->getId());
    }
    return num;
}

int Horario::aulasProfessor(const std::string& professor, int preferencia) const
{
    auto num = 0;

    for (auto c = 0; c < camadasTamanho; c++) {
        for (auto d = 0; d < dias_semana_util; d++) {
            for (auto b = 0; b < blocosTamanho; b++) {
                auto pd = at(d, b, c);
                if (pd && pd->getProfessor()->getId() != professor) {
                    num++;
                }
            }
        }
    }

    auto excesso = num - preferencia;
    return std::max(excesso, 0);
}

int Horario::aulasProfessores(
    const std::unordered_map<std::string, Professor*>& professores
) const
{
    auto num = 0;
    for (auto& p : professores) {
        num += aulasProfessor(p.second->getId(), p.second->preferenciaAulas());
    }
    return num;
}