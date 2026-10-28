-- Banco de dados do PIM II - Sistema de estoque correlacional farmacêutico
-- Autores: José Antonio de Oliveira Rosa && Kaio Eduardo Braga Barbosa

-- nome do banco => pharma_database
create database pharma_database;

-- Para a edição/criação das tabelas, precisa ser selecionado o banco de dados.
use pharma_database;

-- Abaixo será criado as tabelas do sistema, caso seja necessário fazer qualquer alteração execute o comando de DROP
-- e em seguida edite a estrutura da tabela.

-- Iniciaremos pelos possíveis usuários do sistema
create table diretor (
id_diretor char(6) unique not null primary key,
nome_diretor varchar(50) not null,
sobrenome_diretor varchar(50) not null,
codigo_interno varchar(6) not null,
email_diretor varchar(255) not null unique,
senha_diretor varchar(255) not null
);

create table endereco(
id_endereco char(6) unique not null primary key,
cep char(8) not null,
logradouro varchar(50) not null,
numero varchar(10) not null,
complemento varchar(20),
bairro varchar(50) not null,
cidade varchar(50) not null,
estado varchar (50) not null
);

create table filial(
id_filial char(6) unique not null primary key,
id_endereco char(6) not null,
id_diretor char(6) not null,
foreign key (id_endereco) references endereco(id_endereco),
foreign key (id_diretor) references diretor(id_diretor)
);

CREATE TABLE equipamentos (
    id_equipamentos char(6) unique primary key not null,
    modelo varchar(50) not null,
    fabricante varchar(50) not null,
    especificacoes varchar(50) not null,
    garantia varchar(50) not null
);

create table medicamento(
id_medicamento char(6) unique not null primary key,
principio_ativo varchar(50) not null,
forma_farmaceutica varchar(50) not null,
concentracao varchar(50) not null,
tarja varchar(50) not null,
receita bool not null, #0 -> Precisa 1 -> Não precisa
tipagem varchar(50) not null
);

create table produto(
id_produto char(6) unique not null primary key,
nome_produto varchar(100) not null, 
marca_produto varchar(50) not null,
descricao_produto varchar(255) not null,
preco varchar(20) not null,
codigo_de_barras varchar(30) not null,
quantidade_produto varchar(10) not null,
id_filial char(6) not null,
foreign key(id_filial) references filial (id_filial)
);

CREATE TABLE dermocosmetico (
id_dermo char(6) unique primary key not null,
tipo_pele varchar(50) not null,
volume varchar(50) not null,
fator_protecao varchar(50) not null,
indicacao varchar(50) not null,
id_produto char(6) not null,
foreign key (id_produto) references produto(id_produto)
);

create table produto_medicamento (
id_produto char(6) not null,
id_medicamento char(6) not null,
foreign key (id_produto) references produto (id_produto),
foreign key (id_medicamento) references medicamento (id_medicamento)
);

create table produto_dermocosmetico (
id_produto char(6) not null,
id_dermo char(6) not null,
foreign key (id_produto) references produto (id_produto),
foreign key (id_dermo) references dermocosmetico (id_dermo)
);

create table gerente (
id_gerente char(6) unique not null primary key,
nome_gerente varchar(50) not null,
sobrenome_gerente varchar(50) not null,
codigo_interno char(6) unique not null,
email_gerente varchar(255) unique not null,
senha_gerente varchar(255) not null,
id_filial char(6) not null,
id_diretor char(6) not null,
foreign key (id_diretor) references diretor(id_diretor),
foreign key (id_filial) references filial(id_filial)
);

create table gerente_filial (
id_gerente char(6) not null,
id_filial char(6) not null,
foreign key (id_gerente) references gerente (id_gerente),
foreign key (id_filial) references filial (id_filial)
);

create table gerente_produto (
id_gerente char(6) not null,
id_produto char(6) not null,
foreign key (id_gerente) references gerente (id_gerente),
foreign key (id_produto) references produto (id_produto)
);

create table farmaceutico (
id_farmaceutico char(6) unique not null primary key,
nome_farmaceutico varchar(50) not null,
sobrenome_farmaceutico varchar(50) not null,
codigo_interno char(8) unique not null,
crf_farmaceutico varchar(15) unique not null,
email_farmaceutico varchar(255) unique not null,
senha_farmaceutico varchar(255) not null,
id_filial char(6) not null,
id_gerente char(6) not null,
foreign key (id_filial) references filial(id_filial),
foreign key (id_gerente) references gerente(id_gerente)
);

create table farmaceutico_filial (
id_farmaceutico char(6) not null,
id_filial char(6) not null,
foreign key (id_farmaceutico) references farmaceutico (id_farmaceutico),
foreign key (id_filial) references filial (id_filial)
);

create table atendente (
id_atendente char(6) unique not null primary key,
nome_atendente varchar(50) not null,
sobrenome_atendente varchar(50) not null,
email_atendente varchar(255) unique not null,
senha_atendente varchar(255) not null,
id_filial char(6) not null,
id_gerente char(6) not null,
foreign key (id_filial) references filial(id_filial),
foreign key (id_gerente) references gerente(id_gerente)
);

create table atendente_filial (
id_atendente char(6) not null,
id_filial char(6) not null,
foreign key (id_atendente) references atendente (id_atendente),
foreign key (id_filial) references filial (id_filial)
);

CREATE VIEW vw_login_usuarios AS
SELECT 
    id_diretor AS id_usuario, 
    email_diretor AS email, 
    senha_diretor AS senha, 
    'diretor' AS perfil 
FROM diretor
UNION ALL
SELECT 
    id_gerente, 
    email_gerente, 
    senha_gerente, 
    'gerente' 
FROM gerente
UNION ALL
SELECT 
    id_farmaceutico, 
    email_farmaceutico, 
    senha_farmaceutico, 
    'farmaceutico' 
FROM farmaceutico
UNION ALL
SELECT 
    id_atendente, 
    email_atendente, 
    senha_atendente, 
    'atendente' 
FROM atendente;

# ----------------------------------------------------------------------------------------------------
# ------------------------- TESTES TESTE TESTE TESTE TESTE TESTE TESTE TESTE -------------------------
# ----------------------------------------------------------------------------------------------------

#drop table diretor;

#INSERT INTO diretor (id_diretor, nome_diretor ,sobrenome_diretor, codigo_interno, email_diretor, senha_diretor) 
#VALUES ('Y09BAR', 'Kaio Eduardo', 'Braga Barbosa', '000001', 'kaio.braga@gmail', '123123');

#select * from filial;
#select * from endereco;
#select * from gerente;
#select * from diretor;
#select * from farmaceutico;

#SELECT id_usuario, senha, perfil FROM vw_login_usuarios WHERE email = 'kaio.bragab@gmail.com';

#delete from endereco where id_endereco = 'Y09BAR';

#delete from diretor where id_diretor = 'Y09BAR';

#drop database pharma_database;