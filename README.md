# Árvores B e suas variações 

## Sumário

1. [Introdução](#introdução)
2. [Propriedades](#propriedades)
3. [Código](#código)
   - [Estruturas de dados](#estruturas-de-dados)
   - [Criação](#criação)
   - [Busca](#busca)
   - [Inserção](#inserção)
   - [Remoção](#remoção)
4. [Integrantes](#integrantes)

## Introdução

A **Árvore B** é uma estrutura de dados em árvore de busca **balanceada** e **multi-caminho**: em vez de dois filhos, como numa árvore binária, cada nó guarda várias chaves ordenadas e possui vários filhos. Ela foi proposta por Bayer e McCreight na década de 1970 para organizar grandes volumes de dados armazenados em disco.

O gargalo desse tipo de armazenamento é o número de acessos ao disco, e não o número de comparações. Por isso, cada nó da Árvore B é pensado para ocupar um bloco (página) de disco, guardando muitas chaves de uma vez. Com nós largos, a árvore fica **baixa**: um milhão de chaves cabem em cerca de 4 níveis numa árvore de ordem 100, contra cerca de 20 numa árvore binária balanceada. Assim, busca, inserção e remoção custam poucas leituras de disco, em tempo **O(log_m n)**.

A árvore se mantém sempre balanceada porque cresce e diminui **pela raiz**: a inserção divide (*split*) nós cheios e a remoção empresta chaves de irmãos ou funde (*merge*) nós com poucas chaves. Esta implementação funciona em memória, com chaves inteiras, e é a base para variações como a Árvore B+ e a Árvore B\*.

## Propriedades

Uma Árvore B de **ordem m** (m = número máximo de filhos por nó) satisfaz:

| Propriedade | Valor |
|---|---|
| Máximo de filhos por nó | `m` |
| Máximo de chaves por nó | `m - 1` |
| Mínimo de chaves por nó (exceto a raiz) | `(m/2) - 1` |
| Mínimo de filhos por nó interno (exceto a raiz) | `(m/2)` |
| Raiz | no mínimo 1 chave e, se for nó interno, no mínimo 2 filhos |
| Nó interno com `k` filhos | possui `k - 1` chaves |
| Ordenação | as chaves de cada nó estão em ordem crescente e separam as subárvores dos filhos |
| Folhas | todas no mesmo nível |
| Altura | cresce ou diminui apenas quando a raiz é dividida ou esvaziada |
| Chaves | inteiros sem repetição (ordem mínima aceita: 3) |

Exemplo, ordem 5 (de 2 a 4 chaves por nó, exceto na raiz):

```
          __________[30 45]______
         |             |         |
    [10 20 25]     [35 40]    [50 55]
```

## Código

### Estruturas de dados

Cada nó guarda a quantidade de chaves, se é folha, o vetor de chaves e o vetor de filhos. Os vetores têm uma posição extra para tolerar o *overflow* momentâneo que antecede uma divisão.

```c
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
```

Limites de ocupação calculados a partir da ordem:

```c
int arvore_max_chaves(const ArvoreB *arv)
{
    return arv->ordem - 1;
}

int arvore_min_chaves(const ArvoreB *arv)
{
    return (arv->ordem + 1) / 2 - 1; /* ceil(m/2) - 1 */
}
```

### Criação

Cria a árvore vazia: valida a ordem (mínimo 3) e inicia a raiz como uma folha sem chaves.

```c
NoB *criar_no(const ArvoreB *arv, int folha)
{
    NoB *no = malloc(sizeof(NoB));
    if (!no)
        return NULL;

    no->n = 0;
    no->folha = folha;
    /* +1 de folga para o overflow antes do split */
    no->chaves = calloc((size_t)arv->ordem, sizeof(int));
    no->filhos = calloc((size_t)arv->ordem + 1, sizeof(NoB *));

    if (!no->chaves || !no->filhos) {
        free(no->chaves);
        free(no->filhos);
        free(no);
        return NULL;
    }
    return no;
}

ArvoreB *criar_arvore(int ordem)
{
    if (ordem < 3)
        return NULL;

    ArvoreB *arv = malloc(sizeof(ArvoreB));
    if (!arv)
        return NULL;

    arv->ordem = ordem;
    arv->raiz = criar_no(arv, 1); /* raiz começa como folha vazia */
    if (!arv->raiz) {
        free(arv);
        return NULL;
    }
    return arv;
}

void destruir_no(NoB *no)
{
    if (!no)
        return;
    if (!no->folha) {
        for (int i = 0; i <= no->n; i++)
            destruir_no(no->filhos[i]);
    }
    free(no->chaves);
    free(no->filhos);
    free(no);
}

void destruir_arvore(ArvoreB *arv)
{
    if (!arv)
        return;
    destruir_no(arv->raiz);
    free(arv);
}
```

### Busca

A busca desce da raiz até uma folha. Em cada nó, uma busca binária acha a primeira posição `i` com `chaves[i] >= chave`; se a chave está ali, termina; se o nó é folha, a chave não existe; senão, desce para `filhos[i]`. Custo: **O(log_m n)** acessos.

```c
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
```

### Inserção

1. Desce até a folha onde a chave deve ficar e a insere em ordem.
2. Se o nó passar de `m - 1` chaves (*overflow*), `dividir_no` o parte ao meio: a **mediana sobe** para o pai.
3. A divisão pode se repetir de nível em nível. Se a **raiz** dividir, nasce uma nova raiz e a altura da árvore aumenta em 1.

```c
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
```

### Remoção

1. Se a chave está em uma folha, é removida direto.
2. Se está em um nó interno, é trocada pela **predecessora** (maior chave da subárvore esquerda), que então é removida da folha.
3. Se o nó ficar abaixo do mínimo (*underflow*), `corrigir_underflow` **empresta** uma chave de um irmão que tenha sobra ou, se os irmãos estiverem no mínimo, **funde** os dois nós trazendo para baixo a chave separadora do pai.
4. Se a raiz ficar sem chaves e não for folha, o único filho vira a nova raiz e a altura diminui em 1.

```c
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
```

## Integrantes

- Luana Alves
- Adryan Albuquerque
