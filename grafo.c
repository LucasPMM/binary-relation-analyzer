/*
Trabalho desenvolvido por Lucas Paulo Martins Mariz - Técnico em informática formado pelo Coltec/UFMG E Aluno de Ciência da Computação UFMG
*/
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grafo.h"

#define MAX_VERTICES 50
#define INPUT_BUFFER_SIZE 4096

struct grafo {
    int **matriz_adjacencia;
    int n_vertices;
    int elementos[MAX_VERTICES];
    int n_ligacoes;

    int propriedade_reflexiva;
    int propriedade_irreflexiva;
    int propriedade_simetrica;
    int propriedade_anti_simetrica;
    int propriedade_assimetrica;
    int propriedade_transitiva;

    int relacao_equivalencia;
    int relacao_ordem_parcial;
};

static int proximo_inteiro(const char **cursor, int *valor) {
    char *fim;
    long numero;

    while (isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }

    if (**cursor == '\0') {
        return 0;
    }

    errno = 0;
    numero = strtol(*cursor, &fim, 10);
    if (fim == *cursor || errno == ERANGE || numero < INT_MIN || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;
    *cursor = fim;
    return 1;
}

static int contem_apenas_espacos(const char *cursor) {
    while (isspace((unsigned char)*cursor)) {
        cursor++;
    }
    return *cursor == '\0';
}

static int linha_excedeu_buffer(const char *linha, FILE *arquivo) {
    return strchr(linha, '\n') == NULL && !feof(arquivo);
}

static int **calcula_fecho_transitivo(const Grafo *gr) {
    int i, j, k;
    int **fecho = aloca_matriz(gr->n_vertices);

    if (gr->n_vertices > 0 && fecho == NULL) {
        return NULL;
    }

    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            fecho[i][j] = gr->matriz_adjacencia[i][j];
        }
    }

    for (k = 0; k < gr->n_vertices; k++) {
        for (i = 0; i < gr->n_vertices; i++) {
            if (fecho[i][k] == 0) {
                continue;
            }
            for (j = 0; j < gr->n_vertices; j++) {
                if (fecho[k][j] == 1) {
                    fecho[i][j] = 1;
                }
            }
        }
    }

    return fecho;
}

Grafo *cria_grafo(int *excessao_zero) {
    if (excessao_zero == NULL) {
        return NULL;
    }
    return preenche_grafo(excessao_zero);
}

void libera_matriz(int **m, int tam) {
    int i;

    if (m == NULL) {
        return;
    }

    for (i = 0; i < tam; i++) {
        free(m[i]);
    }
    free(m);
}

