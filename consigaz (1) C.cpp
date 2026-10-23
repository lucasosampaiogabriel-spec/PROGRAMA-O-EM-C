	/* Sistema de estoque para Consigaz - PIM (versao em C) */
#include <stdio.h>
#include <string.h>

int main() { 
       /* Login correto */
    char USUARIO[] = "kauan";
    char SENHA[] = "1202";

    /* O que o usuario digita */
    char usuario[20];
    char senha[20];

    int logado = 0;   /* 0 = ainda nao entrou, 1 = entrou */
    int escolha;

    printf("=======BEM VINDO A CONSIGAZ=======\n");
    printf("=== SISTEMA DE ESTOQUE ===\n");

    while (logado == 0) {
        printf("Usuario: ");
        scanf("%s", usuario);
        printf("Senha: ");
        scanf("%s", senha);

        if (strcmp(usuario, USUARIO) == 0 && strcmp(senha, SENHA) == 0) {
            logado = 1;
        } else {
            printf("Usuario ou senha incorretos!\n");
            printf("1 - Tentar novamente\n");
            printf("2 - Sair\n");
            printf("Escolha: ");
            scanf("%d", &escolha);

            if (escolha == 2) {
                return 0;
            }
        }
    }

    printf("\nLogin realizado com sucesso!\n");

    /* Estoque com 3 produtos (posicao 0 = codigo 1, posicao 1 = codigo 2, ...) */
    char nome[3][10] = {"P13", "P20", "P45"};
    int quantidade[3] = {90, 35, 10};
    int preco[3] = {100, 220, 380};

    int opcao = 0;
    int codigo, qtd, i;

    while (opcao != 4) {
        printf("\n=== MENU ===\n");
        printf("1 - Ver estoque\n");
        printf("2 - Adicionar quantidade\n");
        printf("3 - Remover quantidade\n");
        printf("4 - Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        if (opcao == 1) {
            printf("\n=== ESTOQUE ===\n");
            for (i = 0; i < 3; i++) {
                printf("Codigo: %d | Produto: %s | Qtd: %d | Preco: R$%d\n",
                       i + 1, nome[i], quantidade[i], preco[i]);
            }
        }
        else if (opcao == 2) {
            printf("Codigo do produto: ");
            scanf("%d", &codigo);

            if (codigo >= 1 && codigo <= 3) {
                printf("Quantidade para adicionar: ");
                scanf("%d", &qtd);
                quantidade[codigo - 1] += qtd;
                printf("Quantidade adicionada com sucesso!\n");
            } else {
                printf("Produto nao encontrado!\n");
            }
        }
        else if (opcao == 3) {
            printf("Codigo do produto: ");
            scanf("%d", &codigo);

            if (codigo >= 1 && codigo <= 3) {
                printf("Quantidade para remover: ");
                scanf("%d", &qtd);

                if (qtd <= quantidade[codigo - 1]) {
                    quantidade[codigo - 1] -= qtd;
                    printf("Quantidade removida com sucesso!\n");
                } else {
                    printf("Estoque insuficiente!\n");
                }
            } else {
                printf("Produto nao encontrado!\n");
            }
        }
        else if (opcao == 4) {
            printf("Sistema encerrado.\n");
        }
        else {
            printf("Opcao invalida!\n");
        }
    }

    return 0;
}
