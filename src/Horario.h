#ifndef HORARIO_H
#define HORARIO_H

#include "Representacao.h"

class Horario : public Representacao {
    friend class Resolucao;
    friend class Grade;
public:
    Horario(int pBlocosTamanho, int pCamadasTamanho);
    Horario(const Horario& outro);
    Horario& operator=(const Horario& outro);
    virtual ~Horario();
    
    bool colisaoProfessorAlocado(int pDia, int pBloco, std::string professorId);
    
    bool insert(int pDia, int pBloco, int pCamada, ProfessorDisciplina* pProfessorDisciplina);
    bool insert(int pDia, int pBloco, int pCamada, ProfessorDisciplina* pProfessorDisciplina, bool force);

    double getObjectiveFunction();

	std::size_t getHash();
private:
	std::size_t hash_;
};

#endif /* HORARIO_H */

