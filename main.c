/* Demonstração e teste da Árvore B.
 *
 * Uso: ./arvore_b [ordem]      (ordem padrão = 5)
 */
#include <stdio.h>
#include <stdlib.h>

#include "busca.h"
#include "criacao.h"
#include "insercao.h"
#include "remocao.h"
#include "utils.h"

static void mostrar(const ArvoreB *arv, const char *titulo)
{
    printf("\n== %s ==\n", titulo);
    imprimir_arvore(arv, stdout);
    printf("chaves: %d | nós: %d | altura: %d | válida: %s\n",
           contar_chaves(arv), contar_nos(arv), altura(arv),
           validar_arvore(arv) ? "sim" : "NÃO");
}

int main(int argc, char **argv)
{
    int ordem = (argc > 1) ? atoi(argv[1]) : 5;
    ArvoreB *arv = criar_arvore(ordem);

    if (!arv) {
        fprintf(stderr, "Ordem inválida (mínimo %d) ou sem memória.\n",
                ORDEM_MINIMA);
        return 1;
    }
    printf("Árvore B de ordem %d (%d a %d chaves por nó)\n", ordem,
           arvore_min_chaves(arv), arvore_max_chaves(arv));

    /* inserção: exercício do slide de conclusão */
    int entrada[] = {10, 20, 30, 40, 50, 25, 35, 45, 55};
    int qtd = (int)(sizeof entrada / sizeof entrada[0]);

    for (int i = 0; i < qtd; i++) {
        inserir(arv, entrada[i]);
        char titulo[64];
        snprintf(titulo, sizeof titulo, "após inserir %d", entrada[i]);
        mostrar(arv, titulo);
    }

    printf("\ninserir 30 de novo: %s\n",
           inserir(arv, 30) == INS_DUPLICADA ? "duplicada (ignorada)" : "?");

    /* busca */
    int pos;
    NoB *no = buscar(arv, 45, &pos);
    printf("buscar 45: %s", no ? "encontrada" : "não encontrada");
    if (no)
        printf(" (nó com %d chaves, posição %d)", no->n, pos);
    printf("\nbuscar 99: %s\n", contem(arv, 99) ? "encontrada" : "não encontrada");
    printf("mínima: %d | máxima: %d\n", chave_minima(arv->raiz),
           chave_maxima(arv->raiz));
    printf("em ordem: ");
    imprimir_em_ordem(arv, stdout);

    /* remoção */
    int saida[] = {10, 30, 55, 40, 25};
    for (int i = 0; i < 5; i++) {
        remover(arv, saida[i]);
        char titulo[64];
        snprintf(titulo, sizeof titulo, "após remover %d", saida[i]);
        mostrar(arv, titulo);
    }
    printf("\nremover 999: %s\n", remover(arv, 999) ? "removida" : "não existia");

    destruir_arvore(arv);
    return 0;
}
