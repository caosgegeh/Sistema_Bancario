#ifndef iniciar_h
#define iniciar_h
#include <postgresql/libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double juros_saldo;
    int ID;
} Sessao;

void Pause();

void Titulo();

void Autentificar(int *escolha);

void Cadastrar(PGconn *conn);

void Login(PGconn *conn, int *ID);

#endif