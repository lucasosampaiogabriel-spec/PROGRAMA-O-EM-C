/* Sistema de estoque para Consigaz - PIM II (versao em C com niveis de acesso) */
#include <stdio.h>
#include <string.h>

#define MAX_USUARIOS 20

/* Niveis: 1 = Administrador, 2 = Atendente, 3 = Entregador */
struct Usuario {
    char nome[20];
    char senha[20];
    int nivel;
    int ativo;      /* 1 = ativo, 0 = inativo */
};

struct Usuario usuarios[MAX_USUARIOS] = {
    {"lucas", "0407", 1, 1}
};
int totalUsuarios = 1;
int usuarioAtual = -1;   /* posicao do usuario logado */

/* Estoque (posicao 0 = codigo 1, posicao 1 = codigo 2, ...) */
char nomeProduto[3][10] = {"P13", "P20", "P45"};
int quantidade[3] = {90, 35, 10};
int preco[3] = {100, 220, 380};

/* Pedidos. Status: 1 = Pendente, 2 = Em entrega, 3 = Concluido */
#define MAX_PEDIDOS 50

struct Pedido {
    int id;
    char cliente[30];
    int produto;      /* codigo do produto: 1 a 3 */
    int quantidade;
    int status;
    int ativo;        /* 1 = existe, 0 = excluido */
};

struct Pedido pedidos[MAX_PEDIDOS];
int totalPedidos = 0;
int proximoId = 1;

/* Mostra o nome do nivel */
void mostrarNivel(int nivel) {
    if (nivel == 1) {
        printf("Administrador");
    } else if (nivel == 2) {
        printf("Atendente");
    } else {
        printf("Entregador");
    }
}

/* Procura um usuario pelo nome. Devolve a posicao ou -1 se nao achar */
int buscarUsuario(char nomeBusca[]) {
    int i;
    for (i = 0; i < totalUsuarios; i++) {
        if (strcmp(usuarios[i].nome, nomeBusca) == 0) {
            return i;
        }
    }
    return -1;
}

/* ---------- ESTOQUE ---------- */

void verEstoque() {
    int i;
    printf("\n=== ESTOQUE ===\n");
    for (i = 0; i < 3; i++) {
        printf("Codigo: %d | Produto: %s | Qtd: %d | Preco: R$%d\n",
               i + 1, nomeProduto[i], quantidade[i], preco[i]);
    }
}

void adicionarEstoque() {
    int codigo, qtd;
    printf("Codigo do produto: ");
    scanf("%d", &codigo);

    if (codigo >= 1 && codigo <= 3) {
        printf("Quantidade para adicionar: ");
        scanf("%d", &qtd);
        if (qtd > 0) {
            quantidade[codigo - 1] += qtd;
            printf("Quantidade adicionada com sucesso!\n");
        } else {
            printf("Quantidade invalida!\n");
        }
    } else {
        printf("Produto nao encontrado!\n");
    }
}

void removerEstoque() {
    int codigo, qtd;
    printf("Codigo do produto: ");
    scanf("%d", &codigo);

    if (codigo >= 1 && codigo <= 3) {
        printf("Quantidade para remover: ");
        scanf("%d", &qtd);

        if (qtd <= 0) {
            printf("Quantidade invalida!\n");
        } else if (qtd <= quantidade[codigo - 1]) {
            quantidade[codigo - 1] -= qtd;
            printf("Quantidade removida com sucesso!\n");
        } else {
            printf("Estoque insuficiente!\n");
        }
    } else {
        printf("Produto nao encontrado!\n");
    }
}

/* ---------- USUARIOS (so Administrador) ---------- */

