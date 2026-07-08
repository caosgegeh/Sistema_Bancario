#include <stdio.h>
#include <postgresql/libpq-fe.h>
#include <string.h>
#include <stdlib.h>

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
    scanf("%s", usuario);

    printf("\nDigite sua idade: ");
    scanf("%d", &idade);

    // verificação de idade
    // Se o usuario não possui idade minima, o cadastro não vai progredir
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
        // Caso ocorrer um erro, o código via informar
        printf("%s\n", PQerrorMessage(conn));

        printf("Cadastro finalizado com sucesso!\n\n");
        PQclear(res);
    }

}

void Login(PGconn *conn, int *ID) {
    char usuario[50], senha[25];

    printf("Digite seu nome: ");
    scanf("%s", usuario);

    printf("Digite sua senha: ");
    scanf("%s", senha);
    printf("\n\n");

    // vamos executar um comando de loop de usuarios que possuem o mesmo nome
    // para enconrar um usuario corresponde as informações no banco de dados
    char sql[300];
    snprintf(sql, sizeof(sql), "SELECT * FROM sistema where usuario='%s';", usuario);

    PGresult *res = PQexec(conn, sql);
    
    int linha = PQntuples(res);
    for (int i = 0; i < linha; i++) {

        // Comparação de strings
        // Caso as informações recebidas dos usuario seje correspondente as informações presentes
        // no banco de dados, vamos pegar o ID no banco de dados e armazenalo na
        // variavel "ID" da struct
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
    Titulo();
    printf("\n\nDigite 1: para ver seu STATUS\n");
    printf("Digite 2: para DEPOSITAR\n");
    printf("Digite 3: para SACAR\n");
    printf("Digite 4: para SAIR\n");
    printf("\n\nvocê: ");
    scanf("%d", opcao);
}

// função pra pausar a tela 
// nome bem auto explicativo né kkkk
void Pause() {
    int voltar = 0;
    while (voltar != 1) {
        printf("\nDigite 1 para voltar: ");
        scanf("%d", &voltar);
    }
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
    // para autalizar o saldo do usuario, fazemos a soma do que ele já tem, com oq ele quer aplicar
    deposito += Buscar_Saldo(conn, ID);

    char sql[300];
    snprintf(sql, sizeof(sql),
        "UPDATE sistema SET saldo=%.2f where id=%d", deposito, ID
    );

    PGresult *res = PQexec(conn, sql);
    PQclear(res);

    printf("\nDeposito Realizado com sucesso!");
    Pause();
}

void Sacar(PGconn *conn, int ID) {
    double saque = 0;
    // pegamos o valor inserido pelo usuario em 'saque', apos verificamos se o valor for negativo ou nulo, e se for...
    // o usuario tera que digitar o valor novamente
    while (saque <= 0.0 || saque > Buscar_Saldo(conn, ID)) {
        printf("\nDigite o valor que deseja sacar: R$:");
        scanf("%lf", &saque);

        if (saque <= 0.0 || saque > Buscar_Saldo(conn, ID)) {
            printf("\nValor Inválido\nTente novamente");
        }
    }
    // criamos a variavel 'novo_saldo' que vai pegar o valor que ele já tem no banco de dados e subtrair com o saque
    double novo_saldo = (Buscar_Saldo(conn, ID) - saque);
    char sql[300];
    snprintf(sql, sizeof(sql), "UPDATE sistema SET saldo=%f where id=%d;", novo_saldo, ID);
    PGresult *res = PQexec(conn, sql);
    PQclear(res);
    printf("\nSaque Concluido!");
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
        "host=localhost dbname=meubanco user=caos password=123"
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
    while(s.ID < 0) {
        Autentificar(&escolha);

        if (escolha == 1) {
            Cadastrar(conn);
        }
        if (escolha == 2) {
            Login(conn, &s.ID);
        }
        if (escolha == 3) {
            printf("\nVolte sempre!\n");
            return 0;
        }
    }

    while(s.ID >= 0) {
        Opcoes(&escolha);

        if (escolha == 1) {
            Status(conn, s.ID);
        }

        if (escolha == 2) {
            Depositar(conn, s.ID);
        }

        if (escolha == 3) {
            Sacar(conn, s.ID);
        }

        if (escolha == 4) {
            s.ID = -1;
        }
    }
    printf("Volte sempre!\n");


    PQfinish(conn);
    return 0;
}
