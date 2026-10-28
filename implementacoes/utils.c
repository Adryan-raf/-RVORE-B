#include <limits.h>

#include "utils.h"

int altura(const ArvoreB *arv)
{
    int h = 1;
    for (const NoB *no = arv->raiz; !no->folha; no = no->filhos[0])
        h++;
    return h;
}

static int chaves_rec(const NoB *no)
{
    int total = no->n;
    if (!no->folha)
        for (int i = 0; i <= no->n; i++)
            total += chaves_rec(no->filhos[i]);
    return total;
}

static int nos_rec(const NoB *no)
{
    int total = 1;
    if (!no->folha)
        for (int i = 0; i <= no->n; i++)
            total += nos_rec(no->filhos[i]);
    return total;
}

int contar_chaves(const ArvoreB *arv) { return chaves_rec(arv->raiz); }
int contar_nos(const ArvoreB *arv)    { return nos_rec(arv->raiz); }

static void em_ordem_rec(const NoB *no, FILE *saida)
{
    for (int i = 0; i < no->n; i++) {
        if (!no->folha)
            em_ordem_rec(no->filhos[i], saida);
        fprintf(saida, "%d ", no->chaves[i]);
    }
    if (!no->folha)
        em_ordem_rec(no->filhos[no->n], saida);
}

void imprimir_em_ordem(const ArvoreB *arv, FILE *saida)
{
    em_ordem_rec(arv->raiz, saida);
    fputc('\n', saida);
}

static void imprimir_rec(const NoB *no, int nivel, FILE *saida)
{
    for (int i = 0; i < nivel; i++)
        fputs("    ", saida);
    fputc('[', saida);
    for (int i = 0; i < no->n; i++)
        fprintf(saida, i ? " %d" : "%d", no->chaves[i]);
    fputs("]\n", saida);

    if (!no->folha)
        for (int i = 0; i <= no->n; i++)
            imprimir_rec(no->filhos[i], nivel + 1, saida);
}

void imprimir_arvore(const ArvoreB *arv, FILE *saida)
{
    imprimir_rec(arv->raiz, 0, saida);
}

static void coletar_rec(const NoB *no, int *v, int max, int *cont)
{
    for (int i = 0; i < no->n; i++) {
        if (!no->folha)
            coletar_rec(no->filhos[i], v, max, cont);
        if (*cont < max)
            v[(*cont)++] = no->chaves[i];
    }
    if (!no->folha)
        coletar_rec(no->filhos[no->n], v, max, cont);
}

int coletar_chaves(const ArvoreB *arv, int *vetor, int max)
{
    int cont = 0;
    coletar_rec(arv->raiz, vetor, max, &cont);
    return cont;
}

/* Valida a subárvore: chaves estritamente entre (lo, hi), nível das folhas. */
static int validar_rec(const ArvoreB *arv, const NoB *no, long lo, long hi,
                       int nivel, int *nivel_folha, int eh_raiz)
{
    int max = arvore_max_chaves(arv);
    int min = eh_raiz ? 1 : arvore_min_chaves(arv);

    if (no->n > max)
        return 0;
    if (no->n < min && !(eh_raiz && no->folha && no->n == 0))
        return 0;

    for (int i = 0; i < no->n; i++) {
        if (no->chaves[i] <= lo || no->chaves[i] >= hi)
            return 0;
        if (i > 0 && no->chaves[i - 1] >= no->chaves[i])
            return 0;
    }

    if (no->folha) {
        if (*nivel_folha < 0)
            *nivel_folha = nivel;
        return *nivel_folha == nivel;
    }

    for (int i = 0; i <= no->n; i++) {
        long novo_lo = (i == 0) ? lo : no->chaves[i - 1];
        long novo_hi = (i == no->n) ? hi : no->chaves[i];
        if (!no->filhos[i])
            return 0;
        if (!validar_rec(arv, no->filhos[i], novo_lo, novo_hi,
                         nivel + 1, nivel_folha, 0))
            return 0;
    }
    return 1;
}

int validar_arvore(const ArvoreB *arv)
{
    int nivel_folha = -1;
    return validar_rec(arv, arv->raiz, (long)INT_MIN - 1, (long)INT_MAX + 1,
                       0, &nivel_folha, 1);
}
