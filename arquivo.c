#include "arquivo.h"

operacao ler_op(FILE *fd) {
    operacao op;

    op.tipo = fgetc(fd);
    if (fgetc(fd) != ' ') assert(false);

    fgets(op.arg, 200, fd);

    if (op.arg[strlen(op.arg) - 1] == '\n')
        op.arg[strlen(op.arg) - 1] = '\0';
    else
        op.arg[strlen(op.arg)] = '\0';

    return op;
}

// le caracter a caracter de um registro ate encontrar um *
// retorna o tamanho do registro lido
int ler_registro(FILE *fd, char *str) {
    checar_cabecalho(fd);

    short tamanho_registro = get_tam_registro(fd, -1);
    int i = 0;
    char c;

    while(i < tamanho_registro) {
        c = fgetc(fd);
        if (c == '*') break;
        str[i++] = c;
    }

    str[i] = '\0';
    return i;
}

// le caracter a caracter de um campo ate encontrar um |
// retorna o tamanho do campo lido
int ler_campo(FILE *fd, char *str) {
    checar_cabecalho(fd);
    int i = 0;
    char c;

    while ((c = fgetc(fd)) != '|' && c != '*') {
        str[i++] = c;
    }

    if (c == '*') str[0] = '\0';
    str[i] = '\0';
    return i + 1;
}

short get_tam_registro(FILE *fd, int offset) {
    if (offset >= 0) {
        fseek(fd, offset, SEEK_SET);
    }

    short tamanho_registro;
    fread(&tamanho_registro, sizeof(short), 1, fd);

    return tamanho_registro;
}

void checar_cabecalho(FILE *fd) {
    int pos = ftell(fd);
    if (pos < 0) assert(false);
    if (pos < 4)
        fseek(fd, 4, SEEK_SET);
}

char fpeek(FILE *fd) {
    char c = fgetc(fd);
    ungetc(c, fd);

    return c;
}

int insere_reg(FILE *fd, char *reg) {
    short tam_reg = strlen(reg);

    fseek(fd, 0, SEEK_END);
    int offset = ftell(fd);
    fwrite(&tam_reg, sizeof(short), 1, fd);
    fputs(reg, fd);

    return offset;
}