#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include "arvore_b.h"

/* Altura da árvore em níveis (árvore só com a raiz folha = 1). */
int altura(const ArvoreB *arv);

/* Quantidade total de chaves e de nós. */
int contar_chaves(const ArvoreB *arv);
int contar_nos(const ArvoreB *arv);

/* Imprime as chaves em ordem crescente (percurso em ordem). */
void imprimir_em_ordem(const ArvoreB *arv, FILE *saida);

/* Imprime a estrutura da árvore, um nó por linha, com indentação por nível. */
void imprimir_arvore(const ArvoreB *arv, FILE *saida);

/* Copia as chaves em ordem para 'vetor' (capacidade 'max'); retorna quantas. */
int coletar_chaves(const ArvoreB *arv, int *vetor, int max);

/* Verifica todas as propriedades da árvore B:
 *   - chaves ordenadas e dentro dos intervalos definidos pelos ancestrais
 *   - ocupação entre mínimo e máximo (raiz com pelo menos 1 chave)
 *   - todas as folhas no mesmo nível
 * Retorna 1 se a árvore é válida, 0 caso contrário. */
int validar_arvore(const ArvoreB *arv);

#endif /* UTILS_H */
