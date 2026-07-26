#include "iniciar.h"
#include <postgresql/libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

void Pause() {
    int voltar = 0;
    while (voltar != 1) {
        printf("\nDigite 1 para voltar: ");
        scanf("%d", &voltar);
    }
}

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
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            printf("%s\n", PQerrorMessage(conn));
            PQclear(res);
            Pause();
        } else {
            printf("Cadastro finalizado com sucesso!\n\n");
        }
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