#include <stdio.h>
#include <postgresql/libpq-fe.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct Sessao {
    double juros_saldo;
    int ID;
};

void Titulo() {
    printf("\n");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 50; j++) {
            if (i == 1 && j == 0) {
                printf("                 Bank Center\n");
            }
            printf("-");
        }
        printf("\n");
    }
    printf("\n");
}

// Regra Simples: Se a função chama o ID sem ser como ponteiro...
// É pq a função é pra depois de logado, e se a função não tem o ID, ou tem o ID como ponteiro...
// É pq a função é pra quando o usuario não está logado

// lista de opções à autentificação
void Autentificar(int *escolha) {
    Titulo();
    printf("Digite 1: para se CADASTRAR\n");
    printf("Digite 2: para LOGIN\n");
    printf("Digite 3: para SAIR\n");
    printf("\n\nvocê: ");
    scanf("%d", escolha);
}

void Cadastrar(PGconn *conn) {
    char usuario[50], senha[25];
    int idade;

    printf("Digite seu nome: ");
    scanf("%49s", usuario);

    printf("\nDigite sua idade: ");
    scanf("%d", &idade);

    // verificação de idade
    // Se o usuario não possui idade minima, o cadastro não vai progredir
    if (idade < 16) {
        printf("\nVocê não pode se cadastrar ainda\n");
    } else {
        printf("Digite sua senha: ");
        scanf("%24s", senha);


        char sql[300];
        snprintf(sql, sizeof(sql),
            "INSERT INTO sistema (usuario, idade, senha, saldo) VALUES ('%s', %d, crypt('%s', gen_salt('bf')), 0);",
            usuario, idade, senha
        );

        PGresult *res = PQexec(conn, sql);
        // Caso ocorrer um erro, o código via informar
        printf("%s\n", PQerrorMessage(conn));

        printf("Cadastro finalizado com sucesso!\n\n");
        PQclear(res);
    }

}

void Login(PGconn *conn, int *ID) {
    char usuario[50], senha[25];

    printf("Digite seu nome: ");
    scanf("%49s", usuario);

    printf("Digite sua senha: ");
    scanf("%24s", senha);
    printf("\n\n");

    // vamos executar um comando de loop de usuarios que possuem o mesmo nome
    // para encontrar um usuario corresponde as informações no banco de dados
    char const *sql[2] = {usuario, senha};
    PGresult *res = PQexecParams(
        conn, "SELECT * FROM sistema where usuario=$1 AND senha=crypt($2, senha);",
        2,
        NULL,
        sql,
        NULL,
        NULL,
        0
    );

    // os resultados que 'res' vai lançar obrigatoriamente serão mesma senha e usuario, então portanto...
    // se existir, logo na 1ª execução do loop já sera sucesso
    // então só precisamos que o numero da lista seje > 0, que já encontramos o usuario
    int linha = PQntuples(res);
    if (linha > 0) {
        *ID = atoi(PQgetvalue(res, 0, 0));
    } else {
        printf("\nUsuario não foi encontrado\n");
    }
        
    
    PQclear(res);

}


void Opcoes(int *opcao) {
    Titulo();
    printf("\n\nDigite 1: para ver seu STATUS\n");
    printf("Digite 2: para DEPOSITAR\n");
    printf("Digite 3: para SACAR\n");
    printf("Digite 4: para MUDAR a senha\n");
    printf("Digite 5: para fazer PIX\n");
    printf("Digite 6: para SAIR do seu login\n");
    printf("\n\nvocê: ");
    scanf("%d", opcao);
}

void Pause() {
    int voltar = 0;
    while (voltar != 1) {
        printf("\nDigite 1 para voltar: ");
        scanf("%d", &voltar);
    }
}

bool Verificar_senha(PGconn *conn, int ID) {

    char senha[25], ID_string[100];

    printf("\n Digite sua senha: ");
    scanf("%24s", senha);

    snprintf(ID_string, sizeof(ID_string), "%d", ID);
    char const *sql[2] = {ID_string, senha};

    PGresult *res = PQexecParams(
        conn, "SELECT * FROM sistema where id=$1 AND senha=crypt($2, senha);",
        2,
        NULL,
        sql,
        NULL, NULL, 0
    );

    int linha = PQntuples(res);

    for (int i = 0; i < linha; i++) {
        if (ID == atoi(PQgetvalue(res, i, 0))) {
            PQclear(res);
            return true;
        }
    }
    PQclear(res);
    return false;
}


void Status(PGconn *conn, int ID) {
    char sql[300];
    // colocando variaveis dentro de um texto da variavel 'sql'
    // Enfim enviamos o comando dessa variavel ('sql') para o postgres
    // ele vai retornar o nome, idade e saldo do usuario
    snprintf(sql, sizeof(sql), 
        "SELECT usuario, idade, saldo FROM sistema where id=%d;", ID
    );

    PGresult *res = PQexec(conn, sql);

    printf("%s\n", PQerrorMessage(conn));

    int linha = PQntuples(res);

    for (int i = 0; i < linha; i++) {
        printf("Chave: %d\n", ID);
        printf("Nome: %s\n", PQgetvalue(res, i, 0));
        printf("Idade: %d\n", atoi(PQgetvalue(res, i, 1)));
        printf("Saldo: R$:%.2f\n\n", atof(PQgetvalue(res, i, 2)));
    }
    
    PQclear(res);
    Pause();
}

