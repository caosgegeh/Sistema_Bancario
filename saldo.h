#ifndef saldo_h
#define saldo_h
#include <postgresql/libpq-fe.h>
#include <stdbool.h>

double Buscar_Saldo(PGconn *conn, int ID);

bool Reduzir_Saldo(PGconn *conn, int ID, double reduzir);

bool Somar_Saldo(PGconn *conn, int ID, double soma);

void Depositar(PGconn *conn, int ID);

void Sacar(PGconn *conn, int ID);

#endif