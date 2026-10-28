#ifndef ARVORE_B_H
#define ARVORE_B_H

/*
 * Árvore B de ordem m (m = número máximo de filhos por nó).
 *
 *   - máximo de chaves por nó : m - 1
 *   - mínimo de chaves por nó : ceil(m/2) - 1   (exceto a raiz)
 *   - raiz: no mínimo 1 chave (se a árvore não estiver vazia)
 *   - todas as folhas ficam no mesmo nível
 *
 * Chaves são inteiros e não podem se repetir.
 * Cada nó reserva uma posição extra de chave e de filho para tolerar o
 * "overflow" momentâneo que antecede a divisão (split) do nó.
 */

#define ORDEM_MINIMA 3

typedef struct NoB {
    int n;               /* quantidade atual de chaves                 */
    int folha;           /* 1 se o nó é folha, 0 caso contrário        */
    int *chaves;         /* vetor de chaves ordenadas (capacidade m)   */
    struct NoB **filhos; /* vetor de filhos (capacidade m + 1)         */
} NoB;

typedef struct {
    NoB *raiz;
    int ordem;           /* m */
} ArvoreB;

/* Limites de ocupação derivados da ordem. */
int arvore_max_chaves(const ArvoreB *arv);
int arvore_min_chaves(const ArvoreB *arv);

#endif /* ARVORE_B_H */
