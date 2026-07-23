# Sistema_Bancario
Esse repositorio tem foco de apresentar o processo de construir um sistema bancario escrito em C, utilizando ferramentas como PostgresSQL. não foi feito para fins industriais, e sim educacionais.

Para esses projetos é recomendado você ter um compilador C, e ter instalado o banco de dados POSTGRES.


Execute no seu banco de dados

CREATE TABLE IF NOT EXISTS sistema (
id SERIAL PRIMARY KEY,
usuario TEXT,
idade INTEGER,
senha TEXT,
saldo DOUBLE PRECISION
);
