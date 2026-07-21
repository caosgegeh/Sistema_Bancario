#include <stdio.h>
#include <stdbool.h>
#include <postgresql/libpq-fe.h>
#include <string.h>
#include <stdlib.h>

struct Ferramentas {
    double saldo, juros_saldo;
    int ID;
};

void Autentificar(int *escolha) {
    printf("Digite 1: para se CADASTRAR\n");
    printf("Digite 2: para se LOGAR\n");
    printf("Digite 3: para SAIR\n");
    printf("\n\nvocê: ");
    scanf("%d", escolha);
}

void Cadastrar(PGconn *conn) {
    char usuario[50], senha[25];
    int idade;
    printf("Digite seu nome: ");
    scanf("%s", usuario);
    printf("\nDigite sua idade: ");
    scanf("%d", &idade);
    if (idade < 16) {
        printf("\nVocê não pode se cadastrar ainda\n");
    } else {
        printf("Digite sua senha: ");
        scanf("%s", senha);

        char sql[300];
        snprintf(sql, sizeof(sql),
            "INSERT INTO sistema (usuario, idade, senha, saldo) VALUES ('%s', %d, '%s', 0);",
            usuario, idade, senha
        );

        PGresult *res = PQexec(conn, sql);
        printf("%s\n", PQerrorMessage(conn));

        printf("Cadastro finalizado com sucesso!\n\n");
        PQclear(res);
    }

}

void Logar(PGconn *conn, int *ID) {
    char usuario[50], senha[25];

    printf("Digite seu nome: ");
    scanf("%s", usuario);

    printf("Digite sua senha: ");
    scanf("%s", senha);
    printf("\n\n");

    PGresult *res = PQexec(conn, "SELECT * FROM sistema;");
    
    int linha = PQntuples(res);
    for (int i = 0; i < linha; i++) {
        if (strcmp(PQgetvalue(res, i, 1), usuario) == 0 &&
            strcmp(PQgetvalue(res, i, 3), senha) == 0
        ) {
            printf("Login concluido!\n");
            *ID = atoi(PQgetvalue(res, i, 0));
            break;
        }
    }
    PQclear(res);

}

void Opcoes(int *opcao) {
    printf("\n\nDigite 1: para ver seu STATUS\n");
    printf("Digite 2: para ver DEPOSITAR\n");
    printf("Digite 3: para SACAR\n");
    printf("Digite 4: para NEGOCIAR\n");
    printf("Digite 5: para SAIR\n");
    printf("\n\nvocê: ");
    scanf("%d", opcao);
}

int main() {

    //  inicializando as ferramentas
    struct Ferramentas f = {
        .saldo = 0,
        .juros_saldo = 0.01, // Juros sobre o saldo de 1% a.m.
        .ID = -1
    };

    PGconn *conn = PQconnectdb(
        "host=localhost dbname=postgres user=postgres password='123456'"
    );

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Erro\n");
        return 1;
    }

    int escolha;

    //  loop de bloqueio
    while(f.ID < 0) {
        Autentificar(&escolha);

        if (escolha == 1) {
            Cadastrar(conn);
        }
        if (escolha == 2) {
            Logar(conn, &f.ID);
        }
        if (escolha == 3) {
            printf("\nVolte sempre!\n");
            return 0;
        }
    }
    while(f.ID >= 0) {
        Opcoes(&escolha);

        if (escolha == 5) {
            f.ID = -1;
        }
    }
    printf("Volte sempre!\n");


    PQfinish(conn);
    return 0;
}