Grafo *preenche_grafo(int *excessao_zero) {
    FILE *arq;
    Grafo *gr;
    char info[INPUT_BUFFER_SIZE];
    const char *cursor;
    int i, j, n_vertices;
    size_t numero_linha = 1;

    if (excessao_zero == NULL) {
        return NULL;
    }
    *excessao_zero = 0;

    arq = fopen("dados.txt", "r");
    if (arq == NULL) {
        fprintf(stderr, "Erro, nao foi possivel abrir o arquivo dados.txt\n");
        return NULL;
    }

    gr = calloc(1, sizeof(*gr));
    if (gr == NULL) {
        fprintf(stderr, "Erro, nao foi possivel alocar o grafo\n");
        fclose(arq);
        return NULL;
    }

    for (i = 0; i < MAX_VERTICES; i++) {
        gr->elementos[i] = -1;
    }

    if (fgets(info, sizeof(info), arq) == NULL || linha_excedeu_buffer(info, arq)) {
        fprintf(stderr, "Erro de entrada na linha 1: cabecalho ausente ou muito longo\n");
        goto erro;
    }

    cursor = info;
    if (!proximo_inteiro(&cursor, &n_vertices) || n_vertices < 0 || n_vertices > MAX_VERTICES) {
        fprintf(stderr, "Erro de entrada na linha 1: numero de elementos invalido\n");
        goto erro;
    }
    gr->n_vertices = n_vertices;

    for (i = 0; i < gr->n_vertices; i++) {
        if (!proximo_inteiro(&cursor, &gr->elementos[i])) {
            fprintf(stderr, "Erro de entrada na linha 1: quantidade de elementos incorreta\n");
            goto erro;
        }
        for (j = 0; j < i; j++) {
            if (gr->elementos[j] == gr->elementos[i]) {
                fprintf(stderr, "Erro de entrada na linha 1: elementos duplicados\n");
                goto erro;
            }
        }
    }

    if (!contem_apenas_espacos(cursor)) {
        fprintf(stderr, "Erro de entrada na linha 1: quantidade de elementos incorreta\n");
        goto erro;
    }

    if (gr->n_vertices > 0) {
        gr->matriz_adjacencia = aloca_matriz(gr->n_vertices);
        if (gr->matriz_adjacencia == NULL) {
            fprintf(stderr, "Erro, nao foi possivel alocar a matriz de adjacencia\n");
            goto erro;
        }
    }

    while (fgets(info, sizeof(info), arq) != NULL) {
        int origem, destino;

        numero_linha++;
        if (linha_excedeu_buffer(info, arq)) {
            fprintf(stderr, "Erro de entrada na linha %zu: linha muito longa\n", numero_linha);
            goto erro;
        }

        cursor = info;
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor == '\0') {
            continue;
        }

        if (!proximo_inteiro(&cursor, &origem) || !proximo_inteiro(&cursor, &destino) ||
            !contem_apenas_espacos(cursor)) {
            fprintf(stderr, "Erro de entrada na linha %zu: par ordenado invalido\n", numero_linha);
            goto erro;
        }

        if (!insere_aresta(gr, origem, destino)) {
            fprintf(stderr, "Erro de entrada na linha %zu: elemento desconhecido\n", numero_linha);
            goto erro;
        }
    }

    if (ferror(arq)) {
        fprintf(stderr, "Erro durante a leitura de dados.txt\n");
        goto erro;
    }

    gr->propriedade_reflexiva = 1;
    gr->propriedade_irreflexiva = 1;
    gr->propriedade_simetrica = 1;
    gr->propriedade_anti_simetrica = 1;
    gr->propriedade_assimetrica = 1;
    gr->propriedade_transitiva = 1;
    gr->relacao_equivalencia = 1;
    gr->relacao_ordem_parcial = 1;
    *excessao_zero = gr->n_vertices == 0;

    fclose(arq);
    return gr;

erro:
    fclose(arq);
    libera_matriz(gr->matriz_adjacencia, gr->n_vertices);
    free(gr);
    return NULL;
}

int insere_aresta(Grafo *gr, int orig, int dest) {
    int i;
    int nova_origem = -1;
    int novo_destino = -1;

    if (gr == NULL) {
        return 0;
    }

    for (i = 0; i < gr->n_vertices; i++) {
        if (gr->elementos[i] == orig) {
            nova_origem = i;
        }
        if (gr->elementos[i] == dest) {
            novo_destino = i;
        }
    }

    if (nova_origem < 0 || novo_destino < 0) {
        return 0;
    }

    if (gr->matriz_adjacencia[nova_origem][novo_destino] == 0) {
        gr->matriz_adjacencia[nova_origem][novo_destino] = 1;
        gr->n_ligacoes++;
    }
    return 1;
}

