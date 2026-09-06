#create database pharma_database;

#use pharma_database;


################# Filial ##########################

create table branches (
numeration_branch varchar(5) unique not null primary key,
cep_branch char(8) not null,
street_branch varchar(100) not null,
neighborhood_branch varchar(50) not null,
city_branch varchar(50) not null,
uf_branch char(2) not null
);

################## Users ######################
create table administrators (
id_administrator char(8) unique not null primary key,
name_administator varchar(50) not null,
lastname_administrator varchar(50),
email_administrator varchar(100) not null unique,
password_administrator varchar(255) not null
);

create table managers (
id_manager char(8) unique not null primary key,
name_manager varchar(50) not null,
lastname_manager varchar(50) not null,
internal_code_manager char(6) not null unique,
branch_manager char(3) not null, 
password_manager varchar(255) not null,
foreign key (branch_manager) references branches(numeration_branch)
);

create table pharmaceuts (
id_pharmaceut char(8) unique not null primary key,
name_pharmaceut varchar(50) not null,
lastname_pharmaceut varchar(50) not null,
crf_pharmaceuts varchar(8) not null unique
);

create table attendants (
id_attendant char(8) not null primary key unique,
name_attendant varchar(50) not null,
internal_code_attendant char(6) not null unique,
branch_attendant varchar(5) not null,
foreign key (branch_attendant) references branches(numeration_branch)
);

#################### Estoque #######################

create table stock (
id_stock char(8) not null primary key unique,
medication_stock varchar (100) not null,
branch_stock varchar(5) not null,
quantity_stock varchar(5) not null,
foreign key (branch_stock) references branches(numeration_branch)
);