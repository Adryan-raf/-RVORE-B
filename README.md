# Árvore B em C

Implementação didática de uma **Árvore B** em memória, escrita em C99, com os procedimentos de criação, busca, inserção e remoção separados em arquivos `.h` e `.c`. Foi feita para acompanhar o workshop *Árvores B e B+*.

## Sumário

1. [Conceitos usados](#conceitos-usados)
4. [Procedimentos](#procedimentos)
   - [Estruturas de dados](#estruturas-de-dados)
   - [Criação](#criação)
   - [Busca](#busca)
   - [Inserção](#inserção)
   - [Remoção](#remoção)
6. [Limitações e extensões](#limitações-e-extensões)

```

## Como compilar e executar

Rode os comandos no terminal, na pasta raiz do projeto (onde fica o `Makefile`):

```bash
make          # gera ./arvore_b (e ./teste, se teste.c existir)
make run      # compila e executa a demonstração (ordem 5)
make test     # compila e executa o teste aleatório
make clean    # remove binários e a pasta build/
```

Também é possível passar a ordem da árvore: `./arvore_b 7`. É necessário um compilador C com suporte a C99 (por exemplo, `gcc`).

## Procedimentos

### Estruturas de dados

Arquivo `includes/arvore_b.h`. Cada nó guarda suas chaves ordenadas e os ponteiros para os filhos; a árvore guarda a raiz e a ordem.

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

Os limites de ocupação são calculados a partir da ordem:

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

Arquivos `includes/criacao.h` e `implementacoes/criacao.c`.

#### `criar_no`

Aloca um nó vazio. As chaves têm capacidade `m` (uma a mais que o máximo) e os filhos `m + 1`, para absorver o overflow antes da divisão.

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
```

#### `criar_arvore`

Valida a ordem (mínimo 3) e cria a árvore com a raiz como uma folha vazia.

```c
ArvoreB *criar_arvore(int ordem)
{
    if (ordem < ORDEM_MINIMA)
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
```

### Busca

#### `posicao_no`

Busca binária dentro de um nó: devolve a primeira posição `i` com `chaves[i] >= chave`.

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
```

#### `buscar` e `contem`

Em cada nó, encontra a posição da chave. Se a chave está ali, retorna o nó; se o nó é folha, a chave não existe; senão desce para `filhos[i]`.

```c
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
```

### Inserção

Fluxo: desce até a folha, insere a chave em ordem e, se o nó passar de `m - 1` chaves (*overflow*), divide-o: a **mediana sobe** para o pai. A divisão pode se repetir até a raiz; se a raiz dividir, nasce uma nova raiz e a altura aumenta em 1.

#### `inserir`

Ponto de entrada. Se a divisão chegar até a raiz, cria uma nova raiz com a mediana e os dois nós resultantes. Retorna `INS_OK`, `INS_DUPLICADA` ou `INS_SEM_MEMORIA`.

```c
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

Arquivos `includes/remocao.h` e `implementacoes/remocao.c`.

Fluxo: se a chave está em uma folha, é removida direto. Se está em um nó interno, é trocada pela **predecessora** (maior chave da subárvore esquerda), que então é removida da folha. Depois, o filho afetado pode ficar abaixo do mínimo (*underflow*); nesse caso tenta-se **emprestar** de um irmão e, se nenhum puder emprestar, **fundir** os nós.

#### `emprestar_da_esquerda`

O filho `i` está com chaves a menos: a chave separadora do pai desce para ele e a maior chave do irmão esquerdo sobe para o pai.

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
```

#### `emprestar_da_direita`

Simétrico ao anterior, usando o irmão direito.

```c
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
```

#### `fundir_filhos`

Une os filhos `i` e `i + 1`, trazendo para baixo a chave separadora do pai. O nó da direita é liberado.

```c
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
```

#### `corrigir_underflow`

Decide entre emprestar do irmão esquerdo, emprestar do direito ou fundir.

```c
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
```

#### `remover_rec` (função interna)

Remove recursivamente e corrige o underflow do filho por onde desceu.

```c
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
```

#### `remover`

Ponto de entrada. Se a raiz ficar sem chaves e não for folha, o único filho vira a nova raiz e a altura diminui em 1. Retorna 1 se removeu e 0 se a chave não existia.

```c
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

### Utilitários e validação

Arquivos `includes/utils.h` e `implementacoes/utils.c`.

#### `altura`, `contar_chaves` e `contar_nos`

```c
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
```

#### `imprimir_arvore` e `imprimir_em_ordem`

`imprimir_arvore` mostra um nó por linha, com indentação por nível; `imprimir_em_ordem` lista as chaves em ordem crescente.

```c
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
```

#### `validar_arvore`

Confere todas as propriedades da Árvore B: chaves ordenadas e dentro dos intervalos definidos pelos ancestrais, ocupação entre o mínimo e o máximo e todas as folhas no mesmo nível.

```c
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
```

### Exemplo de uso

```c
#include <stdio.h>
#include "criacao.h"
#include "busca.h"
#include "insercao.h"
#include "remocao.h"
#include "utils.h"

int main(void)
{
    ArvoreB *arv = criar_arvore(5);

    for (int k = 10; k <= 100; k += 10)
        inserir(arv, k);

    imprimir_arvore(arv, stdout);
    printf("tem 50? %s\n", contem(arv, 50) ? "sim" : "não");

    remover(arv, 50);
    imprimir_em_ordem(arv, stdout);

    destruir_arvore(arv);
    return 0;
}
```

Compile o exemplo junto com os módulos:

```bash
gcc -std=c99 -Iincludes exemplo.c implementacoes/*.c -o exemplo
```

## Testes

`teste.c` executa, para cada ordem de 3 a 12, milhares de inserções e remoções aleatórias. A cada operação ele confere o resultado contra um vetor de referência, a validade da árvore com `validar_arvore`, a quantidade de chaves e o resultado da busca. No final, esvazia a árvore por completo.

```bash
make test
```

Para checar também a memória:

```bash
gcc -std=c99 -g -fsanitize=address,undefined -Iincludes -o teste_asan teste.c implementacoes/*.c
./teste_asan
```

## Demonstração

O `main.c` insere `10, 20, 30, 40, 50, 25, 35, 45, 55` em uma árvore de ordem 5 (exercício do slide de conclusão), mostrando a árvore a cada passo. Ao final, a raiz fica `[30 45]` com as folhas `[10 20 25]`, `[35 40]` e `[50 55]`. Depois faz buscas e remove algumas chaves para exibir empréstimos e fusões.

## Limitações e extensões

- A árvore guarda apenas chaves inteiras, sem registros associados.
- Tudo fica em memória; não há persistência em disco.
- Possíveis extensões: Árvore B+ (dados nas folhas encadeadas), Árvore B\* (ocupação mínima de 2/3), chaves genéricas com função de comparação e consultas por intervalo.
