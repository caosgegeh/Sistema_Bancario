#include <stdio.h>
#include <postgresql/libpq-fe.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct Sessao {
    double juros_saldo;
    int ID;
};

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
        PQclear(res);
        res = PQexec(conn, "ROLLBACK;");
        PQclear(res);
    }
    
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
