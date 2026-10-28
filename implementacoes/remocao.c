#include <stdlib.h>
#include <string.h>

#include "busca.h"
#include "criacao.h"
#include "remocao.h"

/* O filho pai->filhos[i] tem chaves a menos: pega uma chave emprestada do
 * irmão esquerdo, passando pelo pai. */
void emprestar_da_esquerda(NoB *pai, int i)
{
    NoB *filho = pai->filhos[i];
    NoB *irmao = pai->filhos[i - 1];

    memmove(&filho->chaves[1], &filho->chaves[0], (size_t)filho->n * sizeof(int));
    if (!filho->folha)
        memmove(&filho->filhos[1], &filho->filhos[0],
                (size_t)(filho->n + 1) * sizeof(NoB *));

    filho->chaves[0] = pai->chaves[i - 1];          /* separadora desce  */
    if (!filho->folha)
        filho->filhos[0] = irmao->filhos[irmao->n];
    pai->chaves[i - 1] = irmao->chaves[irmao->n - 1]; /* chave do irmão sobe */

    filho->n++;
    irmao->n--;
}

/* Idem, pegando emprestado do irmão direito. */
void emprestar_da_direita(NoB *pai, int i)
{
    NoB *filho = pai->filhos[i];
    NoB *irmao = pai->filhos[i + 1];

    filho->chaves[filho->n] = pai->chaves[i];       /* separadora desce  */
    if (!filho->folha)
        filho->filhos[filho->n + 1] = irmao->filhos[0];
    pai->chaves[i] = irmao->chaves[0];              /* chave do irmão sobe */

    memmove(&irmao->chaves[0], &irmao->chaves[1],
            (size_t)(irmao->n - 1) * sizeof(int));
    if (!irmao->folha)
        memmove(&irmao->filhos[0], &irmao->filhos[1],
                (size_t)irmao->n * sizeof(NoB *));

    filho->n++;
    irmao->n--;
}

/* Funde pai->filhos[i] e pai->filhos[i+1], trazendo para baixo a chave
 * separadora pai->chaves[i]. O nó da direita é liberado. */
void fundir_filhos(const ArvoreB *arv, NoB *pai, int i)
{
    NoB *esq = pai->filhos[i];
    NoB *dir = pai->filhos[i + 1];
    (void)arv;

    esq->chaves[esq->n] = pai->chaves[i];
    memcpy(&esq->chaves[esq->n + 1], dir->chaves, (size_t)dir->n * sizeof(int));
    if (!esq->folha)
        memcpy(&esq->filhos[esq->n + 1], dir->filhos,
               (size_t)(dir->n + 1) * sizeof(NoB *));
    esq->n += dir->n + 1;

    /* remove a separadora e o ponteiro para 'dir' do pai */
    memmove(&pai->chaves[i], &pai->chaves[i + 1],
            (size_t)(pai->n - i - 1) * sizeof(int));
    memmove(&pai->filhos[i + 1], &pai->filhos[i + 2],
            (size_t)(pai->n - i - 1) * sizeof(NoB *));
    pai->n--;

    free(dir->chaves);
    free(dir->filhos);
    free(dir);
}

/* Se pai->filhos[i] ficou abaixo do mínimo: tenta emprestar de um irmão;
 * se ambos estiverem no mínimo, funde. */
void corrigir_underflow(const ArvoreB *arv, NoB *pai, int i)
{
    int minimo = arvore_min_chaves(arv);

    if (pai->filhos[i]->n >= minimo)
        return;

    if (i > 0 && pai->filhos[i - 1]->n > minimo)
        emprestar_da_esquerda(pai, i);
    else if (i < pai->n && pai->filhos[i + 1]->n > minimo)
        emprestar_da_direita(pai, i);
    else if (i > 0)
        fundir_filhos(arv, pai, i - 1);
    else
        fundir_filhos(arv, pai, i);
}

static int remover_rec(ArvoreB *arv, NoB *no, int chave)
{
    int i = posicao_no(no, chave);
    int achou = (i < no->n && no->chaves[i] == chave);

    if (no->folha) {
        if (!achou)
            return 0;
        memmove(&no->chaves[i], &no->chaves[i + 1],
                (size_t)(no->n - i - 1) * sizeof(int));
        no->n--;
        return 1;
    }

    if (achou) {
        /* nó interno: troca pela predecessora (maior chave da subárvore
         * esquerda) e remove a predecessora lá embaixo */
        int pred = chave_maxima(no->filhos[i]);
        no->chaves[i] = pred;
        remover_rec(arv, no->filhos[i], pred);
    } else {
        if (!remover_rec(arv, no->filhos[i], chave))
            return 0;
    }

    corrigir_underflow(arv, no, i);
    return 1;
}

int remover(ArvoreB *arv, int chave)
{
    int removeu = remover_rec(arv, arv->raiz, chave);

    /* raiz interna ficou sem chaves: a árvore diminui em altura */
    if (arv->raiz->n == 0 && !arv->raiz->folha) {
        NoB *antiga = arv->raiz;
        arv->raiz = antiga->filhos[0];
        free(antiga->chaves);
        free(antiga->filhos);
        free(antiga);
    }
    return removeu;
}
