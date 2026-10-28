# Árvore B em C

Implementação didática de uma **Árvore B** em memória, escrita em C, com os procedimentos básicos (criação, busca, inserção e remoção) separados em arquivos `.h` e `.c`. 

## Conceitos usados

A árvore tem **ordem m**, definida como o número máximo de filhos por nó.

| Propriedade | Valor |
|---|---|
| Máximo de chaves por nó | `m - 1` |
| Mínimo de chaves por nó (exceto a raiz) | `ceil(m/2) - 1` |
| Mínimo de chaves na raiz | 1 (se a árvore não estiver vazia) |
| Folhas | todas no mesmo nível |

- As chaves são inteiros e **não podem se repetir**.
- A ordem mínima aceita é 3 (`ORDEM_MINIMA`).
- Cada nó reserva uma posição extra de chave e de filho para tolerar o *overflow* momentâneo que antecede uma divisão.

## Estrutura do projeto

| Arquivo | Responsabilidade |
|---|---|
| `arvore_b.h` | Estruturas `NoB` e `ArvoreB`, constante `ORDEM_MINIMA` e limites de ocupação |
| `criacao.h` / `criacao.c` | Criação e destruição de nós e da árvore |
| `busca.h` / `busca.c` | Busca por chave, mínimo e máximo, busca binária dentro do nó |
| `insercao.h` / `insercao.c` | Inserção e divisão (split) de nós |
| `remocao.h` / `remocao.c` | Remoção, empréstimo e fusão de nós |
| `utils.h` / `utils.c` | Altura, contagens, impressão, coleta de chaves e validação da árvore |
| `main.c` | Programa de demonstração |
| `teste.c` | Teste aleatório automatizado |
| `Makefile` | Compilação |

## Como compilar e executar

```bash
make                # gera ./arvore_b e ./teste
./arvore_b 5        # demonstração com ordem 5 (o argumento é opcional)
./teste             # teste aleatório (opcional: ./teste <operacoes> <semente>)
make clean          # remove binários e objetos
```

É necessário um compilador C com suporte a C99 (por exemplo, `gcc`).

## API

### Criação (`criacao.h`)

```c
ArvoreB *criar_arvore(int ordem);   // NULL se a ordem for inválida ou faltar memória
void     destruir_arvore(ArvoreB *arv);
int      arvore_vazia(const ArvoreB *arv);
NoB     *criar_no(const ArvoreB *arv, int folha);
void     destruir_no(NoB *no);
```

### Busca (`busca.h`)

```c
NoB *buscar(const ArvoreB *arv, int chave, int *pos); // nó que contém a chave, ou NULL
int  contem(const ArvoreB *arv, int chave);           // 1 se existe, 0 caso contrário
int  chave_minima(const NoB *no);
int  chave_maxima(const NoB *no);
int  posicao_no(const NoB *no, int chave);            // busca binária dentro de um nó
```

### Inserção (`insercao.h`)

```c
int inserir(ArvoreB *arv, int chave);
// Retorna INS_OK, INS_DUPLICADA ou INS_SEM_MEMORIA
NoB *dividir_no(const ArvoreB *arv, NoB *no, int *mediana);
```

### Remoção (`remocao.h`)

```c
int remover(ArvoreB *arv, int chave);  // 1 se removeu, 0 se a chave não existia
void emprestar_da_esquerda(NoB *pai, int i);
void emprestar_da_direita(NoB *pai, int i);
void fundir_filhos(const ArvoreB *arv, NoB *pai, int i);
void corrigir_underflow(const ArvoreB *arv, NoB *pai, int i);
```

### Utilidades (`utils.h`)

```c
int  altura(const ArvoreB *arv);
int  contar_chaves(const ArvoreB *arv);
int  contar_nos(const ArvoreB *arv);
void imprimir_arvore(const ArvoreB *arv, FILE *saida);
void imprimir_em_ordem(const ArvoreB *arv, FILE *saida);
int  coletar_chaves(const ArvoreB *arv, int *vetor, int max);
int  validar_arvore(const ArvoreB *arv); // confere todas as propriedades da árvore B
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
gcc -std=c99 exemplo.c criacao.c busca.c insercao.c remocao.c utils.c -o exemplo
```

## Como os algoritmos funcionam

### Busca

Em cada nó, uma busca binária acha a primeira posição `i` com `chaves[i] >= chave`. Se a chave está ali, termina; se o nó é folha, a chave não existe; caso contrário, desce para `filhos[i]`. O custo é de **O(log_m n)** acessos, uma página por nível.

### Inserção

1. Desce recursivamente até a folha correta.
2. Insere a chave em ordem na folha.
3. Se o nó passou de `m - 1` chaves (*overflow*), `dividir_no` o parte ao meio: a **mediana sobe** para o pai e o nó original fica com a metade esquerda; as chaves à direita vão para um novo nó.
4. A divisão pode se repetir de nível em nível. Se a **raiz** dividir, nasce uma nova raiz e a altura da árvore aumenta em 1.

### Remoção

1. Se a chave está em uma folha, ela é removida direto.
2. Se está em um nó interno, é trocada pela sua **predecessora** (maior chave da subárvore esquerda), que é então removida da folha.
3. Após remover, o filho afetado pode ficar abaixo do mínimo (*underflow*) e `corrigir_underflow` resolve:
   - **empréstimo:** se um irmão tem chaves sobrando, uma chave sobe ao pai e a chave separadora desce ao nó;
   - **fusão:** se os irmãos estão no mínimo, os dois nós se unem trazendo para baixo a chave separadora do pai.
4. Se a raiz ficar sem chaves e não for folha, o único filho vira a nova raiz e a altura diminui em 1.

## Testes

`teste.c` executa, para cada ordem de 3 a 12, milhares de inserções e remoções aleatórias. A cada operação ele confere:

- o resultado da operação contra um vetor de referência;
- a propriedade da árvore com `validar_arvore` (chaves ordenadas e dentro dos intervalos dos ancestrais, ocupação entre o mínimo e o máximo, folhas no mesmo nível);
- a quantidade de chaves e o resultado da busca.

No final, esvazia a árvore por completo. Para checar também a memória:

```bash
gcc -std=c99 -g -fsanitize=address,undefined -o teste_asan \
    teste.c criacao.c busca.c insercao.c remocao.c utils.c
./teste_asan
```

## Demonstração (`main.c`)

O programa insere `10, 20, 30, 40, 50, 25, 35, 45, 55` em uma árvore de ordem 5 (exercício do slide de conclusão), mostrando a árvore a cada passo. Ao final, a raiz fica `[30 45]` com as folhas `[10 20 25]`, `[35 40]` e `[50 55]`. Depois ele faz buscas e remove algumas chaves para exibir empréstimos e fusões.

## Limitações e extensões possíveis

- A árvore guarda apenas chaves inteiras, sem registros associados.
- Tudo fica em memória; não há persistência em disco.
- Possíveis extensões: Árvore B+ (dados nas folhas encadeadas), Árvore B\* (ocupação mínima de 2/3), chaves genéricas com função de comparação e consultas por intervalo.

