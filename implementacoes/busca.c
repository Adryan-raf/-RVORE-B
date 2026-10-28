#include <stddef.h>

#include "busca.h"

int posicao_no(const NoB *no, int chave)
{
    int ini = 0, fim = no->n; /* procura no intervalo [ini, fim) */

    while (ini < fim) {
        int meio = ini + (fim - ini) / 2;
        if (no->chaves[meio] < chave)
            ini = meio + 1;
        else
            fim = meio;
    }
    return ini;
}

NoB *buscar(const ArvoreB *arv, int chave, int *pos)
{
    NoB *no = arv->raiz;

    while (no) {
        int i = posicao_no(no, chave);

        if (i < no->n && no->chaves[i] == chave) {
            if (pos)
                *pos = i;
            return no;
        }
        if (no->folha)
            return NULL;
        no = no->filhos[i]; /* desce uma página por nível */
    }
    return NULL;
}

int contem(const ArvoreB *arv, int chave)
{
    return buscar(arv, chave, NULL) != NULL;
}

int chave_minima(const NoB *no)
{
    while (!no->folha)
        no = no->filhos[0];
    return no->chaves[0];
}

int chave_maxima(const NoB *no)
{
    while (!no->folha)
        no = no->filhos[no->n];
    return no->chaves[no->n - 1];
}
