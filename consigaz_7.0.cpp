/* Sistema de estoque, pedidos e entregas da Consigaz - PIM II (Etapa 5, em C) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USUARIOS 100
#define MAX_PRODUTOS 20
#define MAX_PEDIDOS 100
#define MAX_ITENS 5
#define MAX_HISTORICO 500
#define VERSAO 3
#define ARQUIVO "consigaz_dados.dat"
#define ARQUIVO_BACKUP "consigaz_backup.dat"
#define PRINCIPAL 1   /* salvarDados(PRINCIPAL) grava o arquivo de dados */
#define BACKUP 2      /* salvarDados(BACKUP) grava a copia de seguranca */

/* Niveis: 1 = Administrador, 2 = Atendente, 3 = Entregador, 4 = Cliente */
struct Usuario {
    char login[20];         /* nome de usuario usado no login (uma palavra so) */
    char senha[20];
    int nivel;
    int ativo;              /* 1 = ativo, 0 = inativo */
    int id;                 /* chave primaria: 0, 1, 2... na ordem de criacao */
    char nomeCompleto[40];
    char telefone[16];      /* clientes: obrigatorio */
    char endereco[60];      /* clientes: obrigatorio */
};

struct Produto {
    char nome[10];
    int quantidade;         /* quantidade em estoque */
    int preco;
    int ativo;              /* 1 = ativo, 0 = inativo */
};

/* Pagamento: 1 = Debito, 2 = Credito, 3 = Pix
   Status:    1 = Pendente, 2 = Em rota, 3 = Entregue */
struct Pedido {
    int id;
    int cliente;                    /* ID do cliente */
    int totalItens;
    int produto[MAX_ITENS];         /* codigo de cada produto */
    int qtd[MAX_ITENS];
    int valorUnitario[MAX_ITENS];   /* preco no momento do pedido */
    int pagamento;
    int status;
    int entregador;                 /* ID do entregador (-1 = ainda nao atribuido) */
    int ativo;                      /* 1 = existe, 0 = excluido */
};

struct Historico {
    int tipo;
    int usuario;     /* ID de quem fez */
    int a;
    int b;
};

struct Usuario usuarios[MAX_USUARIOS] = {
    {"lucas", "0407", 1, 1, 0, "Lucas Admin", "", ""}
};
int totalUsuarios = 1;
int usuarioAtual = -1;      /* posicao do usuario logado (e tambem o ID dele) */

struct Produto produtos[MAX_PRODUTOS] = {
    {"P13", 90, 110, 1},
    {"P20", 35, 250, 1},
    {"P45", 10, 450, 1}
};
int totalProdutos = 3;

struct Pedido pedidos[MAX_PEDIDOS];
int totalPedidos = 0;
int proximoPedido = 1;

struct Historico historico[MAX_HISTORICO];
int totalHistorico = 0;

/* Itens do pedido que esta sendo montado (guardados aqui enquanto a pessoa escolhe) */
int itensProduto[MAX_ITENS];
int itensQtd[MAX_ITENS];
int itensValor[MAX_ITENS];
int itensTotal = 0;

/* ---------- LEITURA SEGURA DO TECLADO ---------- */

/* Le uma linha do teclado (aceita espacos). Ignora linha vazia. */
void lerLinha(char *destino, int tamanho) {
    char linha[200];
    int i, j;

    destino[0] = '\0';
    while (destino[0] == '\0') {
        if (fgets(linha, 200, stdin) == NULL) {      /* entrada encerrada (Ctrl+Z) */
            printf("\nEntrada encerrada.\n");
            exit(0);
        }

        i = 0;
        while (linha[i] == ' ') {                    /* pula espacos do comeco */
            i++;
        }
        j = 0;
        while (linha[i] != '\0' && linha[i] != '\n' && j < tamanho - 1) {
            destino[j] = linha[i];
            i++;
            j++;
        }
        while (j > 0 && destino[j - 1] == ' ') {     /* tira espacos do fim */
            j--;
        }
        destino[j] = '\0';
    }
}

/* Le um numero inteiro (so digitos). Se nao for numero, devolve -1.
   Assim, digitar uma letra nao trava o programa. */
int lerInteiro() {
    char texto[30];
    int i, numero;

    lerLinha(texto, 30);

    if (strlen(texto) > 9) {
        return -1;
    }
    numero = 0;
    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] < '0' || texto[i] > '9') {
            return -1;
        }
        numero = numero * 10 + (texto[i] - '0');
    }
    return numero;
}

/* ---------- FUNCOES AUXILIARES ---------- */

/* Devolve 1 se os dois textos forem iguais, senao 0 */
int iguais(char *a, char *b) {
    int i = 0;
    while (a[i] != '\0' && a[i] == b[i]) {
        i++;
    }
    if (a[i] == b[i]) {
        return 1;
    }
    return 0;
}

/* Devolve 1 se o texto for exatamente "0" */
int ehZero(char *texto) {
    if (texto[0] == '0' && texto[1] == '\0') {
        return 1;
    }
    return 0;
}

/* Devolve 1 se o texto comeca com o prefixo (sem diferenciar maiuscula de minuscula) */
int comecaCom(char *texto, char *prefixo) {
    int i;
    for (i = 0; prefixo[i] != '\0'; i++) {
        if (texto[i] == '\0' || toupper(texto[i]) != toupper(prefixo[i])) {
            return 0;
        }
    }
    return 1;
}

/* Devolve 1 se o texto tem so numeros */
int soNumeros(char *texto) {
    int i;
    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] < '0' || texto[i] > '9') {
            return 0;
        }
    }
    return 1;
}

/* Devolve 1 se o texto tem algum espaco */
int temEspaco(char *texto) {
    int i;
    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] == ' ') {
            return 1;
        }
    }
    return 0;
}

/* Telefone: 10 ou 11 numeros (com DDD) */
int telefoneValido(char *tel) {
    if ((strlen(tel) == 10 || strlen(tel) == 11) && soNumeros(tel) == 1) {
        return 1;
    }
    return 0;
}

void mostrarNivel(int nivel) {
    if (nivel == 1) {
        printf("Administrador");
    } else if (nivel == 2) {
        printf("Atendente");
    } else if (nivel == 3) {
        printf("Entregador");
    } else {
        printf("Cliente");
    }
}

void mostrarStatus(int status) {
    if (status == 1) {
        printf("Pendente");
    } else if (status == 2) {
        printf("Em rota");
    } else {
        printf("Entregue");
    }
}

void mostrarPagamento(int pagamento) {
    if (pagamento == 1) {
        printf("Debito");
    } else if (pagamento == 2) {
        printf("Credito");
    } else {
        printf("Pix");
    }
}

void mostrarEntregador(int e) {
    if (e == -1) {
        printf("nao atribuido");
    } else {
        printf("%s (ID %02d)", usuarios[e].nomeCompleto, e);
    }
}

/* Procura um usuario pelo login. Devolve o ID ou -1 */
int buscarUsuario(char *login) {
    int i;
    for (i = 0; i < totalUsuarios; i++) {
        if (iguais(usuarios[i].login, login) == 1) {
            return i;
        }
    }
    return -1;
}

/* Devolve o ID se for um CLIENTE, ou -1 */
int buscarClientePorId(int id) {
    if (id >= 0 && id < totalUsuarios && usuarios[id].nivel == 4) {
        return id;
    }
    return -1;
}