void gerenciarUsuarios() {
    int op = -1;
    int i, pos, nivel;
    char nomeBusca[20];

    while (op != 0) {
        printf("\n=== GERENCIAR USUARIOS ===\n");
        printf("1 - Cadastrar usuario\n");
        printf("2 - Editar usuario\n");
        printf("3 - Consultar usuarios\n");
        printf("4 - Inativar usuario\n");
        printf("0 - Voltar\n");
        printf("Escolha: ");
        scanf("%d", &op);

        if (op == 1) {
            if (totalUsuarios >= MAX_USUARIOS) {
                printf("Limite de usuarios atingido!\n");
            } else {
                printf("Nome do novo usuario: ");
                scanf("%19s", nomeBusca);

                if (buscarUsuario(nomeBusca) != -1) {
                    printf("Esse usuario ja existe!\n");
                } else {
                    printf("Senha: ");
                    scanf("%19s", usuarios[totalUsuarios].senha);
                    printf("Nivel (1-Administrador, 2-Atendente, 3-Entregador): ");
                    scanf("%d", &nivel);

                    if (nivel >= 1 && nivel <= 3) {
                        strcpy(usuarios[totalUsuarios].nome, nomeBusca);
                        usuarios[totalUsuarios].nivel = nivel;
                        usuarios[totalUsuarios].ativo = 1;
                        totalUsuarios++;
                        printf("Usuario cadastrado com sucesso!\n");
                    } else {
                        printf("Nivel invalido!\n");
                    }
                }
            }
        }
        else if (op == 2) {
            printf("Nome do usuario a editar: ");
            scanf("%19s", nomeBusca);
            pos = buscarUsuario(nomeBusca);

            if (pos == -1) {
                printf("Usuario nao encontrado!\n");
            } else {
                printf("Nova senha: ");
                scanf("%19s", usuarios[pos].senha);
                printf("Novo nivel (1-Administrador, 2-Atendente, 3-Entregador): ");
                scanf("%d", &nivel);

                if (nivel >= 1 && nivel <= 3) {
                    usuarios[pos].nivel = nivel;
                    printf("Usuario editado com sucesso!\n");
                } else {
                    printf("Nivel invalido! Apenas a senha foi alterada.\n");
                }
            }
        }
        else if (op == 3) {
            printf("\n=== USUARIOS ===\n");
            for (i = 0; i < totalUsuarios; i++) {
                printf("%s | Nivel: ", usuarios[i].nome);
                mostrarNivel(usuarios[i].nivel);
                if (usuarios[i].ativo == 1) {
                    printf(" | Ativo\n");
                } else {
                    printf(" | Inativo\n");
                }
            }
        }
        else if (op == 4) {
            printf("Nome do usuario a inativar: ");
            scanf("%19s", nomeBusca);
            pos = buscarUsuario(nomeBusca);

            if (pos == -1) {
                printf("Usuario nao encontrado!\n");
            } else if (pos == usuarioAtual) {
                printf("Voce nao pode inativar o seu proprio usuario!\n");
            } else {
                usuarios[pos].ativo = 0;
                printf("Usuario inativado com sucesso!\n");
            }
        }
        else if (op != 0) {
            printf("Opcao invalida!\n");
        }
    }
}

/* ---------- PEDIDOS ---------- */

void mostrarStatus(int status) {
    if (status == 1) {
        printf("Pendente");
    } else if (status == 2) {
        printf("Em entrega");
    } else {
        printf("Concluido");
    }
}

/* Procura um pedido (que nao foi excluido) pelo numero. Devolve a posicao ou -1 */
int buscarPedido(int id) {
    int i;
    for (i = 0; i < totalPedidos; i++) {
        if (pedidos[i].id == id && pedidos[i].ativo == 1) {
            return i;
        }
    }
    return -1;
}

void verPedidos() {
    int i, contador = 0;
    printf("\n=== PEDIDOS ===\n");
    for (i = 0; i < totalPedidos; i++) {
        if (pedidos[i].ativo == 1) {
            printf("Pedido %d | Cliente: %s | Produto: %s | Qtd: %d | Status: ",
                   pedidos[i].id, pedidos[i].cliente,
                   nomeProduto[pedidos[i].produto - 1], pedidos[i].quantidade);
            mostrarStatus(pedidos[i].status);
            printf("\n");
            contador++;
        }
    }
    if (contador == 0) {
        printf("Nenhum pedido registrado.\n");
    }
}

