#ifndef seguranca_e_pix_h
#define seguranca_e_pix_h
#include <postgresql/libpq-fe.h>
#include <stdbool.h>

bool Encontrar_Chave(PGconn *conn, int ID);

bool Verificar_senha(PGconn *conn, int ID);

void Mudar_senha(PGconn *conn, int ID);

void Transferir(PGconn *conn, int ID);

#endif