void imprime_matriz(Grafo *gr) {
    int i, j;

    printf("\n\t");
    for (i = 0; i < gr->n_vertices; i++) {
        printf("%d ", gr->elementos[i]);
    }

    printf("\n\n\n");
    for (i = 0; i < gr->n_vertices; i++) {
        printf("%d\t", gr->elementos[i]);
        for (j = 0; j < gr->n_vertices; j++) {
            printf("%d ", gr->matriz_adjacencia[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

int **aloca_matriz(int tam) {
    int i;
    int **m;

    if (tam <= 0) {
        return NULL;
    }

    m = calloc((size_t)tam, sizeof(*m));
    if (m == NULL) {
        return NULL;
    }

    for (i = 0; i < tam; i++) {
        m[i] = calloc((size_t)tam, sizeof(*m[i]));
        if (m[i] == NULL) {
            libera_matriz(m, i);
            return NULL;
        }
    }
    return m;
}

void reflexiva(Grafo *gr) {
    int i;

    gr->propriedade_reflexiva = 1;
    for (i = 0; i < gr->n_vertices; i++) {
        if (gr->matriz_adjacencia[i][i] == 0) {
            gr->propriedade_reflexiva = 0;
        }
    }

    if (gr->propriedade_reflexiva == 1) {
        printf("1. Reflexiva: V\n");
        return;
    }

    printf("1. Reflexiva: F\n");
    for (i = 0; i < gr->n_vertices; i++) {
        if (gr->matriz_adjacencia[i][i] == 0) {
            printf("(%d,%d); ", gr->elementos[i], gr->elementos[i]);
        }
    }
    printf("\n");
}

void irreflexiva(Grafo *gr) {
    int i;

    gr->propriedade_irreflexiva = 1;
    for (i = 0; i < gr->n_vertices; i++) {
        if (gr->matriz_adjacencia[i][i] == 1) {
            gr->propriedade_irreflexiva = 0;
        }
    }

    if (gr->propriedade_irreflexiva == 1) {
        printf("2. Irreflexiva: V\n");
        return;
    }

    printf("2. Irreflexiva: F\n");
    for (i = 0; i < gr->n_vertices; i++) {
        if (gr->matriz_adjacencia[i][i] == 1) {
            printf("(%d,%d); ", gr->elementos[i], gr->elementos[i]);
        }
    }
    printf("\n");
}

void simetrica(Grafo *gr) {
    int i, j;

    gr->propriedade_simetrica = 1;
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            if (gr->matriz_adjacencia[i][j] == 1 && gr->matriz_adjacencia[j][i] == 0) {
                gr->propriedade_simetrica = 0;
            }
        }
    }

    if (gr->propriedade_simetrica == 1) {
        printf("3. Simetrica: V\n");
        return;
    }

    printf("3. Simetrica: F\n");
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            if (gr->matriz_adjacencia[i][j] == 1 && gr->matriz_adjacencia[j][i] == 0) {
                printf("(%d,%d) e (%d,%d); ", gr->elementos[i], gr->elementos[j],
                       gr->elementos[j], gr->elementos[i]);
            }
        }
    }
    printf("\n");
}

void anti_simetrica(Grafo *gr) {
    int i, j;

    gr->propriedade_anti_simetrica = 1;
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = i + 1; j < gr->n_vertices; j++) {
            if (gr->matriz_adjacencia[i][j] == 1 && gr->matriz_adjacencia[j][i] == 1) {
                gr->propriedade_anti_simetrica = 0;
            }
        }
    }

    if (gr->propriedade_anti_simetrica == 1) {
        printf("4. Anti-simetrica: V\n");
        return;
    }

    printf("4. Anti-simetrica: F\n");
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = i + 1; j < gr->n_vertices; j++) {
            if (gr->matriz_adjacencia[i][j] == 1 && gr->matriz_adjacencia[j][i] == 1) {
                printf("(%d,%d) e (%d,%d); ", gr->elementos[i], gr->elementos[j],
                       gr->elementos[j], gr->elementos[i]);
            }
        }
    }
    printf("\n");
}

void assimetrica(Grafo *gr) {
    int i, j;

    gr->propriedade_assimetrica = 1;
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            if (gr->matriz_adjacencia[i][j] == 1 && gr->matriz_adjacencia[j][i] == 1) {
                gr->propriedade_assimetrica = 0;
            }
        }
    }

    if (gr->propriedade_assimetrica == 1) {
        printf("5. Asimetrica: V\n");
    } else {
        printf("5. Asimetrica: F\n");
    }
}

int transitiva(Grafo *gr) {
    int i, j;
    int **fecho = calcula_fecho_transitivo(gr);

    if (gr->n_vertices > 0 && fecho == NULL) {
        fprintf(stderr, "Erro, nao foi possivel calcular o fecho transitivo\n");
        return -1;
    }

    gr->propriedade_transitiva = 1;
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            if (fecho[i][j] == 1 && gr->matriz_adjacencia[i][j] == 0) {
                gr->propriedade_transitiva = 0;
            }
        }
    }

    if (gr->propriedade_transitiva == 1) {
        printf("6. Transitiva: V\n");
    } else {
        printf("6. Transitiva: F\n");
        for (i = 0; i < gr->n_vertices; i++) {
            for (j = 0; j < gr->n_vertices; j++) {
                if (fecho[i][j] == 1 && gr->matriz_adjacencia[i][j] == 0) {
                    printf("(%d,%d); ", gr->elementos[i], gr->elementos[j]);
                }
            }
        }
        printf("\n");
    }

    libera_matriz(fecho, gr->n_vertices);
    return 0;
}