// Essa função tem como objetivo pegar o saldo do usuario no banco de dados e retornar o valor do saldo
// nome tbm bem auto explicativo KKK
double Buscar_Saldo(PGconn *conn, int ID) {
    char sql[300];

    snprintf(sql, sizeof(sql),
    "SELECT saldo FROM sistema where id=%d", ID
    );
    PGresult *res = PQexec(conn, sql);

    double saldo = atof(PQgetvalue(res, 0, 0));

    PQclear(res);

    return saldo;
}

void Reduzir_Saldo(PGconn *conn, int ID, double reduzir) {
    double saldo = Buscar_Saldo(conn, ID);
    double novo_saldo = saldo-reduzir;
    char sql[300];
    snprintf(sql, sizeof(sql), "UPDATE sistema SET saldo=%f where id=%d;", novo_saldo, ID);
    PGresult *res = PQexec(conn, sql);
    PQclear(res);
}

void Somar_Saldo(PGconn *conn, int ID, double soma) {
    double saldo = Buscar_Saldo(conn, ID);
    double novo_saldo = saldo+soma;
    char sql[300];
    snprintf(sql, sizeof(sql), "UPDATE sistema SET saldo=%f where id=%d;", novo_saldo, ID);
    PGresult *res = PQexec(conn, sql);
    PQclear(res);
}

void Depositar(PGconn *conn, int ID) {
    double deposito = 0.0;
    // colocamos o valor inserido pelo usuario em  'deposito'
    // se o valor for negativo, o usuario tera que digitar o valor novamente
    while (deposito <= 0.0) {

        printf("\nQuanto deseja depositar? R$:");
        scanf("%lf", &deposito);

        if (deposito <= 0) {
            printf("\nValor Inválido\nTente novamente\n");
        }
    }
    Somar_Saldo(conn, ID, deposito);

    printf("\nDeposito Realizado com sucesso!");
    Pause();
}

void Sacar(PGconn *conn, int ID) {
    double saldo = Buscar_Saldo(conn, ID);
    if (Verificar_senha(conn, ID)) {
        double saque = 0;
        // pegamos o valor inserido pelo usuario em 'saque', apos verificamos se o valor for negativo ou nulo, e se for...
        // o usuario tera que digitar o valor novamente
        while (saque <= 0.0 || saque > saldo) {
            printf("\nDigite o valor que deseja sacar: R$:");
            scanf("%lf", &saque);

            if (saque <= 0.0 || saque > saldo) {
                printf("\nValor Inválido\nTente novamente");
            }
        }
        
        Reduzir_Saldo(conn, ID, saque);
        printf("\nSaque Concluido!");
    } else {
        printf("\nSenha Incorreta\n");
    }
        Pause();
}

void Mudar_senha(PGconn *conn, int ID) {
    if (Verificar_senha(conn, ID)) {
        char new_senha[25], sql[300];
        printf("\nDigite sua nova senha: ");
        scanf("%24s", new_senha);

        snprintf(sql, sizeof(sql),
            "UPDATE sistema SET senha = crypt('%s', gen_salt('bf')) where id=%d;", new_senha, ID
        );
        PGresult *res = PQexec(conn, sql);
        printf("\nTarefa finalizada!\n");
        PQclear(res);

    } else {
        printf("\nSenha Incorreta\n");
    }
    Pause();
}

void Transferir(PGconn *conn, int ID) {
    double saldo = Buscar_Saldo(conn, ID);
    double PIX = 0;
    // pegamos o valor inserido pelo usuario em 'saque', apos verificamos se o valor for negativo ou nulo, e se for...
    // o usuario tera que digitar o valor novamente
    while (PIX <= 0.0 || PIX > saldo) {
        printf("\nDigite o valor que deseja transferir: R$:");
        scanf("%lf", &PIX);

        if (PIX <= 0.0 || PIX > saldo) {
            printf("\nValor Inválido\nTente novamente");
        }
    }
    int chave=0;

    do {
        printf("\nDigite a chave PIX: ");
        scanf("%d", &chave);
        printf("\n");
    } while(!Verificar_senha(conn, ID));

    Reduzir_Saldo(conn, ID, PIX);
    Somar_Saldo(conn, chave, PIX);

    printf("\nPix Realizado com sucesso!\n");
    Pause();
}

int main() {

    //  inicializando as ferramentas
    struct Sessao s = {
        .juros_saldo = 0.01, // Juros sobre o saldo de 1% a.m.
        .ID = -1
    };

    //  conexão ao banco de dados
    PGconn *conn = PQconnectdb(
        "host=localhost dbname=postgres user=postgres password='123'"
    );

    //  Se der erro na conexão o programa fechara imediatamente, informando erro
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Erro\n");
        return 1;
    }

    int escolha;

    //  loop de bloqueio
    // o sistema de dos dois loops While funciona com base no ID
    // se o valor do ID for negativo, ele ficara preso na pagina de bloqueo com opcoes de login, cadastro e sair
    // se o valor do ID for positivo ou 0, já localizamos o usuario e podemos ativar a conta dele

    while (true) {
        while(s.ID < 0) {
            Autentificar(&escolha);

            switch (escolha) {
                case 1: Cadastrar(conn); break;

                case 2: Login(conn, &s.ID); break;

                case 3: printf("\nVolte Sempre!\n"); return 0; 
            }
        }

        while(s.ID >= 0) {
            Opcoes(&escolha);
            switch (escolha) {
                case 1: Status(conn, s.ID); break;

                case 2: Depositar(conn, s.ID); break;

                case 3: Sacar(conn, s.ID); break;

                case 4: Mudar_senha(conn, s.ID); break;

                case 5: Transferir(conn, s.ID); break;

                case 6: s.ID = -1; break;
            }
        }
    }

    printf("Volte sempre!\n");


    PQfinish(conn);
    return 0;
}