void criarPedido() {
    int produto, qtd;
    char cliente[30];

    if (totalPedidos >= MAX_PEDIDOS) {
        printf("Limite de pedidos atingido!\n");
        return;
    }

    printf("Nome do cliente: ");
    scanf(" %29[^\n]", cliente);        /* aceita nome com espacos */
    printf("Codigo do produto (1-P13, 2-P20, 3-P45): ");
    scanf("%d", &produto);

    if (produto < 1 || produto > 3) {
        printf("Produto nao encontrado!\n");
        return;
    }

    printf("Quantidade: ");
    scanf("%d", &qtd);

    if (qtd <= 0) {
        printf("Quantidade invalida!\n");
        return;
    }
    if (qtd > quantidade[produto - 1]) {
        printf("Estoque insuficiente! Pedido nao registrado.\n");
        return;
    }

    /* O pedido tira os botijoes do estoque */
    quantidade[produto - 1] -= qtd;

    pedidos[totalPedidos].id = proximoId;
    strcpy(pedidos[totalPedidos].cliente, cliente);
    pedidos[totalPedidos].produto = produto;
    pedidos[totalPedidos].quantidade = qtd;
    pedidos[totalPedidos].status = 1;
    pedidos[totalPedidos].ativo = 1;

    printf("Pedido %d registrado com sucesso!\n", proximoId);
    totalPedidos++;
    proximoId++;
}

void editarPedido() {
    int id, pos, op, novoProduto, novaQtd, novoStatus;
    char novoCliente[30];

    printf("Numero do pedido: ");
    scanf("%d", &id);
    pos = buscarPedido(id);

    if (pos == -1) {
        printf("Pedido nao encontrado!\n");
        return;
    }

    printf("O que deseja editar?\n");
    printf("1 - Produto e quantidade\n");
    printf("2 - Status\n");
    printf("3 - Nome do cliente\n");
    printf("0 - Cancelar\n");
    printf("Escolha: ");
    scanf("%d", &op);

    if (op == 1) {
        if (pedidos[pos].status == 3) {
            printf("Pedido concluido nao pode ter produto ou quantidade alterados!\n");
            return;
        }

        printf("Novo codigo do produto (1-P13, 2-P20, 3-P45): ");
        scanf("%d", &novoProduto);
        printf("Nova quantidade: ");
        scanf("%d", &novaQtd);

        if (novoProduto < 1 || novoProduto > 3 || novaQtd <= 0) {
            printf("Dados invalidos! Pedido nao alterado.\n");
            return;
        }

        /* Devolve ao estoque o que o pedido antigo tinha tirado, para conferir o novo */
        quantidade[pedidos[pos].produto - 1] += pedidos[pos].quantidade;

        if (novaQtd <= quantidade[novoProduto - 1]) {
            quantidade[novoProduto - 1] -= novaQtd;
            pedidos[pos].produto = novoProduto;
            pedidos[pos].quantidade = novaQtd;
            printf("Pedido editado com sucesso!\n");
        } else {
            /* Nao deu: desfaz a devolucao e mantem o pedido como estava */
            quantidade[pedidos[pos].produto - 1] -= pedidos[pos].quantidade;
            printf("Estoque insuficiente! Pedido nao alterado.\n");
        }
    }
    else if (op == 2) {
        printf("Novo status (1-Pendente, 2-Em entrega, 3-Concluido): ");
        scanf("%d", &novoStatus);

        if (novoStatus >= 1 && novoStatus <= 3) {
            pedidos[pos].status = novoStatus;
            printf("Status atualizado com sucesso!\n");
        } else {
            printf("Status invalido!\n");
        }
    }
    else if (op == 3) {
        printf("Novo nome do cliente: ");
        scanf(" %29[^\n]", novoCliente);
        strcpy(pedidos[pos].cliente, novoCliente);
        printf("Cliente atualizado com sucesso!\n");
    }
    else if (op != 0) {
        printf("Opcao invalida!\n");
    }
}

void excluirPedido() {
    int id, pos, confirma;

    printf("Numero do pedido a excluir: ");
    scanf("%d", &id);
    pos = buscarPedido(id);

    if (pos == -1) {
        printf("Pedido nao encontrado!\n");
        return;
    }

    printf("Tem certeza que deseja excluir o pedido %d? (1-Sim, 2-Nao): ", id);
    scanf("%d", &confirma);

    if (confirma == 1) {
        /* Se o pedido ainda nao foi concluido, os botijoes voltam para o estoque */
        if (pedidos[pos].status != 3) {
            quantidade[pedidos[pos].produto - 1] += pedidos[pos].quantidade;
        }
        pedidos[pos].ativo = 0;
        printf("Pedido excluido com sucesso!\n");
    } else {
        printf("Exclusao cancelada.\n");
    }
}