void equivalencia(Grafo *gr) {
    gr->relacao_equivalencia = gr->propriedade_reflexiva == 1 &&
                               gr->propriedade_simetrica == 1 &&
                               gr->propriedade_transitiva == 1;
    printf("Relaçao de equivalencia: %c\n", gr->relacao_equivalencia ? 'V' : 'F');
}

void ordem_parcial(Grafo *gr) {
    gr->relacao_ordem_parcial = gr->propriedade_reflexiva == 1 &&
                                gr->propriedade_anti_simetrica == 1 &&
                                gr->propriedade_transitiva == 1;
    printf("Relaçao de ordem parcial: %c\n", gr->relacao_ordem_parcial ? 'V' : 'F');
}

void fecho_padrao(Grafo *gr) {
    int i, j;
    bool primeiro = true;

    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            if (gr->matriz_adjacencia[i][j] == 1) {
                if (!primeiro) {
                    printf(",");
                }
                printf("(%d,%d)", gr->elementos[i], gr->elementos[j]);
                primeiro = false;
            }
        }
    }
}

void fecho_reflexivo(Grafo *gr) {
    int i;
    bool precisa_virgula = gr->n_ligacoes > 0;

    printf("Fecho reflexivo da relaçao = {");
    fecho_padrao(gr);
    for (i = 0; i < gr->n_vertices; i++) {
        if (gr->matriz_adjacencia[i][i] == 0) {
            if (precisa_virgula) {
                printf(",");
            }
            printf("(%d,%d)", gr->elementos[i], gr->elementos[i]);
            precisa_virgula = true;
        }
    }
    printf("}\n");
}

void fecho_simetrico(Grafo *gr) {
    int i, j;
    bool precisa_virgula = gr->n_ligacoes > 0;

    printf("Fecho simetrico da relaçao = {");
    fecho_padrao(gr);
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            if (gr->matriz_adjacencia[i][j] == 1 && gr->matriz_adjacencia[j][i] == 0) {
                if (precisa_virgula) {
                    printf(",");
                }
                printf("(%d,%d)", gr->elementos[j], gr->elementos[i]);
                precisa_virgula = true;
            }
        }
    }
    printf("}\n");
}

int fecho_transitivo(Grafo *gr) {
    int i, j;
    bool precisa_virgula = gr->n_ligacoes > 0;
    int **fecho = calcula_fecho_transitivo(gr);

    if (gr->n_vertices > 0 && fecho == NULL) {
        fprintf(stderr, "Erro, nao foi possivel calcular o fecho transitivo\n");
        return -1;
    }

    printf("Fecho transitivo da relaçao = {");
    fecho_padrao(gr);
    for (i = 0; i < gr->n_vertices; i++) {
        for (j = 0; j < gr->n_vertices; j++) {
            if (fecho[i][j] == 1 && gr->matriz_adjacencia[i][j] == 0) {
                if (precisa_virgula) {
                    printf(",");
                }
                printf("(%d,%d)", gr->elementos[i], gr->elementos[j]);
                precisa_virgula = true;
            }
        }
    }
    printf("}\n");

    libera_matriz(fecho, gr->n_vertices);
    return 0;
}

int propriedades(Grafo *gr) {
    printf("\n\nPropriedades:\n\n");
    imprime_matriz(gr);
    reflexiva(gr);
    irreflexiva(gr);
    simetrica(gr);
    anti_simetrica(gr);
    assimetrica(gr);
    if (transitiva(gr) != 0) {
        return -1;
    }
    printf("\n");
    equivalencia(gr);
    ordem_parcial(gr);
    printf("\n");
    fecho_reflexivo(gr);
    fecho_simetrico(gr);
    if (fecho_transitivo(gr) != 0) {
        return -1;
    }
    printf("\n");
    return 0;
}

int inicializar(void) {
    int excessao_zero;
    Grafo *gr = cria_grafo(&excessao_zero);
    int resultado;

    if (gr == NULL) {
        return EXIT_FAILURE;
    }

    resultado = propriedades(gr);
    libera_matriz(gr->matriz_adjacencia, gr->n_vertices);
    free(gr);
    return resultado == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