/* Devolve o ID se for um ENTREGADOR ativo, ou -1 */
int buscarEntregadorPorId(int id) {
    if (id >= 0 && id < totalUsuarios && usuarios[id].nivel == 3 && usuarios[id].ativo == 1) {
        return id;
    }
    return -1;
}

/* Devolve 1 se o codigo e de um produto ativo */
int produtoValido(int codigo) {
    if (codigo >= 1 && codigo <= totalProdutos && produtos[codigo - 1].ativo == 1) {
        return 1;
    }
    return 0;
}

/* Guarda uma operacao no historico (feita pelo usuario logado) */
void registrarHistorico(int tipo, int a, int b) {
    if (totalHistorico < MAX_HISTORICO) {
        historico[totalHistorico].tipo = tipo;
        historico[totalHistorico].usuario = usuarioAtual;
        historico[totalHistorico].a = a;
        historico[totalHistorico].b = b;
        totalHistorico++;
    }
}

/* ---------- ARQUIVO: SALVAR, CARREGAR E BACKUP ---------- */

void salvarDados(int destino) {
    FILE *arq;
    int i;
    int versao = VERSAO;

    if (destino == BACKUP) {
        arq = fopen(ARQUIVO_BACKUP, "wb");
    } else {
        arq = fopen(ARQUIVO, "wb");
    }
    if (arq == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    fwrite(&versao, sizeof(int), 1, arq);

    fwrite(&totalUsuarios, sizeof(int), 1, arq);
    for (i = 0; i < totalUsuarios; i++) {
        fwrite(&usuarios[i], sizeof(struct Usuario), 1, arq);
    }

    fwrite(&totalProdutos, sizeof(int), 1, arq);
    for (i = 0; i < totalProdutos; i++) {
        fwrite(&produtos[i], sizeof(struct Produto), 1, arq);
    }

    fwrite(&totalPedidos, sizeof(int), 1, arq);
    fwrite(&proximoPedido, sizeof(int), 1, arq);
    for (i = 0; i < totalPedidos; i++) {
        fwrite(&pedidos[i], sizeof(struct Pedido), 1, arq);
    }

    fwrite(&totalHistorico, sizeof(int), 1, arq);
    for (i = 0; i < totalHistorico; i++) {
        fwrite(&historico[i], sizeof(struct Historico), 1, arq);
    }

    fclose(arq);
}

void carregarDados() {
    FILE *arq;
    int i;
    int versao = 0;

    arq = fopen(ARQUIVO, "rb");
    if (arq == NULL) {
        return;     /* ainda nao existe arquivo: comeca com os dados iniciais */
    }

    fread(&versao, sizeof(int), 1, arq);
    if (versao != VERSAO) {
        fclose(arq);
        return;
    }

    fread(&totalUsuarios, sizeof(int), 1, arq);
    for (i = 0; i < totalUsuarios; i++) {
        fread(&usuarios[i], sizeof(struct Usuario), 1, arq);
    }

    fread(&totalProdutos, sizeof(int), 1, arq);
    for (i = 0; i < totalProdutos; i++) {
        fread(&produtos[i], sizeof(struct Produto), 1, arq);
    }

    fread(&totalPedidos, sizeof(int), 1, arq);
    fread(&proximoPedido, sizeof(int), 1, arq);
    for (i = 0; i < totalPedidos; i++) {
        fread(&pedidos[i], sizeof(struct Pedido), 1, arq);
    }

    fread(&totalHistorico, sizeof(int), 1, arq);
    for (i = 0; i < totalHistorico; i++) {
        fread(&historico[i], sizeof(struct Historico), 1, arq);
    }

    fclose(arq);
}

/* ---------- CLIENTES: DADOS E BUSCA ---------- */

/* Pede um dado do cliente e confere se esta certo.
   tipo: 1 = telefone, 2 = endereco.
   Devolve 1 se deu certo ou 0 se a pessoa cancelou. */
int pedirDado(int tipo, char *destino) {
    int ok = 0;
    int escolha;

    while (ok == 0) {
        if (tipo == 1) {
            printf("Telefone com DDD (so numeros): ");
        } else {
            printf("Endereco (rua, numero e bairro): ");
        }
        lerLinha(destino, 60);

        if (tipo == 1 && telefoneValido(destino) == 0) {
            printf("Telefone invalido! Digite 10 ou 11 numeros.\n");
        } else if (tipo == 2 && strlen(destino) < 5) {
            printf("Endereco muito curto!\n");
        } else {
            ok = 1;
        }

        if (ok == 0) {
            printf("1 - Tentar novamente\n");
            printf("2 - Cancelar\n");
            printf("Escolha: ");
            escolha = lerInteiro();
            if (escolha == 2) {
                return 0;
            }
        }
    }
    return 1;
}

/* Pede telefone e endereco (os dois obrigatorios). So grava no fim.
   Devolve 1 se deu certo ou 0 se cancelou. */
int lerDadosCliente(int pos) {
    char tel[60], end[60];

    if (pedirDado(1, tel) == 0 || pedirDado(2, end) == 0) {
        return 0;
    }
    strcpy(usuarios[pos].telefone, tel);
    strcpy(usuarios[pos].endereco, end);
    return 1;
}

/* Confere um login novo. Devolve 1 se pode usar, ou 0 (e explica o motivo) */
int loginValido(char *login) {
    if (ehZero(login) == 1) {           /* o "0" serve para voltar na tela de login */
        printf("Nome de usuario invalido!\n");
        return 0;
    }
    if (temEspaco(login) == 1) {
        printf("O nome de usuario nao pode ter espaco! Use so uma palavra (ex: lucasg).\n");
        printf("O nome com espaco vai em \"Nome completo\".\n");
        return 0;
    }
    if (strlen(login) > 19) {
        printf("Nome de usuario muito grande (ate 19 letras)!\n");
        return 0;
    }
    if (buscarUsuario(login) != -1) {
        printf("Esse nome de usuario ja existe!\n");
        return 0;
    }
    return 1;
}

/* O que aparece depende de quem esta vendo:
   o Administrador ve tudo; os outros veem so o necessario para atender ou entregar. */
void mostrarCliente(int i) {
    printf("ID %02d | %s", usuarios[i].id, usuarios[i].nomeCompleto);
    if (usuarios[usuarioAtual].nivel == 1) {
        printf(" | Usuario: %s", usuarios[i].login);
        if (usuarios[i].ativo == 1) {
            printf(" | Ativo");
        } else {
            printf(" | Inativo");
        }
    }
    printf(" | Tel: %s | Endereco: %s\n", usuarios[i].telefone, usuarios[i].endereco);
}

/* Lista os clientes cujo nome COMECA com o texto digitado */
void buscarPorNome() {
    char busca[40];
    int i, achou = 0;

    printf("Nome (ou o comeco do nome): ");
    lerLinha(busca, 40);

    for (i = 0; i < totalUsuarios; i++) {
        if (usuarios[i].nivel == 4 && comecaCom(usuarios[i].nomeCompleto, busca) == 1) {
            mostrarCliente(i);
            achou++;
        }
    }
    if (achou == 0) {
        printf("Nenhum cliente encontrado.\n");
    }
}

/* Pede o ID do cliente. Digitando 0, busca pelo nome e pede o ID de novo.
   Devolve o ID do cliente ou -1. */
int pedirIdCliente() {
    int id, cliente;

    printf("ID do cliente (digite 0 para buscar pelo nome): ");
    id = lerInteiro();

    if (id == 0) {
        buscarPorNome();
        printf("ID do cliente: ");
        id = lerInteiro();
    }

    cliente = buscarClientePorId(id);
    if (cliente == -1) {
        printf("Cliente nao encontrado! Digite o numero do ID, ou 0 para buscar pelo nome.\n");
    }
    return cliente;
}

/* ---------- ESTOQUE E PRODUTOS ---------- */

void verEstoque() {
    int i;
    printf("\n=== ESTOQUE ===\n");
    for (i = 0; i < totalProdutos; i++) {
        printf("Codigo: %d | Produto: %s | Qtd: %d | Preco: R$%d",
               i + 1, produtos[i].nome, produtos[i].quantidade, produtos[i].preco);
        if (produtos[i].ativo == 0) {
            printf(" | Inativo");
        }
        printf("\n");
    }
}

/* Lista de produtos ativos, para o cliente e para os pedidos (sem a quantidade exata) */
void verProdutos() {
    int i;
    printf("\n=== PRODUTOS ===\n");
    for (i = 0; i < totalProdutos; i++) {
        if (produtos[i].ativo == 1) {
            printf("Codigo: %d | Produto: %s | Preco: R$%d | ", i + 1, produtos[i].nome, produtos[i].preco);
            if (produtos[i].quantidade > 0) {
                printf("Disponivel\n");
            } else {
                printf("Indisponivel\n");
            }
        }
    }
}

void adicionarEstoque() {
    int codigo, qtd;

    verEstoque();
    printf("Codigo do produto: ");
    codigo = lerInteiro();

    if (codigo >= 1 && codigo <= totalProdutos) {
        printf("Quantidade para adicionar: ");
        qtd = lerInteiro();
        if (qtd > 0) {
            produtos[codigo - 1].quantidade += qtd;
            registrarHistorico(1, codigo, qtd);
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

    verEstoque();
    printf("Codigo do produto: ");
    codigo = lerInteiro();

    if (codigo >= 1 && codigo <= totalProdutos) {
        printf("Quantidade para remover: ");
        qtd = lerInteiro();

        if (qtd <= 0) {
            printf("Quantidade invalida!\n");
        } else if (qtd <= produtos[codigo - 1].quantidade) {
            produtos[codigo - 1].quantidade -= qtd;
            registrarHistorico(2, codigo, qtd);
            printf("Quantidade removida com sucesso!\n");
        } else {
            printf("Estoque insuficiente!\n");
        }
    } else {
        printf("Produto nao encontrado!\n");
    }
}

/* Devolve o codigo do produto com esse nome, ou 0 se nao existir */
int buscarProduto(char *nome) {
    int i;
    for (i = 0; i < totalProdutos; i++) {
        if (iguais(produtos[i].nome, nome) == 1) {
            return i + 1;
        }
    }
    return 0;
}

/* So Administrador */
void gerenciarProdutos() {
    int op = -1;
    int codigo, novoPreco;
    char nomeNovo[40];

    while (op != 0) {
        printf("\n=== PRODUTOS E PRECOS ===\n");
        printf("1 - Cadastrar produto\n");
        printf("2 - Atualizar preco\n");
        printf("3 - Alterar nome\n");
        printf("4 - Inativar produto\n");
        printf("5 - Reativar produto\n");
        printf("0 - Voltar\n");
        printf("Escolha: ");
        op = lerInteiro();

        if (op == 1) {
            if (totalProdutos >= MAX_PRODUTOS) {
                printf("Limite de produtos atingido!\n");
            } else {
                printf("Nome do produto (ate 9 letras, ex: P90): ");
                lerLinha(nomeNovo, 40);

                if (strlen(nomeNovo) > 9) {
                    printf("Nome muito grande (ate 9 letras)!\n");
                } else if (buscarProduto(nomeNovo) != 0) {
                    printf("Esse produto ja existe!\n");
                } else {
                    printf("Preco (R$): ");
                    novoPreco = lerInteiro();

                    if (novoPreco <= 0) {
                        printf("Preco invalido!\n");
                    } else {
                        strcpy(produtos[totalProdutos].nome, nomeNovo);
                        produtos[totalProdutos].quantidade = 0;
                        produtos[totalProdutos].preco = novoPreco;
                        produtos[totalProdutos].ativo = 1;
                        totalProdutos++;
                        registrarHistorico(8, totalProdutos, novoPreco);
                        printf("Produto cadastrado! Codigo: %d (estoque 0: use Adicionar quantidade)\n", totalProdutos);
                    }
                }
            }
        }
        else if (op >= 2 && op <= 5) {
            verEstoque();
            printf("Codigo do produto: ");
            codigo = lerInteiro();

            if (codigo < 1 || codigo > totalProdutos) {
                printf("Produto nao encontrado!\n");
            }
            else if (op == 2) {
                printf("Novo preco de %s (R$): ", produtos[codigo - 1].nome);
                novoPreco = lerInteiro();

                if (novoPreco <= 0) {
                    printf("Preco invalido!\n");
                } else {
                    produtos[codigo - 1].preco = novoPreco;
                    registrarHistorico(9, codigo, novoPreco);
                    printf("Preco atualizado! (pedidos antigos mantem o preco da epoca)\n");
                }
            }
            else if (op == 3) {
                printf("Novo nome (ate 9 letras): ");
                lerLinha(nomeNovo, 40);

                if (strlen(nomeNovo) > 9) {
                    printf("Nome muito grande (ate 9 letras)!\n");
                } else if (buscarProduto(nomeNovo) != 0) {
                    printf("Esse nome ja esta em uso!\n");
                } else {
                    strcpy(produtos[codigo - 1].nome, nomeNovo);
                    registrarHistorico(11, codigo, 0);
                    printf("Nome alterado com sucesso!\n");
                }
            }
            else if (op == 4) {
                produtos[codigo - 1].ativo = 0;
                registrarHistorico(10, codigo, 0);
                printf("Produto inativado! Nao aparece mais para novos pedidos.\n");
            }
            else {
                produtos[codigo - 1].ativo = 1;
                registrarHistorico(10, codigo, 1);
                printf("Produto reativado!\n");
            }
        }
        else if (op != 0) {
            printf("Opcao invalida!\n");
        }

        salvarDados(PRINCIPAL);
    }
}

/* ---------- CLIENTES ---------- */

/* Cria uma conta de cliente (nivel 4). Usada na tela inicial e pelos funcionarios. */
void cadastrarCliente() {
    char login[40];
    char nomeCompleto[40];

    printf("\n=== CADASTRAR CLIENTE ===\n");

    if (totalUsuarios >= MAX_USUARIOS) {
        printf("Limite de usuarios atingido!\n");
        return;
    }

    printf("Nome completo: ");
    lerLinha(nomeCompleto, 40);

    printf("Nome de usuario para o login (uma palavra so, sem espacos. Ex: lucasg): ");
    lerLinha(login, 40);
    if (loginValido(login) == 0) {
        return;
    }

    printf("Senha: ");
    lerLinha(usuarios[totalUsuarios].senha, 20);

    if (lerDadosCliente(totalUsuarios) == 0) {
        printf("Cadastro cancelado.\n");
        return;
    }

    strcpy(usuarios[totalUsuarios].login, login);
    strcpy(usuarios[totalUsuarios].nomeCompleto, nomeCompleto);
    usuarios[totalUsuarios].nivel = 4;
    usuarios[totalUsuarios].ativo = 1;
    usuarios[totalUsuarios].id = totalUsuarios;

    printf("Cliente cadastrado com sucesso! ID do cliente: %02d\n", totalUsuarios);
    totalUsuarios++;
}

/* Administrador e Atendente atualizam os dados de um cliente */
void atualizarDadosCliente() {
    int cliente, op;
    char nomeCompleto[40];

    cliente = pedirIdCliente();
    if (cliente == -1) {
        return;
    }

    printf("Cliente: %s\n", usuarios[cliente].nomeCompleto);
    printf("O que deseja atualizar?\n");
    printf("1 - Nome completo\n");
    printf("2 - Telefone e endereco\n");
    printf("0 - Cancelar\n");
    printf("Escolha: ");
    op = lerInteiro();

    if (op == 1) {
        printf("Novo nome completo: ");
        lerLinha(nomeCompleto, 40);
        strcpy(usuarios[cliente].nomeCompleto, nomeCompleto);
        printf("Nome atualizado com sucesso!\n");
    }
    else if (op == 2) {
        if (lerDadosCliente(cliente) == 1) {
            printf("Dados atualizados com sucesso!\n");
        } else {
            printf("Atualizacao cancelada.\n");
        }
    }
    else if (op != 0) {
        printf("Opcao invalida!\n");
    }
}

/* Acoes do menu de clientes que cada nivel pode ver (0 = vazio)
   Acoes: 1 = Buscar, 2 = Cadastrar, 3 = Atualizar dados
   Linhas: 1 = Administrador, 2 = Atendente, 3 = Entregador, 4 = Cliente */
int acoesClientes[5][3] = {
    {0, 0, 0},
    {1, 2, 3},
    {1, 2, 3},
    {1, 0, 0},
    {0, 0, 0}
};

void mostrarAcaoCliente(int acao) {
    if (acao == 1) {
        printf("Buscar cliente");
    } else if (acao == 2) {
        printf("Cadastrar cliente");
    } else {
        printf("Atualizar dados do cliente");
    }
}

void gerenciarClientes(int nivel) {
    int op = -1;
    int acao, i, id, cliente;

    while (op != 0) {
        printf("\n=== CLIENTES ===\n");
        for (i = 0; i < 3 && acoesClientes[nivel][i] != 0; i++) {
            printf("%d - ", i + 1);
            mostrarAcaoCliente(acoesClientes[nivel][i]);
            printf("\n");
        }
        printf("0 - Voltar\n");
        printf("Escolha: ");
        op = lerInteiro();

        acao = 0;
        if (op >= 1 && op <= 3 && acoesClientes[nivel][op - 1] != 0) {
            acao = acoesClientes[nivel][op - 1];
        }

        if (acao == 1) {
            printf("1 - Buscar por ID\n");
            printf("2 - Buscar por nome\n");
            printf("Escolha: ");
            id = lerInteiro();

            if (id == 1) {
                printf("ID do cliente: ");
                id = lerInteiro();
                cliente = buscarClientePorId(id);
                if (cliente == -1) {
                    printf("Cliente nao encontrado!\n");
                } else {
                    mostrarCliente(cliente);
                }
            } else if (id == 2) {
                buscarPorNome();
            } else {
                printf("Opcao invalida!\n");
            }
        }
        else if (acao == 2) {
            cadastrarCliente();
        }
        else if (acao == 3) {
            atualizarDadosCliente();
        }
        else if (op != 0) {
            printf("Opcao invalida!\n");
        }

        salvarDados(PRINCIPAL);
    }
}

/* ---------- USUARIOS (so Administrador) ---------- */

void gerenciarUsuarios() {
    int op = -1;
    int i, id, nivel, sub;
    char login[40];
    char nomeCompleto[40];

    while (op != 0) {
        printf("\n=== GERENCIAR USUARIOS ===\n");
        printf("1 - Cadastrar usuario\n");
        printf("2 - Editar usuario\n");
        printf("3 - Consultar usuarios\n");
        printf("4 - Inativar usuario\n");
        printf("5 - Reativar usuario\n");
        printf("0 - Voltar\n");
        printf("Escolha: ");
        op = lerInteiro();

        if (op == 1) {
            if (totalUsuarios >= MAX_USUARIOS) {
                printf("Limite de usuarios atingido!\n");
            } else {
                printf("Nome completo: ");
                lerLinha(nomeCompleto, 40);
                printf("Nome de usuario para o login (uma palavra so, sem espacos. Ex: lucasg): ");
                lerLinha(login, 40);

                if (loginValido(login) == 1) {
                    printf("Senha: ");
                    lerLinha(usuarios[totalUsuarios].senha, 20);
                    printf("Nivel (1-Administrador, 2-Atendente, 3-Entregador, 4-Cliente): ");
                    nivel = lerInteiro();

                    if (nivel < 1 || nivel > 4) {
                        printf("Nivel invalido!\n");
                    } else if (nivel == 4 && lerDadosCliente(totalUsuarios) == 0) {
                        printf("Cadastro cancelado.\n");
                    } else {
                        strcpy(usuarios[totalUsuarios].login, login);
                        strcpy(usuarios[totalUsuarios].nomeCompleto, nomeCompleto);
                        usuarios[totalUsuarios].nivel = nivel;
                        usuarios[totalUsuarios].ativo = 1;
                        usuarios[totalUsuarios].id = totalUsuarios;
                        printf("Usuario cadastrado com sucesso! ID: %02d\n", totalUsuarios);
                        totalUsuarios++;
                    }
                }
            }
        }
        else if (op == 2) {
            printf("ID do usuario a editar: ");
            id = lerInteiro();

            if (id < 0 || id >= totalUsuarios) {
                printf("Usuario nao encontrado!\n");
            } else {
                printf("Editando: %s (%s)\n", usuarios[id].nomeCompleto, usuarios[id].login);
                printf("1 - Nome completo\n");
                printf("2 - Senha\n");
                printf("3 - Nivel\n");
                printf("4 - Telefone e endereco (so cliente)\n");
                printf("0 - Cancelar\n");
                printf("Escolha: ");
                sub = lerInteiro();

                if (sub == 1) {
                    printf("Novo nome completo: ");
                    lerLinha(nomeCompleto, 40);
                    strcpy(usuarios[id].nomeCompleto, nomeCompleto);
                    printf("Nome atualizado com sucesso!\n");
                }
                else if (sub == 2) {
                    printf("Nova senha: ");
                    lerLinha(usuarios[id].senha, 20);
                    printf("Senha atualizada com sucesso!\n");
                }
                else if (sub == 3) {
                    printf("Novo nivel (1-Administrador, 2-Atendente, 3-Entregador, 4-Cliente): ");
                    nivel = lerInteiro();

                    if (nivel < 1 || nivel > 4) {
                        printf("Nivel invalido!\n");
                    } else if (id == usuarioAtual && nivel != 1) {
                        printf("Voce nao pode mudar o seu proprio nivel!\n");
                    } else if (nivel == 4 && usuarios[id].telefone[0] == '\0' && lerDadosCliente(id) == 0) {
                        printf("Nivel nao alterado.\n");
                    } else {
                        usuarios[id].nivel = nivel;
                        printf("Nivel atualizado com sucesso!\n");
                    }
                }
                else if (sub == 4) {
                    if (usuarios[id].nivel != 4) {
                        printf("So clientes tem esses dados!\n");
                    } else if (lerDadosCliente(id) == 1) {
                        printf("Dados atualizados com sucesso!\n");
                    } else {
                        printf("Atualizacao cancelada.\n");
                    }
                }
                else if (sub != 0) {
                    printf("Opcao invalida!\n");
                }
            }
        }
        else if (op == 3) {
            printf("\n=== USUARIOS ===\n");
            for (i = 0; i < totalUsuarios; i++) {
                printf("ID %02d | %s (%s) | Nivel: ", usuarios[i].id, usuarios[i].nomeCompleto, usuarios[i].login);
                mostrarNivel(usuarios[i].nivel);
                if (usuarios[i].ativo == 1) {
                    printf(" | Ativo");
                } else {
                    printf(" | Inativo");
                }
                if (usuarios[i].telefone[0] != '\0') {
                    printf(" | Tel: %s | Endereco: %s", usuarios[i].telefone, usuarios[i].endereco);
                }
                printf("\n");
            }
        }
        else if (op == 4 || op == 5) {
            if (op == 4) {
                printf("ID do usuario a inativar: ");
            } else {
                printf("ID do usuario a reativar: ");
            }
            id = lerInteiro();

            if (id < 0 || id >= totalUsuarios) {
                printf("Usuario nao encontrado!\n");
            } else if (op == 4 && id == usuarioAtual) {
                printf("Voce nao pode inativar o seu proprio usuario!\n");
            } else if (op == 4) {
                usuarios[id].ativo = 0;
                printf("Usuario inativado com sucesso!\n");
            } else {
                usuarios[id].ativo = 1;
                printf("Usuario reativado com sucesso!\n");
            }
        }
        else if (op != 0) {
            printf("Opcao invalida!\n");
        }

        salvarDados(PRINCIPAL);
    }
}

/* ---------- PEDIDOS ---------- */

/* Devolve ao estoque os itens do pedido p */
void devolverItensDoPedido(int p) {
    int i;
    for (i = 0; i < pedidos[p].totalItens; i++) {
        produtos[pedidos[p].produto[i] - 1].quantidade += pedidos[p].qtd[i];
    }
}

/* Tira do estoque os itens do pedido p (desfaz o devolverItensDoPedido) */
void retirarItensDoPedido(int p) {
    int i;
    for (i = 0; i < pedidos[p].totalItens; i++) {
        produtos[pedidos[p].produto[i] - 1].quantidade -= pedidos[p].qtd[i];
    }
}

/* Devolve ao estoque os itens que ainda estao sendo montados */
void devolverItensTemporarios() {
    int i;
    for (i = 0; i < itensTotal; i++) {
        produtos[itensProduto[i] - 1].quantidade += itensQtd[i];
    }
    itensTotal = 0;
}

/* Passa os itens montados para dentro do pedido p */
void copiarItensParaPedido(int p) {
    int i;
    pedidos[p].totalItens = itensTotal;
    for (i = 0; i < itensTotal; i++) {
        pedidos[p].produto[i] = itensProduto[i];
        pedidos[p].qtd[i] = itensQtd[i];
        pedidos[p].valorUnitario[i] = itensValor[i];
    }
}

/* Soma o valor dos itens do pedido p */
int totalDoPedido(int p) {
    int i, total = 0;
    for (i = 0; i < pedidos[p].totalItens; i++) {
        total += pedidos[p].qtd[i] * pedidos[p].valorUnitario[i];
    }
    return total;
}

/* Mostra os itens do pedido p, por exemplo: 2x P13, 1x P20 */
void mostrarItens(int p) {
    int i;
    for (i = 0; i < pedidos[p].totalItens; i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%dx %s", pedidos[p].qtd[i], produtos[pedidos[p].produto[i] - 1].nome);
    }
}

/* O cliente escolhe os produtos e quantidades (ate MAX_ITENS).
   Cada item ja sai do estoque enquanto e escolhido. Devolve quantos itens ficaram (0 = cancelou). */
int lerItens() {
    int codigo, qtd, mais;

    itensTotal = 0;
    mais = 1;

    while (mais == 1) {
        verProdutos();
        printf("Codigo do produto: ");
        codigo = lerInteiro();

        if (produtoValido(codigo) == 0) {
            printf("Produto nao encontrado!\n");
        } else {
            printf("Quantidade: ");
            qtd = lerInteiro();

            if (qtd <= 0) {
                printf("Quantidade invalida!\n");
            } else if (qtd > produtos[codigo - 1].quantidade) {
                printf("Estoque insuficiente!\n");
            } else {
                produtos[codigo - 1].quantidade -= qtd;
                itensProduto[itensTotal] = codigo;
                itensQtd[itensTotal] = qtd;
                itensValor[itensTotal] = produtos[codigo - 1].preco;
                itensTotal++;
                printf("Item adicionado ao pedido.\n");
            }
        }

        if (itensTotal == 0) {
            printf("1 - Tentar novamente\n");
            printf("2 - Cancelar\n");
            printf("Escolha: ");
            mais = lerInteiro();
            if (mais == 2) {
                return 0;
            }
            mais = 1;
        } else if (itensTotal >= MAX_ITENS) {
            printf("Limite de %d produtos por pedido.\n", MAX_ITENS);
            mais = 2;
        } else {
            printf("Adicionar outro produto? (1-Sim, 2-Nao): ");
            mais = lerInteiro();
        }
    }
    return itensTotal;
}

/* Procura um pedido pelo numero. Devolve a posicao ou -1.
   Pedido excluido nao aparece. O cliente so enxerga os proprios pedidos. */
int buscarPedido(int id) {
    int i;
    int nivel = usuarios[usuarioAtual].nivel;

    for (i = 0; i < totalPedidos; i++) {
        if (pedidos[i].id == id && pedidos[i].ativo == 1) {
            if (nivel != 4 || pedidos[i].cliente == usuarioAtual) {
                return i;
            }
        }
    }
    return -1;
}

void verPedidos() {
    int i, contador = 0;
    int nivel = usuarios[usuarioAtual].nivel;

    if (nivel == 4) {
        printf("\n=== MEUS PEDIDOS ===\n");
    } else {
        printf("\n=== PEDIDOS ===\n");
    }

    for (i = 0; i < totalPedidos; i++) {
        if (pedidos[i].ativo == 1 && (nivel != 4 || pedidos[i].cliente == usuarioAtual)) {
            printf("Pedido %d | Cliente: %s (ID %02d) | Itens: ",
                   pedidos[i].id, usuarios[pedidos[i].cliente].nomeCompleto, pedidos[i].cliente);
            mostrarItens(i);
            printf(" | Total: R$%d | Pagamento: ", totalDoPedido(i));
            mostrarPagamento(pedidos[i].pagamento);
            printf(" | Status: ");
            mostrarStatus(pedidos[i].status);
            printf(" | Entregador: ");
            mostrarEntregador(pedidos[i].entregador);
            printf("\n");
            contador++;
        }
    }
    if (contador == 0) {
        printf("Nenhum pedido encontrado.\n");
    }
}

/* Pergunta a forma de pagamento. Devolve 1, 2 ou 3 (ou 0 se for invalida) */
int escolherPagamento() {
    int forma;
    printf("Forma de pagamento (1-Debito, 2-Credito, 3-Pix): ");
    forma = lerInteiro();

    if (forma < 1 || forma > 3) {
        printf("Forma de pagamento invalida!\n");
        return 0;
    }
    return forma;
}

void criarPedido() {
    int i, pagamento, confirma, total, cliente;
    int nivel = usuarios[usuarioAtual].nivel;

    if (totalPedidos >= MAX_PEDIDOS) {
        printf("Limite de pedidos atingido!\n");
        return;
    }

    /* Cliente faz o pedido em nome proprio; os funcionarios buscam o cliente */
    if (nivel == 4) {
        cliente = usuarioAtual;
    } else {
        cliente = pedirIdCliente();
        if (cliente == -1) {
            printf("Se ele ainda nao tem cadastro, use a opcao Clientes > Cadastrar cliente.\n");
            return;
        }
        if (usuarios[cliente].ativo == 0) {
            printf("Cliente inativo!\n");
            return;
        }
        printf("Cliente encontrado: %s\n", usuarios[cliente].nomeCompleto);
    }

    if (lerItens() == 0) {
        printf("Pedido cancelado.\n");
        return;
    }

    pagamento = escolherPagamento();
    if (pagamento == 0) {
        devolverItensTemporarios();
        return;
    }

    total = 0;
    printf("\n=== RESUMO DO PEDIDO ===\n");
    printf("Cliente: %s (ID %02d)\n", usuarios[cliente].nomeCompleto, cliente);
    for (i = 0; i < itensTotal; i++) {
        printf("%dx %s | R$%d cada | Subtotal: R$%d\n", itensQtd[i], produtos[itensProduto[i] - 1].nome,
               itensValor[i], itensQtd[i] * itensValor[i]);
        total += itensQtd[i] * itensValor[i];
    }
    printf("Pagamento: ");
    mostrarPagamento(pagamento);
    printf("\n");
    printf("VALOR TOTAL: R$%d\n", total);
    printf("Confirma o pedido? (1-Sim, 2-Nao): ");
    confirma = lerInteiro();

    if (confirma != 1) {
        devolverItensTemporarios();
        printf("Pedido cancelado.\n");
        return;
    }

    pedidos[totalPedidos].id = proximoPedido;
    pedidos[totalPedidos].cliente = cliente;
    copiarItensParaPedido(totalPedidos);
    pedidos[totalPedidos].pagamento = pagamento;
    pedidos[totalPedidos].status = 1;
    pedidos[totalPedidos].entregador = -1;
    pedidos[totalPedidos].ativo = 1;

    registrarHistorico(3, proximoPedido, cliente);
    printf("Pedido %d registrado com sucesso! Status: Pendente\n", proximoPedido);
    totalPedidos++;
    proximoPedido++;
}

/* O que cada nivel pode editar em um pedido (0 = vazio)
   Acoes: 1 = Produtos e quantidades, 2 = Cliente (ID), 3 = Forma de pagamento
   Linhas: 1 = Administrador, 2 = Atendente, 3 = Entregador, 4 = Cliente */
int acoesEditar[5][3] = {
    {0, 0, 0},
    {1, 2, 3},
    {1, 2, 3},
    {0, 0, 0},
    {1, 3, 0}
};

void mostrarAcaoEditar(int acao) {
    if (acao == 1) {
        printf("Produtos e quantidades");
    } else if (acao == 2) {
        printf("Cliente (ID)");
    } else {
        printf("Forma de pagamento");
    }
}

void editarPedido() {
    int id, pos, op, acao, i;
    int novaForma, novoCliente;
    int nivel = usuarios[usuarioAtual].nivel;

    printf("Numero do pedido: ");
    id = lerInteiro();
    pos = buscarPedido(id);

    if (pos == -1) {
        printf("Pedido nao encontrado!\n");
        return;
    }

    /* Pedido com entrega finalizada nao pode ser editado */
    if (pedidos[pos].status == 3) {
        printf("Pedido entregue nao pode ser editado!\n");
        return;
    }
    /* O cliente so altera pedido pendente */
    if (nivel == 4 && pedidos[pos].status == 2) {
        printf("Pedido em rota nao pode ser alterado!\n");
        return;
    }

    printf("O que deseja editar?\n");
    for (i = 0; i < 3 && acoesEditar[nivel][i] != 0; i++) {
        printf("%d - ", i + 1);
        mostrarAcaoEditar(acoesEditar[nivel][i]);
        printf("\n");
    }
    printf("0 - Cancelar\n");
    printf("Escolha: ");
    op = lerInteiro();

    acao = 0;
    if (op >= 1 && op <= 3 && acoesEditar[nivel][op - 1] != 0) {
        acao = acoesEditar[nivel][op - 1];
    }

    if (acao == 1) {
        if (pedidos[pos].status == 2) {
            printf("Pedido em rota nao pode ter produtos alterados!\n");
            return;
        }

        printf("Informe os produtos do pedido de novo.\n");
        devolverItensDoPedido(pos);              /* o que o pedido tinha volta para o estoque */

        if (lerItens() == 0) {
            retirarItensDoPedido(pos);           /* cancelou: o pedido antigo continua valendo */
            printf("Pedido nao alterado.\n");
        } else {
            copiarItensParaPedido(pos);
            registrarHistorico(4, pedidos[pos].id, 0);
            printf("Pedido editado com sucesso! Novo valor total: R$%d\n", totalDoPedido(pos));
        }
    }
    else if (acao == 2) {
        novoCliente = pedirIdCliente();
        if (novoCliente != -1) {
            pedidos[pos].cliente = novoCliente;
            registrarHistorico(4, pedidos[pos].id, 0);
            printf("Cliente atualizado com sucesso!\n");
        }
    }
    else if (acao == 3) {
        novaForma = escolherPagamento();
        if (novaForma != 0) {
            pedidos[pos].pagamento = novaForma;
            registrarHistorico(4, pedidos[pos].id, 0);
            printf("Forma de pagamento atualizada com sucesso!\n");
        }
    }
    else if (op != 0) {
        printf("Opcao invalida!\n");
    }
}

/* So Administrador */
void excluirPedido() {
    int id, pos, confirma;

    printf("Numero do pedido a excluir: ");
    id = lerInteiro();
    pos = buscarPedido(id);

    if (pos == -1) {
        printf("Pedido nao encontrado!\n");
        return;
    }

    printf("Tem certeza que deseja excluir o pedido %d? (1-Sim, 2-Nao): ", id);
    confirma = lerInteiro();

    if (confirma == 1) {
        /* Se o pedido ainda nao foi entregue, os botijoes voltam para o estoque */
        if (pedidos[pos].status != 3) {
            devolverItensDoPedido(pos);
        }
        pedidos[pos].ativo = 0;
        registrarHistorico(5, id, 0);
        printf("Pedido excluido com sucesso!\n");
    } else {
        printf("Exclusao cancelada.\n");
    }
}

/* Acoes do submenu de pedidos que cada nivel pode ver (0 = vazio)
   Acoes: 1 = Ver, 2 = Criar, 3 = Editar, 4 = Excluir
   Linhas: 1 = Administrador, 2 = Atendente, 3 = Entregador, 4 = Cliente */
int acoesPedidos[5][4] = {
    {0, 0, 0, 0},
    {1, 2, 3, 4},
    {1, 2, 3, 0},
    {1, 0, 0, 0},
    {1, 2, 3, 0}
};

void mostrarAcaoPedido(int acao) {
    int nivel = usuarios[usuarioAtual].nivel;

    if (acao == 1) {
        if (nivel == 4) {
            printf("Ver meus pedidos");
        } else {
            printf("Ver pedidos");
        }
    } else if (acao == 2) {
        if (nivel == 4) {
            printf("Fazer pedido");
        } else {
            printf("Criar pedido");
        }
    } else if (acao == 3) {
        printf("Editar pedido");
    } else {
        printf("Excluir pedido");
    }
}

void gerenciarPedidos(int nivel) {
    int op = -1;
    int acao, i;

    while (op != 0) {
        printf("\n=== PEDIDOS ===\n");
        for (i = 0; i < 4 && acoesPedidos[nivel][i] != 0; i++) {
            printf("%d - ", i + 1);
            mostrarAcaoPedido(acoesPedidos[nivel][i]);
            printf("\n");
        }
        printf("0 - Voltar\n");
        printf("Escolha: ");
        op = lerInteiro();

        acao = 0;
        if (op >= 1 && op <= 4 && acoesPedidos[nivel][op - 1] != 0) {
            acao = acoesPedidos[nivel][op - 1];
        }

        if (acao == 1) {
            verPedidos();
        }
        else if (acao == 2) {
            criarPedido();
        }
        else if (acao == 3) {
            editarPedido();
        }
        else if (acao == 4) {
            excluirPedido();
        }
        else if (op != 0) {
            printf("Opcao invalida!\n");
        }

        salvarDados(PRINCIPAL);
    }
}

/* ---------- ENTREGAS ---------- */

/* O Entregador ve so as entregas dele; os outros veem todas */
void verEntregas() {
    int i, c, contador = 0;
    int nivel = usuarios[usuarioAtual].nivel;

    printf("\n=== ENTREGAS ===\n");
    for (i = 0; i < totalPedidos; i++) {
        if (pedidos[i].ativo == 1 && (nivel != 3 || pedidos[i].entregador == usuarioAtual)) {
            c = pedidos[i].cliente;
            printf("Pedido %d | Cliente: %s (ID %02d) | Tel: %s | Endereco: %s | Entregador: ",
                   pedidos[i].id, usuarios[c].nomeCompleto, c, usuarios[c].telefone, usuarios[c].endereco);
            mostrarEntregador(pedidos[i].entregador);
            printf(" | Status: ");
            mostrarStatus(pedidos[i].status);
            printf("\n");
            contador++;
        }
    }
    if (contador == 0) {
        printf("Nenhuma entrega encontrada.\n");
    }
}

/* Administrador e Atendente escolhem quem vai entregar o pedido */
void atribuirEntregador() {
    int id, pos, idEntregador, i;

    printf("Numero do pedido: ");
    id = lerInteiro();
    pos = buscarPedido(id);

    if (pos == -1) {
        printf("Pedido nao encontrado!\n");
        return;
    }
    if (pedidos[pos].status != 1) {
        printf("So e possivel atribuir entregador em pedido pendente!\n");
        return;
    }

    printf("Entregadores disponiveis:\n");
    for (i = 0; i < totalUsuarios; i++) {
        if (usuarios[i].nivel == 3 && usuarios[i].ativo == 1) {
            printf("ID %02d | %s\n", i, usuarios[i].nomeCompleto);
        }
    }

    printf("ID do entregador: ");
    idEntregador = lerInteiro();

    if (buscarEntregadorPorId(idEntregador) == -1) {
        printf("Entregador nao encontrado!\n");
        return;
    }

    pedidos[pos].entregador = idEntregador;
    registrarHistorico(7, id, idEntregador);
    printf("Entregador atribuido com sucesso!\n");
}

/* novoStatus 2 = iniciar a entrega (em rota), 3 = finalizar (entregue) */
void alterarStatusEntrega(int novoStatus) {
    int id, pos;
    int nivel = usuarios[usuarioAtual].nivel;

    printf("Numero do pedido: ");
    id = lerInteiro();
    pos = buscarPedido(id);

    if (pos == -1) {
        printf("Pedido nao encontrado!\n");
        return;
    }
    if (pedidos[pos].entregador == -1) {
        printf("Atribua um entregador antes!\n");
        return;
    }
    if (nivel == 3 && pedidos[pos].entregador != usuarioAtual) {
        printf("Essa entrega nao e sua!\n");
        return;
    }
    if (novoStatus == 2 && pedidos[pos].status != 1) {
        printf("So um pedido pendente pode sair para entrega!\n");
        return;
    }
    if (novoStatus == 3 && pedidos[pos].status != 2) {
        printf("So um pedido em rota pode ser finalizado!\n");
        return;
    }

    pedidos[pos].status = novoStatus;
    registrarHistorico(6, id, novoStatus);

    if (novoStatus == 2) {
        printf("Entrega iniciada! Status: Em rota\n");
    } else {
        printf("Entrega finalizada! Status: Entregue\n");
    }
}

/* Acoes do menu de entregas que cada nivel pode ver (0 = vazio)
   Acoes: 1 = Ver, 2 = Atribuir entregador, 3 = Iniciar entrega, 4 = Finalizar entrega
   Linhas: 1 = Administrador, 2 = Atendente, 3 = Entregador, 4 = Cliente */
int acoesEntregas[5][4] = {
    {0, 0, 0, 0},
    {1, 2, 3, 4},
    {1, 2, 0, 0},
    {1, 3, 4, 0},
    {0, 0, 0, 0}
};

void mostrarAcaoEntrega(int acao) {
    if (acao == 1) {
        if (usuarios[usuarioAtual].nivel == 3) {
            printf("Minhas entregas");
        } else {
            printf("Ver entregas");
        }
    } else if (acao == 2) {
        printf("Atribuir entregador");
    } else if (acao == 3) {
        printf("Iniciar entrega (em rota)");
    } else {
        printf("Finalizar entrega (entregue)");
    }
}

void gerenciarEntregas(int nivel) {
    int op = -1;
    int acao, i;

    while (op != 0) {
        printf("\n=== ENTREGAS ===\n");
        for (i = 0; i < 4 && acoesEntregas[nivel][i] != 0; i++) {
            printf("%d - ", i + 1);
            mostrarAcaoEntrega(acoesEntregas[nivel][i]);
            printf("\n");
        }
        printf("0 - Voltar\n");
        printf("Escolha: ");
        op = lerInteiro();

        acao = 0;
        if (op >= 1 && op <= 4 && acoesEntregas[nivel][op - 1] != 0) {
            acao = acoesEntregas[nivel][op - 1];
        }

        if (acao == 1) {
            verEntregas();
        }
        else if (acao == 2) {
            atribuirEntregador();
        }
        else if (acao == 3) {
            alterarStatusEntrega(2);
        }
        else if (acao == 4) {
            alterarStatusEntrega(3);
        }
        else if (op != 0) {
            printf("Opcao invalida!\n");
        }

        salvarDados(PRINCIPAL);
    }
}

/* ---------- HISTORICO E BACKUP (so Administrador) ---------- */

void verHistorico() {
    int i, t, a, b;

    printf("\n=== HISTORICO ===\n");
    if (totalHistorico == 0) {
        printf("Nenhuma operacao registrada.\n");
        return;
    }

    for (i = 0; i < totalHistorico; i++) {
        t = historico[i].tipo;
        a = historico[i].a;
        b = historico[i].b;

        printf("%d | %s (ID %02d) | ", i + 1, usuarios[historico[i].usuario].nomeCompleto, historico[i].usuario);

        if (t == 1) {
            printf("Entrada de estoque: %s +%d\n", produtos[a - 1].nome, b);
        } else if (t == 2) {
            printf("Saida de estoque: %s -%d\n", produtos[a - 1].nome, b);
        } else if (t == 3) {
            printf("Pedido %d criado para o cliente ID %02d\n", a, b);
        } else if (t == 4) {
            printf("Pedido %d editado\n", a);
        } else if (t == 5) {
            printf("Pedido %d excluido\n", a);
        } else if (t == 6) {
            printf("Pedido %d mudou para o status ", a);
            mostrarStatus(b);
            printf("\n");
        } else if (t == 7) {
            printf("Pedido %d: entregador atribuido (ID %02d)\n", a, b);
        } else if (t == 8) {
            printf("Produto cadastrado: %s (preco R$%d)\n", produtos[a - 1].nome, b);
        } else if (t == 9) {
            printf("Preco de %s atualizado para R$%d\n", produtos[a - 1].nome, b);
        } else if (t == 10) {
            if (b == 1) {
                printf("Produto %s reativado\n", produtos[a - 1].nome);
            } else {
                printf("Produto %s inativado\n", produtos[a - 1].nome);
            }
        } else if (t == 11) {
            printf("Nome do produto %d alterado para %s\n", a, produtos[a - 1].nome);
        } else {
            printf("Backup dos dados realizado\n");
        }
    }
}

void fazerBackup() {
    salvarDados(BACKUP);
    registrarHistorico(12, 0, 0);
    printf("Backup realizado! Arquivo: %s\n", ARQUIVO_BACKUP);
}

/* ---------- MENU PRINCIPAL ---------- */

/* Acoes do menu principal que cada nivel pode ver (0 = vazio)
   Acoes: 1 = Ver estoque, 2 = Adicionar, 3 = Remover, 4 = Pedidos,
          5 = Gerenciar usuarios, 6 = Trocar de usuario, 7 = Sair,
          8 = Ver produtos e precos, 9 = Clientes, 10 = Produtos e precos,
          11 = Entregas, 12 = Historico, 13 = Backup
   Linhas: 1 = Administrador, 2 = Atendente, 3 = Entregador, 4 = Cliente */
int acoesMenu[5][14] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 3, 4, 11, 9, 10, 5, 12, 13, 6, 7, 0, 0},
    {1, 3, 4, 11, 9, 6, 7, 0, 0, 0, 0, 0, 0, 0},
    {1, 4, 11, 9, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0},
    {8, 4, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

void mostrarAcaoMenu(int acao) {
    if (acao == 1) {
        printf("Ver estoque");
    } else if (acao == 2) {
        printf("Adicionar quantidade");
    } else if (acao == 3) {
        printf("Remover quantidade");
    } else if (acao == 4) {
        printf("Pedidos");
    } else if (acao == 5) {
        printf("Gerenciar usuarios");
    } else if (acao == 6) {
        printf("Trocar de usuario");
    } else if (acao == 7) {
        printf("Sair do sistema");
    } else if (acao == 8) {
        printf("Ver produtos e precos");
    } else if (acao == 9) {
        printf("Clientes");
    } else if (acao == 10) {
        printf("Produtos e precos");
    } else if (acao == 11) {
        printf("Entregas");
    } else if (acao == 12) {
        printf("Historico");
    } else {
        printf("Backup dos dados");
    }
}

/* ---------- PROGRAMA PRINCIPAL ---------- */

int main() {
    char usuario[40];
    char senha[40];
    int logado;
    int voltar;
    int inicio;
    int escolha;
    int pos;
    int opcao;
    int nivel;
    int acao;
    int i;
    int sairPrograma = 0;

    carregarDados();

    printf("=======BEM VINDO A CONSIGAZ=======\n");

    /* Repete tela inicial + login + menu ate o usuario escolher sair do programa */
    while (sairPrograma == 0) {

        /* ----- TELA INICIAL ----- */
        inicio = 0;
        while (inicio != 1) {
            printf("\n=== SISTEMA CONSIGAZ ===\n");
            printf("1 - Entrar\n");
            printf("2 - Criar conta de cliente\n");
            printf("3 - Sair\n");
            printf("Escolha: ");
            inicio = lerInteiro();

            if (inicio == 2) {
                cadastrarCliente();
                salvarDados(PRINCIPAL);
            }
            else if (inicio == 3) {
                return 0;
            }
            else if (inicio != 1) {
                printf("Opcao invalida!\n");
            }
        }

        /* ----- LOGIN ----- */
        logado = 0;
        voltar = 0;

        while (logado == 0 && voltar == 0) {
            printf("Usuario (digite 0 para voltar): ");
            lerLinha(usuario, 40);

            if (ehZero(usuario) == 1) {
                voltar = 1;
            } else {
                printf("Senha: ");
                lerLinha(senha, 40);

                pos = buscarUsuario(usuario);

                if (pos != -1 && iguais(senha, usuarios[pos].senha) == 1 && usuarios[pos].ativo == 1) {
                    logado = 1;
                    usuarioAtual = pos;
                } else {
                    printf("Usuario ou senha incorretos!\n");
                    printf("1 - Tentar novamente\n");
                    printf("2 - Voltar ao menu principal\n");
                    printf("3 - Sair\n");
                    printf("Escolha: ");
                    escolha = lerInteiro();

                    if (escolha == 2) {
                        voltar = 1;
                    }
                    else if (escolha == 3) {
                        return 0;
                    }
                }
            }
        }

        /* ----- MENU: cada nivel so ve as opcoes que pode usar ----- */
        if (logado == 1) {
            printf("\nLogin realizado com sucesso!\n");
            nivel = usuarios[usuarioAtual].nivel;
            acao = 0;

            while (acao != 6 && acao != 7) {
                printf("\n=== MENU (");
                mostrarNivel(nivel);
                printf(": %s | ID %02d) ===\n", usuarios[usuarioAtual].nomeCompleto, usuarioAtual);

                for (i = 0; i < 14 && acoesMenu[nivel][i] != 0; i++) {
                    printf("%d - ", i + 1);
                    mostrarAcaoMenu(acoesMenu[nivel][i]);
                    printf("\n");
                }

                printf("Escolha uma opcao: ");
                opcao = lerInteiro();

                /* Traduz o numero digitado para a acao daquele nivel */
                acao = 0;
                if (opcao >= 1 && opcao <= 14 && acoesMenu[nivel][opcao - 1] != 0) {
                    acao = acoesMenu[nivel][opcao - 1];
                }

                if (acao == 1) {
                    verEstoque();
                }
                else if (acao == 2) {
                    adicionarEstoque();
                }
                else if (acao == 3) {
                    removerEstoque();
                }
                else if (acao == 4) {
                    gerenciarPedidos(nivel);
                }
                else if (acao == 5) {
                    gerenciarUsuarios();
                }
                else if (acao == 6) {
                    printf("Saindo da conta...\n");
                }
                else if (acao == 7) {
                    printf("Sistema encerrado.\n");
                    sairPrograma = 1;
                }
                else if (acao == 8) {
                    verProdutos();
                }
                else if (acao == 9) {
                    gerenciarClientes(nivel);
                }
                else if (acao == 10) {
                    gerenciarProdutos();
                }
                else if (acao == 11) {
                    gerenciarEntregas(nivel);
                }
                else if (acao == 12) {
                    verHistorico();
                }
                else if (acao == 13) {
                    fazerBackup();
                }
                else {
                    printf("Opcao invalida!\n");
                }

                salvarDados(PRINCIPAL);     /* guarda tudo no arquivo depois de cada acao */
            }

            usuarioAtual = -1;
        }
    }

    return 0;
}
