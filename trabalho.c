// Nícolas dos Santos Carvalho - 128660
// Hudson Henrique da Silva    - 128849
// Ciencia da Computacao - UEM
// 18/10/2023

// --------------------------------------------------------------------------------------------------------------------------

#include "arquivo.h"
#include <math.h>

#define ORDEM 5
#define NO_NULO (No){.chave = -1, .offset = -1}

// --------------------------------------------------------------------------------------------------------------------------

typedef enum {
    JA_EXISTE,
    SEM_PROMOCAO,
    PROMOCAO
} INSERIR_FLAG;

typedef enum {
    NA_PAGINA,
    DESCER
} BUSCAR_FLAG;

typedef struct {
    BUSCAR_FLAG tipo;
    int posicao;
} ResultadoBusca;

typedef struct {
    int chave;
    int offset;
} No;

typedef struct {
    int id;
    No nos[ORDEM - 1];
    int filhas[ORDEM];
} Pagina;

Pagina pagina_criar(int id) {
    Pagina p = {0};
    p.id = id;

    for (int i = 0; i < ORDEM - 1; i++)
        p.nos[i] = NO_NULO;

    for (int i = 0; i < ORDEM; i++)
        p.filhas[i] = -1;

    return p;
}

bool pagina_livre(Pagina *pag) {
    int i = 0;
    while (i < ORDEM - 1 && pag->nos[i].chave > 0) i++;
    return i < ORDEM - 1;
}

ResultadoBusca pagina_busca(Pagina *pag, int chave) {
    int i = 0;
    while (i < ORDEM - 1 && pag->nos[i].chave > 0 && pag->nos[i].chave < chave) i++;

    if (
        i >= ORDEM - 1 ||
        pag->nos[i].chave != chave
    ) return (ResultadoBusca){.tipo = DESCER, .posicao = i};

    return (ResultadoBusca){.tipo = NA_PAGINA, .posicao = i};
}

void pagina_divide(
    Pagina *pag, int chave, int offset, int filho, Pagina *nova_pagina, int *filho_dir, No *no_ref, int novo_id
) {
    No novos_no[ORDEM] = {0};
    int novos_filhos[ORDEM + 1] = {0};

    memcpy_range(novos_no, pag->nos, 0, ORDEM - 1);
    novos_no[ORDEM - 1] = NO_NULO;

    memcpy_range(novos_filhos, pag->filhas, 0, ORDEM);
    novos_filhos[ORDEM] = -1;

    // insere ordenado
    int i = 0;
    while (i < ORDEM - 1 && novos_no[i].chave > 0 && novos_no[i].chave < chave) i++;

    No n = (No){.chave = chave, .offset = offset};
    insere_posicao(novos_no, ORDEM, i, n);
    insere_posicao(novos_filhos, ORDEM + 1, i + 1, filho);
    
    // cria e seta corretamente as paginas
    *nova_pagina = pagina_criar(novo_id);
    int meio = ORDEM / 2;

    // reseta a pagina para ter os [0..meio - 1] elementos 
    memcpy_range(pag->nos, novos_no, 0, meio);
    memcpy_range(pag->filhas, novos_filhos, 0, meio + 1);
    
    // limpar nos e filhos da pag anterior
    memfill(pag->nos, meio, ORDEM - 1, NO_NULO);
    memfill(pag->filhas, meio + 1, ORDEM, -1);

    // seta o no_ref que subira para ser o [meio]
    *no_ref = novos_no[meio];
    *filho_dir = novo_id;    

    // reseta a pagina nova para ter os [meio + 1..] elementos 
    memcpy_range(nova_pagina->nos, novos_no, meio + 1, ORDEM);
    memcpy_range(nova_pagina->filhas, novos_filhos, meio + 1, ORDEM + 1);
}

int pagina_inserir(Pagina *pag, int chave, int offset, int filho_dir) {
    ResultadoBusca resultado = pagina_busca(pag, chave);
    
    if (resultado.tipo == NA_PAGINA) return -1;
    if (resultado.posicao >= ORDEM - 1) return 0;

    int i = resultado.posicao;
    No n = (No){.chave = chave, .offset = offset};
    insere_posicao(pag->nos, ORDEM - 1, i, n);
    insere_posicao(pag->filhas, ORDEM, i + 1, filho_dir);

    return 0; 
}

// --------------------------------------------------------------------------------------------------------------------------

void executa_op(FILE *dados, FILE* btree, char *path);
int op_buscar(FILE *btree, int id);
int op_inserir(FILE *btree, int id, int offset);

INSERIR_FLAG arvore_inserir(FILE *btree, int id, int offset, Pagina *pag, int *filho_dir, No *proximo);
int arvore_buscar(FILE *btree, int id, Pagina *pag);
void arvore_construir(FILE *dados);
void arvore_print(FILE *btree);
void arvore_escrever_pagina(FILE *btree, Pagina *pag);
Pagina *arvore_ler_pagina(FILE *btree, int offset);