/* Submenu de pedidos. Administrador e Atendente criam e editam; so o Administrador exclui */
void gerenciarPedidos(int nivel) {
    int op = -1;

    while (op != 0) {
        printf("\n=== PEDIDOS ===\n");
        printf("1 - Ver pedidos\n");
        printf("2 - Criar pedido\n");
        printf("3 - Editar pedido\n");
        printf("4 - Excluir pedido\n");
        printf("0 - Voltar\n");
        printf("Escolha: ");
        scanf("%d", &op);

        if (op == 1) {
            verPedidos();                           /* todos os niveis */
        }
        else if (op == 2) {
            if (nivel == 1 || nivel == 2) {
                criarPedido();
            } else {
                printf("Acesso negado!\n");
            }
        }
        else if (op == 3) {
            if (nivel == 1 || nivel == 2) {
                editarPedido();
            } else {
                printf("Acesso negado!\n");
            }
        }
        else if (op == 4) {
            if (nivel == 1) {                       /* so Administrador */
                excluirPedido();
            } else {
                printf("Acesso negado!\n");
            }
        }
        else if (op != 0) {
            printf("Opcao invalida!\n");
        }
    }
}

/* ---------- PROGRAMA PRINCIPAL ---------- */

int main() {
    char usuario[20];
    char senha[20];
    int logado;
    int escolha;
    int pos;
    int opcao;
    int nivel;
    int sairPrograma = 0;

    printf("=======BEM VINDO A CONSIGAZ=======\n");

    /* Repete login + menu ate o usuario escolher sair do programa */
    while (sairPrograma == 0) {

        /* ----- LOGIN ----- */
        printf("\n=== SISTEMA DE ESTOQUE ===\n");
        logado = 0;

        while (logado == 0) {
            printf("Usuario: ");
            scanf("%19s", usuario);
            printf("Senha: ");
            scanf("%19s", senha);

            pos = buscarUsuario(usuario);

            if (pos != -1 && strcmp(senha, usuarios[pos].senha) == 0 && usuarios[pos].ativo == 1) {
                logado = 1;
                usuarioAtual = pos;
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
        nivel = usuarios[usuarioAtual].nivel;
        opcao = 0;

        /* ----- MENU (todos veem as opcoes, so quem tem permissao executa) ----- */
        while (opcao != 6 && opcao != 7) {
            printf("\n=== MENU (");
            mostrarNivel(nivel);
            printf(": %s) ===\n", usuarios[usuarioAtual].nome);
            printf("1 - Ver estoque\n");
            printf("2 - Adicionar quantidade\n");
            printf("3 - Remover quantidade\n");
            printf("4 - Pedidos\n");
            printf("5 - Gerenciar usuarios\n");
            printf("6 - Trocar de usuario\n");
            printf("7 - Sair do sistema\n");
            printf("Escolha uma opcao: ");
            scanf("%d", &opcao);

            if (opcao == 1) {
                verEstoque();                       /* todos os niveis */
            }
            else if (opcao == 2) {
                if (nivel == 1) {                   /* so Administrador */
                    adicionarEstoque();
                } else {
                    printf("Acesso negado!\n");
                }
            }
            else if (opcao == 3) {
                if (nivel == 1 || nivel == 2) {     /* Administrador e Atendente */
                    removerEstoque();
                } else {
                    printf("Acesso negado!\n");
                }
            }
            else if (opcao == 4) {
                gerenciarPedidos(nivel);            /* as permissoes ficam dentro do submenu */
            }
            else if (opcao == 5) {
                if (nivel == 1) {                   /* so Administrador */
                    gerenciarUsuarios();
                } else {
                    printf("Acesso negado!\n");
                }
            }
            else if (opcao == 6) {
                printf("Saindo da conta...\n");
                usuarioAtual = -1;
            }
            else if (opcao == 7) {
                printf("Sistema encerrado.\n");
                sairPrograma = 1;
            }
            else {
                printf("Opcao invalida!\n");
            }
        }
    }

    return 0;
}
