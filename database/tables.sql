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
email_diretor varchar(255) not null unique,
senha_diretor varchar(255) not null
);

create table gerente (
id_gerente char(6) unique not null primary key,
nome_gerente varchar(50) not null,
sobrenome_gerente varchar(50) not null,
codigo_interno char(8) unique not null,
email_gerente varchar(255) unique not null,
senha_gerente varchar(255) not null,
id_filial char(6) not null,
id_diretor char(6) not null,
foreign key (id_diretor) references diretor(id_diretor),
foreign key (id_filial) references filial(id_filial)
);

create table farmaceutico (
id_farmaceutico char(6) unique not null primary key,
nome_farmaceutico varchar(50) not null,
sobrenome_farmaceutico varchar(50) not null,
crf_farmaceutico varchar(15) unique not null,
email_farmaceutico varchar(255) unique not null,
senha_farmaceutico varchar(255) not null,
id_filial char(6) not null,
id_gerente char(6) not null,
foreign key (id_filial) references filial(id_filial),
foreign key (id_gerente) references gerente(id_gerente)
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

-- Aqui finalizamos os usuários e os seus tipos. O próximo passo vai ser criar a tabela da filial.

create table filial(
id_filial char(6) unique not null primary key,
id_endereco char(6) not null,
id_diretor char(6) not null,
foreign key (id_endereco) references endereco(id_endereco),
foreign key (id_diretor) references diretor(id_diretor)
);

create table endereco(
id_endereco char(6) unique not null,
cep char(8) not null,
logradouro varchar(50) not null,
numero varchar(10) not null,
complemento varchar(20),
bairro varchar(50) not null,
cidade varchar(50) not null,
estado varchar (50) not null
);