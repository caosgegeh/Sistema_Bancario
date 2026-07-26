#include <postgresql/libpq-fe.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "saldo.h"
#include "iniciar.h"
#include "seguranca_e_pix.h"

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

bool Reduzir_Saldo(PGconn *conn, int ID, double reduzir) {

    char str_reduzir[100], str_id[100];

    snprintf(str_reduzir, sizeof(str_reduzir), "%.2f", reduzir);
    snprintf(str_id, sizeof(str_id), "%d", ID);

    char const *parametros[2] = {str_reduzir, str_id};
    
    PGresult *res = PQexecParams(conn,
        "UPDATE sistema SET saldo=saldo-$1 where id=$2",
        2, NULL, parametros, NULL, NULL, 0
    );
    if(PQresultStatus(res) == PGRES_COMMAND_OK) {
        PQclear(res);
        return true;
    }
    PQclear(res);
    return false;
}

bool Somar_Saldo(PGconn *conn, int ID, double soma) {
    double novo_saldo = soma;
    char str_id[100], str_soma[100];

    snprintf(str_soma, sizeof(str_soma), "%.2f", soma);
    snprintf(str_id, sizeof(str_id), "%d", ID);

    char const *parametros[2] = {str_soma, str_id};
    
    PGresult *res = PQexecParams(
        conn,
        "UPDATE sistema SET saldo=saldo+$1 where id=$2;",
        2, NULL, parametros, NULL, NULL, 0
    );

    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        PQclear(res);
        return true;
    }

    PQclear(res);
    return false;
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
    if (Somar_Saldo(conn, ID, deposito)) {
        printf("\nDeposito Realizado com sucesso!");
    } else {
        printf("\nErro: Não foi possivel concluir seu deposito.\n");
    }
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
        
        if (Reduzir_Saldo(conn, ID, saque)) {
            printf("\nSaque Concluido!");
        } else {
            printf("\nErro: não foi possivel concluir o saque\nTente novamente mais tarde");
        }

    } else {
        printf("\nSenha Incorreta\n");
    }
        Pause();
}


