#include <stdio.h>
#include <postgresql/libpq-fe.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "iniciar.h"
#include "saldo.h"
#include "seguranca_e_pix.h"


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


int main() {

    //  inicializando as ferramentas
    Sessao s = {
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

                case 3:
                printf("\nVolte Sempre!\n");
                PQfinish(conn);
                return 0; 
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