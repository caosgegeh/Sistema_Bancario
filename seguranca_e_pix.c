#include "seguranca_e_pix.h"
#include "saldo.h"
#include "iniciar.h"
#include <postgresql/libpq-fe.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool Encontrar_Chave(PGconn *conn, int ID) {
    char str_id[100];
    snprintf(str_id, sizeof(str_id), "%d", ID);

    char const *parametros[1] = {str_id};

    PGresult *res = PQexecParams(
        conn,
        "SELECT * FROM sistema where id=$1",
        1,
        NULL,
        parametros,
        NULL,
        NULL,
        0
    );

    if (PQntuples(res) > 0) {
        printf("\n%s encontrado!\n", PQgetvalue(res, 0, 1));
        PQclear(res);
        return true;
    }
    printf("\nID/Chave não localizados\n");
    PQclear(res);
    return false;
    
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
    // Usuario ira digitar a chave pix e a senha dele, se uma dessas informações digitadas estiver incorretas ou não serem encontradas
    // o Usuario permanecera em loop
    do {
        do {

            printf("\nDigite a chave PIX: ");
            scanf("%d", &chave);
            printf("\n");

        } while(!Encontrar_Chave(conn, chave));

    } while(!Verificar_senha(conn, ID));

    PGresult *res = PQexec(conn, "BEGIN;");
    PQclear(res);

    // Caso conseguimos transferir, vamos salvar, caso não, retornamos o valor pro usuario.
    if (Reduzir_Saldo(conn, ID, PIX) && Somar_Saldo(conn, chave, PIX)) {

        res = PQexec(conn, "COMMIT;");
        printf("\nPix Realizado com sucesso!\n");
        
        PQclear(res);
    } else {
        printf("\n Não foi possivel fazer a transferência");
        res = PQexec(conn, "ROLLBACK;");
        PQclear(res);
    }
    
    Pause();
}