void seek_offset(FILE *btree, int offset);
short tamanho_arvore(FILE *btree);
void escreve_tamanho(FILE *btree, short tam);
short raiz_arvore(FILE *btree);
void escreve_raiz(FILE *btree, short tam);

int main(int argc, char *argv[]) {
    FILE *dados = fopen("dados50.dat", "rb+");
    if (dados == NULL) ERRO("Impossivel abrir 'dados100.dat'\n");
    if (argc < 2) ERRO("E preciso inlcuir '-c'; '-e <arquivo>'; '-p'\n");

    if (!strcmp(argv[1], "-c")) { 
        arvore_construir(dados);
        printf("Arvore construida com sucesso!\n");

    } else if (!strcmp(argv[1], "-e")) {
        if (argc < 3) ERRO("E preciso incluir o arquivo de execucao\n");
        FILE *btree = fopen("btree.dat", "rb+");
        if (btree == NULL) ERRO("Impossivel abrir 'btree.dat'\n");
        // executa_op(dados, btree, "op-teste.txt");
        executa_op(dados, btree, argv[2]);
        fclose(btree);
    
    } else if (!strcmp(argv[1], "-p")) {
        FILE *btree = fopen("btree.dat", "rb+");
        if (btree == NULL) ERRO("Impossivel abrir 'btree.dat'\n");
        arvore_print(btree);
        printf("Impressão realizada com sucesso!\n");
        fclose(btree);

    } else {
        ERRO("comando nao conhecido '%s'\n", argv[1]);
    }

    fclose(dados);
    return 0;
}

// executa operacao a operacao do arquivo de operacoes
void executa_op(FILE *dados, FILE* btree, char *path) {
    FILE *fd = fopen(path, "r");
    if (fd == NULL) ERRO("Impossivel abrir '%s'", path);
    char buffer[200] = "\0";

    while(fpeek(fd) != EOF) {
        operacao op = ler_op(fd);
        char id[200];
        strcpy(id, op.arg);
        strtok(id, "|");

        switch(op.tipo) {
            case 'b':
                printf("Busca pelo registro de chave \"%s\"\n", id);

                int pos = op_buscar(btree, atoi(id));

                if (pos < 0) {
                    printf("Erro: registro nao encontrado!\n\n");
                } else {
                    fseek(dados, pos, SEEK_SET);
                    ler_registro(dados, buffer);
                    printf("%s (%i bytes - offset %i)\n\n", buffer, (int)strlen(buffer), pos);
                }
                break;
            case 'i':
                printf("Insercao do registro de chave \"%s\"\n", id);
                int offset = insere_reg(dados, op.arg);
                int inserir = op_inserir(btree, atoi(id), offset);

                if (inserir < 0)
                    printf("Erro: chave \"%s\" ja existente\n\n", id);
                else
                    printf("%s (%li bytes - offset %i)\n\n", op.arg, strlen(op.arg), offset);
                break;
            default:
                printf("argumento invalido! (%c)", op.tipo);
                assert(false);
        }
    }

    fclose(fd);
}

void arvore_construir(FILE *dados) {
    FILE *btree = fopen("btree.dat", "wr+");
    if (btree == NULL) ERRO("Impossivel abrir 'btree.dat'\n");
    
    short tam = 0;
    short raiz = -1;

    escreve_raiz(btree, raiz);
    escreve_tamanho(btree, tam);

    char id[10] = "\0";
    int desloc = 0;
    int offset = 4;
    short tam_campo = 0;

    fseek(dados, offset, SEEK_SET);

    while (fpeek(dados) != EOF) {
        tam_campo = get_tam_registro(dados);
        desloc = ler_campo(dados, id);

        printf("%s\n", id);
        op_inserir(btree, atoi(id), offset);

        offset += 2 + tam_campo;
        fseek(dados, (int)tam_campo - desloc, SEEK_CUR);
    }

    fclose(btree);
}

int op_buscar(FILE *btree, int id) {
    short raiz = raiz_arvore(btree);
    Pagina *pagina_raiz = arvore_ler_pagina(btree, raiz);

    int resultado = arvore_buscar(btree, id, pagina_raiz);

    free(pagina_raiz);
    return resultado;
}

int arvore_buscar(FILE *btree, int id, Pagina *pag) {
    if (pag == NULL)
        return -1;
    
    ResultadoBusca resultado = pagina_busca(pag, id);

    if (resultado.tipo == NA_PAGINA)
        return pag->nos[resultado.posicao].offset;
    
    Pagina *prox_pag = arvore_ler_pagina(btree, pag->filhas[resultado.posicao]);
    int offset = arvore_buscar(btree, id, prox_pag);

    free(prox_pag);
    return offset;
}

int op_inserir(FILE *btree, int id, int offset) {
    short raiz = raiz_arvore(btree);
    Pagina *pagina_raiz = arvore_ler_pagina(btree, raiz);    

    int fd;
    No no;
    INSERIR_FLAG resultado = arvore_inserir(btree, id, offset, pagina_raiz, &fd, &no);

    if (resultado == JA_EXISTE) {
        free(pagina_raiz);
        return -1;
    }

    if (resultado == PROMOCAO) {
        short novo_id = tamanho_arvore(btree);
        Pagina pag = pagina_criar(novo_id);
        pagina_inserir(&pag, no.chave, no.offset, fd);
        pag.filhas[0] = raiz;
        
        escreve_raiz(btree, novo_id);
        escreve_tamanho(btree, novo_id + 1);
        arvore_escrever_pagina(btree, &pag);
    }
    
    free(pagina_raiz);
    return 0;
}

