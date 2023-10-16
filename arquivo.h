#ifndef __ARQUIVO_H__
#define __ARQUIVO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define ERRO(...) {         \
    printf("ERRO: ");       \
    printf(__VA_ARGS__);    \
    exit(1);                \
}

#define memcpy_range(dest, src, from, to) { \
    for (int x = (from); x < (to); x++) {   \
        dest[x - (from)] = src[x];          \
    }                                       \
}

#define memfill(arr, from, to, val) {       \
    for (int x = (from); x < (to); x++) {   \
        arr[x] = val;                       \
    }                                       \
}

#define insere_posicao(arr, tam, i, val) {  \
   for (int x = (tam) - 1; x > (i); x--) {  \
        arr[x] = arr[x - 1];                \
    }                                       \
    arr[(i)] = val;                         \
}

// -----------------------------------------------------------------------------

typedef struct {
    char tipo;
    char arg[200];
} operacao;

// le uma operacao do arquivo de operacoes
operacao ler_op(FILE *fd);

// -----------------------------------------------------------------------------

int ler_registro(FILE *fd, char *str);
int ler_campo(FILE *fd, char *str);
short get_tam_registro(FILE *fd);
void checar_cabecalho(FILE *fd);
char fpeek(FILE *fd);

int insere_reg(FILE *fd, char *reg);

#endif
