#ifndef REPRESENTACAO_H
#define REPRESENTACAO_H

#include <string>
#include <vector>
#include <tuple>
#include <faptp-lib/ProfessorDisciplina.h>
#include <faptp-lib/Util.h>
#include <faptp-lib/Constantes.h>

class Representacao
{
    friend class Resolucao;
public:
    Representacao(int pBlocosTamanho, int pCamadasTamanho);
    Representacao(const Representacao& outro);
    Representacao& operator=(const Representacao& outro);
    virtual ~Representacao() = default;

    int getPosition(int pDia, int pBloco, int pCamada) const;

    ProfessorDisciplina* at(int pDia, int pBloco, int pCamada) const;
    ProfessorDisciplina* at(int position) const;

    const std::vector<ProfessorDisciplina*>& getMatriz() const;

    virtual bool insert(int pDia, int pBloco, int pCamada, ProfessorDisciplina* pProfessorDisciplina);
    virtual bool insert(int dia, int bloco, int camada, ProfessorDisciplina* pd, bool force);

    // Bloco, dia, camada
    void get3DMatrix(std::size_t pLinear, int triDimensional[3]);
    // Dia, bloco, camada
    std::tuple<int, int, int> getCoords(std::size_t pLinear) const;

    std::vector<ProfessorDisciplina*>::iterator getFirstDisciplina(std::vector<ProfessorDisciplina*>::iterator iter, std::vector<ProfessorDisciplina*>::iterator iterEnd, Disciplina* pDisciplina);

    int getFirstDisciplina(Disciplina* pDisciplina);
    int getFirstDisciplina(Disciplina* pDisciplina, std::vector<ProfessorDisciplina*> pMatriz);

    std::vector<int> getAllEmpty(int camada);
    void clearDisciplina(const ProfessorDisciplina* pProfessorDisciplina, int camada);
    virtual void clearSlot(int pDia, int pBloco, int pCamda);

    int getPositionDisciplina(std::vector<ProfessorDisciplina*>::iterator iter, std::vector<ProfessorDisciplina*>::iterator iterEnd, std::vector<ProfessorDisciplina*>::iterator iterFound);

    int getBlocosTamanho() const;

protected:
    int blocosTamanho;
    int camadasTamanho;

    int size;

    std::vector<ProfessorDisciplina*> matriz;
    const std::vector<std::tuple<int, int, int>>& coordenadas;
};

inline int Representacao::getPosition(int pDia, int pBloco, int pCamada) const
{
    return pBloco + pDia * blocosTamanho + pCamada * blocosTamanho * dias_semana_util;
}

inline ProfessorDisciplina* Representacao::at(int pDia, int pBloco, int pCamada) const
{
  return at(getPosition(pDia, pBloco, pCamada));
}

inline ProfessorDisciplina* Representacao::at(int position) const
{
  return matriz[position];
}

inline std::tuple<int, int, int> 
Representacao::getCoords(std::size_t pLinear) const
{
  return coordenadas[pLinear];
}

#endif /* REPRESENTACAO_H */

