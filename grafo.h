#ifndef GRAFO_H
#define GRAFO_H

typedef struct grafo Grafo;

Grafo *cria_grafo(int *excessao_zero);
Grafo *preenche_grafo(int *excessao_zero);
int insere_aresta(Grafo *gr, int orig, int dest);
void imprime_matriz(Grafo *gr);
int inicializar(void);
int propriedades(Grafo *gr);
void libera_matriz(int **m, int tam);
int **aloca_matriz(int tam);
void reflexiva(Grafo *gr);
void irreflexiva(Grafo *gr);
void simetrica(Grafo *gr);
void anti_simetrica(Grafo *gr);
void assimetrica(Grafo *gr);
int transitiva(Grafo *gr);
void equivalencia(Grafo *gr);
void ordem_parcial(Grafo *gr);
void fecho_reflexivo(Grafo *gr);
void fecho_simetrico(Grafo *gr);
int fecho_transitivo(Grafo *gr);
void fecho_padrao(Grafo *gr);

#endif
