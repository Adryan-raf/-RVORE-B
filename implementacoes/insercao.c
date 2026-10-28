#include <string.h>

#include "busca.h"
#include "criacao.h"
#include "insercao.h"

NoB *dividir_no(const ArvoreB *arv, NoB *no, int *mediana)
{
    int meio = no->n / 2;
    NoB *dir = criar_no(arv, no->folha);
    if (!dir)
        return NULL;

    *mediana = no->chaves[meio];

    /* chaves à direita da mediana vão para o novo nó */
    dir->n = no->n - meio - 1;
    memcpy(dir->chaves, &no->chaves[meio + 1], (size_t)dir->n * sizeof(int));
    if (!no->folha)
        memcpy(dir->filhos, &no->filhos[meio + 1],
               (size_t)(dir->n + 1) * sizeof(NoB *));

    no->n = meio; /* o nó original fica só com a metade esquerda */
    return dir;
}

/* Insere recursivamente na subárvore de 'no'.
 * Se o nó estourar, devolve em *mediana e *novo_dir o resultado do split
 * para que o chamador (pai) os incorpore. */
static int inserir_rec(ArvoreB *arv, NoB *no, int chave,
                       int *mediana, NoB **novo_dir)
{
    int i = posicao_no(no, chave);

    if (i < no->n && no->chaves[i] == chave)
        return INS_DUPLICADA;

    *novo_dir = NULL;

    if (no->folha) {
        memmove(&no->chaves[i + 1], &no->chaves[i],
                (size_t)(no->n - i) * sizeof(int));
        no->chaves[i] = chave;
        no->n++;
    } else {
        int med_filho;
        NoB *dir_filho;
        int r = inserir_rec(arv, no->filhos[i], chave, &med_filho, &dir_filho);

        if (r != INS_OK)
            return r;
        if (dir_filho) { /* o filho dividiu: incorpora a mediana aqui */
            memmove(&no->chaves[i + 1], &no->chaves[i],
                    (size_t)(no->n - i) * sizeof(int));
            memmove(&no->filhos[i + 2], &no->filhos[i + 1],
                    (size_t)(no->n - i) * sizeof(NoB *));
            no->chaves[i] = med_filho;
            no->filhos[i + 1] = dir_filho;
            no->n++;
        }
    }

    if (no->n > arvore_max_chaves(arv)) { /* overflow: divide */
        *novo_dir = dividir_no(arv, no, mediana);
        if (!*novo_dir)
            return INS_SEM_MEMORIA;
    }
    return INS_OK;
}

int inserir(ArvoreB *arv, int chave)
{
    int mediana;
    NoB *dir;
    int r = inserir_rec(arv, arv->raiz, chave, &mediana, &dir);

    if (r != INS_OK)
        return r;

    if (dir) { /* a raiz dividiu: a árvore cresce em altura */
        NoB *nova = criar_no(arv, 0);
        if (!nova)
            return INS_SEM_MEMORIA;
        nova->chaves[0] = mediana;
        nova->filhos[0] = arv->raiz;
        nova->filhos[1] = dir;
        nova->n = 1;
        arv->raiz = nova;
    }
    return INS_OK;
}