INSERIR_FLAG arvore_inserir(FILE *btree, int id, int offset, Pagina *pag, int *filho_dir, No *proximo) {
    if (pag == NULL) {
        *proximo = (No){.chave = id, .offset = offset};
        *filho_dir = -1;

        return PROMOCAO;
    }

    ResultadoBusca resultado = pagina_busca(pag, id);
    
    if (resultado.tipo == NA_PAGINA)
        return JA_EXISTE;
    
    Pagina *prox_pag = arvore_ler_pagina(btree, pag->filhas[resultado.posicao]);
    int fd = -1;
    No no = {0};
    INSERIR_FLAG flag = arvore_inserir(btree, id, offset, prox_pag, &fd, &no);
    free(prox_pag);

    if (flag == PROMOCAO) {
        if (pagina_livre(pag)) {
            // printf("%i %i %i\n", no.chave, no.offset, fd);
            pagina_inserir(pag, no.chave, no.offset, fd);
            arvore_escrever_pagina(btree, pag);

            return SEM_PROMOCAO;
        } else {
            Pagina nova_pagina = {0};
            pagina_divide(
                pag, 
                no.chave, no.offset, fd, 
                &nova_pagina, filho_dir, proximo, 
                tamanho_arvore(btree)
            );
            arvore_escrever_pagina(btree, pag);
            arvore_escrever_pagina(btree, &nova_pagina);

            escreve_tamanho(btree, tamanho_arvore(btree) + 1);

            return PROMOCAO;
        }
    } 

    // flag = SEM_PROMOCAO | JA_EXISTE    
    return flag;
}

void arvore_print(FILE *btree) {
    short tamanho = tamanho_arvore(btree);
    short raiz = raiz_arvore(btree);

    for (int i = 0; i < tamanho; i++) {
        Pagina *pag = arvore_ler_pagina(btree, i);

        if (pag->id == raiz) printf("- - - - - - Raiz - - - - - -\n");
        printf("Pagina %i\n", pag->id);
        
        printf("Chaves: ");
        int j = 0;
        while(j < ORDEM - 1 && pag->nos[j].chave > -1) {
            printf("%i", pag->nos[j].chave);
            
            j++;
            if (j < ORDEM -1 && pag->nos[j].chave > -1) printf(" | ");
        }
        printf("\n");
        
        printf("Offsets: ");
        j = 0;
        while(j < ORDEM - 1 && pag->nos[j].offset > -1) {
            printf("%i", pag->nos[j].offset);
            
            j++;
            if (j < ORDEM -1 && pag->nos[j].offset > -1) printf(" | ");
        }
        printf("\n");
        
        printf("Filhos: ");
        for (int k = 0; k <= j; k++) {
            printf("%i", pag->filhas[k]);
            
            if (k + 1 <= j) printf(" | ");
        }
        printf("\n");
        if (pag->id == raiz) printf("- - - - - - - - - - - - - - -\n");
        printf("\n");

        free(pag);
    }
}

void arvore_escrever_pagina(FILE *btree, Pagina *pag) {
    seek_offset(btree, pag->id);
    fwrite(pag, sizeof(Pagina), 1, btree);
}

Pagina *arvore_ler_pagina(FILE *btree, int offset) {
    if (offset < 0) return NULL;
    
    Pagina *pag = malloc(sizeof(Pagina));
    
    seek_offset(btree, offset);
    fread(pag, sizeof(Pagina), 1, btree);

    return pag;
}

void seek_offset(FILE *btree, int offset) {
    fseek(btree, 4 + sizeof(Pagina)*offset, SEEK_SET);
}

short tamanho_arvore(FILE *btree) {
    int pos = ftell(btree);
    short tam = 0;
    fseek(btree, 0, SEEK_SET);
    fread(&tam, sizeof(short), 1, btree);
    fseek(btree, pos, SEEK_SET);
    return tam;
}

void escreve_tamanho(FILE *btree, short tam) {
    int pos = ftell(btree);
    fseek(btree, 0, SEEK_SET);
    fwrite(&tam, sizeof(short), 1, btree);
    fseek(btree, pos, SEEK_SET);
}

short raiz_arvore(FILE *btree) {
    int pos = ftell(btree);
    short raiz = 0;
    fseek(btree, 2, SEEK_SET);
    fread(&raiz, sizeof(short), 1, btree);
    fseek(btree, pos, SEEK_SET);
    return raiz;
}

void escreve_raiz(FILE *btree, short raiz) {
    int pos = ftell(btree);
    fseek(btree, 2, SEEK_SET);
    fwrite(&raiz, sizeof(short), 1, btree);
    fseek(btree, pos, SEEK_SET);
}