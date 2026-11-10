# PIM II — Sistema de Estoque Farmacêutico

Sistema de consulta e gerenciamento de estoque para uma rede farmacêutica com múltiplas filiais. A proposta é centralizar as informações dos produtos e permitir que os usuários consultem a disponibilidade de um medicamento em toda a rede, inclusive quando ele não estiver disponível na filial atual.

> **Status:** em desenvolvimento. Já há login para os quatro perfis, cadastro de usuários e filiais, listagem de usuários e cadastro de produtos comuns, medicamentos, dermocosméticos e equipamentos.

## Sobre o projeto

O Projeto Integrado Multidisciplinar II propõe uma aplicação capaz de integrar o estoque de `N` filiais de uma rede farmacêutica. Atualmente, cada unidade pode controlar seus produtos de maneira isolada, dificultando a consulta de itens disponíveis em outras lojas.

Com uma base de dados unificada, o sistema deverá permitir:

- consultar produtos e quantidades disponíveis por filial;
- localizar rapidamente uma unidade que possua determinado produto;
- controlar entradas e saídas do estoque;
- restringir operações conforme o perfil do usuário;
- exigir autorização de um farmacêutico para medicamentos de maior severidade;
- manter registros de operações e acessos relevantes.

## Objetivo

Disponibilizar uma visão integrada do estoque da rede farmacêutica, apoiando o atendimento ao cliente e tornando mais eficiente o gerenciamento dos produtos em cada filial.

## Escopo funcional

As funcionalidades previstas para o sistema são:

1. **Autenticação de usuários** — login e criação de contas conforme o perfil de acesso.
2. **Cadastro de filiais** — criação e manutenção das unidades da rede.
3. **Cadastro de produtos** — inclusão unitária ou importação de múltiplos produtos por arquivo CSV.
4. **Consulta de estoque** — listagem e filtragem de produtos por palavra-chave, filial ou rede.
5. **Manutenção de produtos** — edição e remoção de produtos por usuários autorizados.
6. **Movimentação de estoque** — registro de entradas e saídas de produtos.
7. **Controle de medicamentos** — tratamento diferenciado para medicamentos que exigem receita ou autorização profissional.
8. **Logs** — registro de operações para acompanhamento geral e por filial.

As funcionalidades acima representam o escopo planejado; sua disponibilidade dependerá da implementação de cada etapa do projeto.

## Perfis de usuário

| Perfil | Responsabilidade e permissões principais |
| --- | --- |
| **Diretor** | Administra a rede: cria filiais, gerencia usuários de qualquer unidade, acessa todo o estoque e consulta os logs da aplicação. Pode liberar medicamentos com ou sem receita. |
| **Gerente** | Administra uma filial: gerencia seus produtos e estoque, consulta o estoque das demais unidades e cria contas locais de farmacêuticos e atendentes. Pode liberar medicamentos com ou sem receita. |
| **Farmacêutico** | Consulta produtos e estoque da própria filial e pode liberar medicamentos que exigem receita; o CRF da conta deve estar cadastrado. |
| **Atendente** | Consulta produtos e registra saídas na própria filial. Para medicamento com receita, informa o CRF de um farmacêutico cadastrado na mesma filial. |

## Classificação de produtos

Os produtos deverão ser classificados por categoria e subcategoria:

- **Medicamentos:** genérico, similar, referência, controlado e MIP;
- **Dermocosméticos:** protetor solar, hidratante e tratamento facial;
- **Perfumaria:** perfume, colônia e body splash;
- **Higiene:** bucal, íntima e corporal;
- **Bebês:** fraldas, higiene infantil e alimentação infantil;
- **Suplementos:** vitaminas, minerais e esportivos;
- **Ortopedia:** joelheira, cinta e meia de compressão;
- **Equipamentos:** termômetro, esfigmomanômetro e oxímetro.

## Tecnologias e arquitetura

- **Linguagem:** C;
- **Execução prevista:** aplicação via terminal, sem interface gráfica;
- **Banco de dados:** MySQL, relacional;
- **Nome previsto do banco:** `pharma_database`;
- **Organização:** princípios de Clean Code e uma divisão inspirada em MVC, com responsabilidades separadas por domínio.

O diretório `backend` segue a separação entre configurações e regras da aplicação:

```text
.
├── main.c
├── backend/
│   ├── config/
│   │   ├── Makefile
│   │   ├── database_connector.c
│   │   └── database_connector.h
│   └── src/
│       ├── domain/
│       ├── infrastructure/
│       │   ├── external_services/
│       │   │   └── bcrypt/
│       │   └── repository/
│       ├── presentation/
│       ├── use_case/
│       ├── utils/
│       └── validation/
├── frontend/
│   └── screens/
├── database/
│   ├── database.md
│   └── tables.sql
└── documentation/
```

Responsabilidades das camadas:

- `config`: configurações da aplicação e da conexão com o banco;
- `domain`: entidades e regras de validação;
- `infrastructure/repository`: acesso direto e exclusivo ao banco de dados;
- `infrastructure/external_services`: integração com serviços ou bibliotecas externas;
- `presentation`: rotas, entrada das requisições e middleware de autenticação/autorização;
- `use_case`: execução das regras de negócio a partir de uma solicitação.

## Cadastro e autenticação de usuários — implementados

O primeiro uso continua começando pelo cadastro do diretor. Depois disso, o usuário pode entrar pela tela de login. O sistema consulta a view `vw_login_usuarios`, verifica a senha com bcrypt e direciona o usuário conforme o perfil: diretor, gerente, farmacêutico ou atendente. Diretor e gerente podem cadastrar funcionários; os dados do supervisor e da filial são associados à conta criada.

O fluxo implementado é:

```text
main.c
  └── check_first_use()
        ├── check_director_exists()       → consulta COUNT(*) no MySQL
        ├── nenhum diretor                 → cadastro inicial do diretor
        └── diretor existente              → tela de login
              └── valida credenciais e perfil → tela inicial correspondente

Cadastro de funcionários:
  diretor ou gerente → seleção de filial → cadastro de gerente,
                       farmacêutico ou atendente → INSERT no MySQL
```

### Funcionalidades já implementadas

- conexão com o banco MySQL `pharma_database`;
- verificação de primeiro uso por meio da quantidade de diretores cadastrados;
- tentativa de conexão novamente após cinco segundos quando o banco está indisponível;
- tela de cadastro em terminal com `ncurses`;
- validação do tamanho mínimo do terminal: 70 colunas por 24 linhas;
- formulário com nome, sobrenome, código interno, e-mail e senha;
- ocultação da senha durante a digitação;
- suporte a navegação entre os botões de cadastro e cancelamento;
- validação de campos obrigatórios;
- validação do código interno com seis caracteres;
- validação básica do formato do e-mail;
- geração de identificador alfanumérico de seis caracteres;
- geração de salt e hash da senha com bcrypt, usando custo 12;
- persistência do diretor na tabela `diretor`;
- mensagens de erro e repetição do formulário quando o cadastro falha;
- encerramento correto da conexão MySQL e da sessão `ncurses`;
- login dos quatro perfis, validação de senha bcrypt e identificação do usuário autenticado;
- alteração de senha no perfil com código de confirmação enviado por SMTP;
- geração de IDs sem colisão com IDs já existentes nas tabelas de usuários;
- cadastro de gerentes, farmacêuticos e atendentes por diretor ou gerente;
- associação dos funcionários à filial e ao supervisor responsável;
- cadastro e listagem de filiais, com endereço;
- listagem dos usuários cadastrados com nome, perfil e filial.

## Cadastro de produtos — implementado parcialmente

A tela de produtos permite cadastrar um produto comum e complementar seu cadastro com os atributos específicos de uma das três categorias implementadas. Os dados gerais do produto incluem nome, marca, descrição, preço, código de barras, quantidade e filial. O cadastro gera IDs e grava os dados nas tabelas relacionadas do MySQL.

Tipos com fluxo de cadastro implementado:

- **Produto comum:** dados gerais e vínculo com a filial;
- **Medicamento:** princípio ativo, forma farmacêutica, concentração, tarja, indicação de receita e tipagem;
- **Dermocosmético:** tipo de pele, volume, fator de proteção e indicação;
- **Equipamento:** modelo, fabricante, especificações e garantia.

As categorias perfumaria, higiene, bebês, suplementos e ortopedia continuam no planejamento. Cadastro por CSV, edição, remoção, consulta integrada e movimentação de estoque também não estão descritos como concluídos nesta versão.

### Arquivos envolvidos no cadastro

| Arquivo | Responsabilidade |
| --- | --- |
| [`main.c`](main.c) | Controla o início da aplicação, verifica o primeiro uso e direciona o fluxo para o cadastro inicial ou para o login futuro. |
| [`frontend/screens/create_account.c`](frontend/screens/create_account.c) | Implementa a interface de terminal do cadastro, leitura dos campos, máscara da senha, navegação e mensagens de erro. |
| [`frontend/screens/create_account.h`](frontend/screens/create_account.h) | Expõe `create_account_screen` para o ponto de entrada da aplicação. |
| [`backend/src/presentation/director_presentation.c`](backend/src/presentation/director_presentation.c) | Faz a ponte entre a tela e o caso de uso, encaminhando os dados para o cadastro. |
| [`backend/src/presentation/director_presentation.h`](backend/src/presentation/director_presentation.h) | Declara `register_new_director`. |
| [`backend/src/use_case/director_use_case.c`](backend/src/use_case/director_use_case.c) | Orquestra o caso de uso: valida os dados, gera o ID, criptografa a senha e solicita a persistência. |
| [`backend/src/use_case/director_use_case.h`](backend/src/use_case/director_use_case.h) | Declara `check_first_use` e `create_director`. |
| [`backend/src/domain/director_domain.c`](backend/src/domain/director_domain.c) | Contém as regras de domínio para campos obrigatórios, código interno e e-mail. |
| [`backend/src/domain/director_domain.h`](backend/src/domain/director_domain.h) | Declara as validações do domínio do diretor. |
| [`backend/src/infrastructure/repository/director_repository.c`](backend/src/infrastructure/repository/director_repository.c) | Consulta a existência de diretores e insere o novo diretor no MySQL. |
| [`backend/src/infrastructure/repository/director_repository.h`](backend/src/infrastructure/repository/director_repository.h) | Declara as operações de persistência do diretor. |
| [`backend/src/domain/validation/password_crypt.c`](backend/src/domain/validation/password_crypt.c) | Encapsula a geração e a verificação de hashes bcrypt. |
| [`backend/src/domain/validation/password_crypt.h`](backend/src/domain/validation/password_crypt.h) | Declara `password_hash` e `password_verify`. |
| [`backend/src/utils/generate_id.c`](backend/src/utils/generate_id.c) | Gera identificadores alfanuméricos de seis caracteres. |
| [`backend/src/utils/generate_id.h`](backend/src/utils/generate_id.h) | Declara `generate_id`. |
| [`backend/src/infrastructure/external_services/bcrypt/bcrypt.c`](backend/src/infrastructure/external_services/bcrypt/bcrypt.c) | Implementa a camada bcrypt utilizada para gerar salt, criar hash e verificar senhas. |
| [`backend/src/infrastructure/external_services/bcrypt/crypt_blowfish.c`](backend/src/infrastructure/external_services/bcrypt/crypt_blowfish.c) | Implementação interna do algoritmo Blowfish usada pelo bcrypt. |
| [`backend/src/infrastructure/external_services/bcrypt/*.h`](backend/src/infrastructure/external_services/bcrypt/bcrypt.h) | Declarações e constantes da implementação bcrypt/Blowfish. |

### Infraestrutura de suporte

| Arquivo | Responsabilidade |
| --- | --- |
| [`backend/config/database_connector.c`](backend/config/database_connector.c) | Inicializa o cliente MySQL e abre conexões com o banco. |
| [`backend/config/database_connector.h`](backend/config/database_connector.h) | Declara `connect_to_database` e disponibiliza o tipo `MYSQL`. |
| [`backend/src/infrastructure/repository/query_executor.c`](backend/src/infrastructure/repository/query_executor.c) | Executa consultas genéricas e imprime resultados ou linhas afetadas. |
| [`backend/src/infrastructure/repository/query_executor.h`](backend/src/infrastructure/repository/query_executor.h) | Declara `execute_query`. |

O repositório específico do diretor foi separado do executor genérico: `director_repository.c` conhece as consultas necessárias para verificar e inserir diretores, enquanto `query_executor.c` permanece como utilitário geral de consultas.

## Arquivos de código-fonte

Em C, cada arquivo `.c` contém a implementação de uma parte do sistema, enquanto cada arquivo `.h` apresenta a interface pública que pode ser utilizada por outros arquivos. Essa separação evita a duplicação de código e permite que os módulos dependam de contratos bem definidos. Os arquivos do fluxo de cadastro do diretor estão relacionados na seção [Cadastro do diretor — implementado](#cadastro-do-diretor--implementado).

## Por que usamos um Makefile?

O `Makefile` automatiza a compilação do projeto. Sem ele, seria necessário lembrar todos os arquivos `.c`, diretórios de headers, bibliotecas e opções de compilação sempre que o programa fosse construído.

Neste projeto, o Makefile centraliza:

- o compilador utilizado (`gcc`);
- as opções de aviso e depuração (`-Wall`, `-Wextra` e `-g`);
- o caminho dos headers do MySQL (`-I...`);
- o caminho e as bibliotecas necessárias na etapa de linkedição (`-L...`, `-lmysqlclient`, `-lncurses` e `-lcurl`);
- o nome do executável (`pharmaStock`);
- a lista de arquivos `.c` que formam o programa;
- a remoção do executável gerado.

O arquivo está em [`backend/config/Makefile`](backend/config/Makefile), mas seus caminhos são relativos à raiz do repositório. Por isso, os comandos abaixo devem ser executados a partir da raiz do projeto:

```bash
# Compila o programa
make -f backend/config/Makefile

# Remove o executável gerado
make -f backend/config/Makefile clean
```

### Como o Makefile funciona

As principais variáveis atuais são:

```make
CC      = gcc
CFLAGS  = -Wall -Wextra -g -I/opt/homebrew/opt/mysql-client/include
LDFLAGS = -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient -lncurses
TARGET  = pharmaStock
SRC     = main.c \
          backend/config/database_connector.c \
          backend/src/infrastructure/repository/query_executor.c \
          backend/src/use_case/director_use_case.c \
          backend/src/infrastructure/repository/director_repository.c \
          frontend/screens/create_account.c \
          backend/src/presentation/director_presentation.c \
          backend/src/domain/director_domain.c \
          backend/src/infrastructure/external_services/bcrypt/bcrypt.c \
          backend/src/infrastructure/external_services/bcrypt/crypt_blowfish.c \
          backend/src/utils/generate_id.c \
          backend/src/domain/validation/password_crypt.c
```

- `CC` define o compilador;
- `CFLAGS` define avisos, informações de depuração e caminhos de headers;
- `LDFLAGS` define caminhos e bibliotecas usadas na linkedição;
- `TARGET` define o nome do executável;
- `SRC` lista todos os arquivos `.c` que devem ser compilados e vinculados.

A regra `all` chama a regra principal, que executa uma linha equivalente a:

```bash
gcc -Wall -Wextra -g \
  -I/opt/homebrew/opt/mysql-client/include \
  main.c \
  backend/config/database_connector.c \
  backend/src/infrastructure/repository/query_executor.c \
  backend/src/use_case/director_use_case.c \
  backend/src/infrastructure/repository/director_repository.c \
  frontend/screens/create_account.c \
  backend/src/presentation/director_presentation.c \
  backend/src/domain/director_domain.c \
  backend/src/infrastructure/external_services/bcrypt/bcrypt.c \
  backend/src/infrastructure/external_services/bcrypt/crypt_blowfish.c \
  backend/src/utils/generate_id.c \
  backend/src/domain/validation/password_crypt.c \
  -o pharmaStock \
  -L/opt/homebrew/opt/mysql-client/lib \
  -lmysqlclient -lncurses -lcurl
```

O `-I` informa onde encontrar headers, como `mysql/mysql.h`. O `-L` informa onde encontrar as bibliotecas compiladas. Já `-lmysqlclient`, `-lncurses` e `-lcurl` vinculam o cliente MySQL, a biblioteca de terminal e o cliente HTTP/SMTP usados pelo programa.

## Regra para adicionar arquivos `.c`

Todo novo arquivo `.c` precisa ser adicionado à variável `SRC` do Makefile para participar da compilação. Colocar o arquivo dentro de `src` não é suficiente: o compilador só recebe os arquivos explicitamente listados em `SRC`.

O cadastro do diretor já está incluído no `SRC`. Para adicionar outro módulo, acrescente seu caminho à lista. Por exemplo:

```make
SRC = main.c \
      backend/config/database_connector.c \
      backend/src/infrastructure/repository/query_executor.c \
      backend/src/use_case/director_use_case.c \
      backend/src/novo_modulo/novo_modulo.c
```

Ao criar um novo módulo, siga este fluxo:

1. Crie o arquivo `.c` no diretório correspondente à responsabilidade do módulo.
2. Crie um `.h` quando as funções desse módulo precisarem ser utilizadas por outros arquivos.
3. Coloque no `.h` apenas as declarações públicas e inclua-o no `.c` com `#include "arquivo.h"`.
4. Adicione o caminho do `.c` à variável `SRC` do Makefile.
5. Compile a partir da raiz do projeto e corrija todos os avisos indicados por `-Wall` e `-Wextra`.

Não inclua um arquivo `.c` dentro de outro arquivo `.c`. O correto é incluir o `.h` e listar cada implementação `.c` uma única vez em `SRC`; caso contrário, podem ocorrer definições duplicadas durante a linkedição.

## Banco de dados

O banco utilizado é o MySQL, com o nome `pharma_database`. O script [`database/tables.sql`](database/tables.sql) contém a criação inicial do banco e das tabelas, enquanto [`database/database.md`](database/database.md) descreve a intenção das entidades.

O cadastro do diretor utiliza a tabela `diretor`, que possui os campos `id_diretor`, `nome_diretor`, `sobrenome_diretor`, `codigo_interno`, `email_diretor` e `senha_diretor`. O ID é gerado pela aplicação, o e-mail é único e a senha armazenada é o hash bcrypt, não o texto original.

No campo `medicamento.receita`, `0` significa que o medicamento exige receita e `1` que não exige. Medicamentos cadastrados pela versão anterior podem ter esse valor invertido pelo formulário antigo; revise esses registros existentes antes de usar a nova autorização de vendas.

Antes de executar o script, é necessário ter um servidor MySQL disponível e selecionar o banco correto. O esquema ainda está em desenvolvimento e pode exigir atualização ao longo da implementação.

As credenciais são carregadas de um arquivo `.env` pelo módulo `backend/config/env_parser.c`. Configure o arquivo localmente antes de executar a aplicação e não publique seus valores. Os nomes das variáveis esperadas devem ser conferidos em `backend/config/database_connector.c`.

Para enviar os códigos de confirmação da alteração de senha, configure também `SMTP_URL`, `SMTP_USERNAME`, `SMTP_PASSWORD` e `SMTP_FROM` no `.env`. Use o endereço do servidor SMTP com STARTTLS (por exemplo, `smtp://smtp.example.com:587`) ou TLS implícito (`smtps://smtp.example.com:465`), e informe em `SMTP_FROM` o endereço de remetente autorizado pelo provedor. Se o servidor usar uma autoridade certificadora privada, `SMTP_CA_BUNDLE` pode apontar para o arquivo PEM dessa CA. A tabela `recuperacao_senha` é criada por `database/tables.sql`; os códigos expiram após 10 minutos e são descartados após o uso.

## Modelo de dados e regras de negócio

O modelo prevê entidades para usuários, filiais, produtos e estoque. Os usuários são associados às filiais conforme seu perfil, enquanto os produtos possuem informações gerais e tabelas específicas para atributos próprios de cada categoria.

As regras de acesso são:

| Perfil | Responsabilidade e permissões principais |
| --- | --- |
| **Diretor** | Administra a rede: cria filiais, gerencia usuários de qualquer unidade, acessa todo o estoque e consulta os logs da aplicação. |
| **Gerente** | Administra uma filial: gerencia seus produtos e estoque, consulta o estoque das demais unidades e cria contas locais de farmacêuticos e atendentes. |
| **Farmacêutico** | Consulta os produtos, acessa o estoque local, registra baixas autorizadas e libera medicamentos que exigem sua responsabilidade profissional, mediante CRF. |
| **Atendente** | Consulta os produtos da rede e registra a saída de medicamentos comuns da própria filial. Operações de maior severidade dependem de autorização do farmacêutico. |

As categorias de produtos previstas são medicamentos, dermocosméticos, perfumaria, higiene, bebês, suplementos, ortopedia e equipamentos, cada uma com suas respectivas subcategorias.

## Documentação do projeto

- [Documento de estruturação do PIM II](documentation/Estruturação%20do%20PIM%20II%20.pdf)
- [Modelo de dados do banco](database/database.md)
- [Script SQL inicial](database/tables.sql)
- [Modelo da arquitetura](documentation/docs_explain_project/modelo_repositório.md)
- [Tipos de usuário](documentation/docs_explain_project/users_types.md)
- [Diagrama de caso de uso](documentation/UML_caso_de_uso.png)
- [Diagrama de classes](documentation/UML_diagrama_de_classes.png)
- [Diagrama entidade-relacionamento](documentation/der%20-%20pim%20II.brM3)

## Artefatos gerados

O comando `make` gera o executável `pharmaStock`. Dependendo da configuração do depurador, também pode ser criado o diretório `pharmaStock.dSYM`, que contém informações de depuração para sistemas macOS. Esses arquivos são artefatos de compilação, não código-fonte.

## Próximas etapas

- implementar edição e remoção de usuários, filiais e produtos;
- implementar cadastro para as demais categorias de produtos previstas;
- implementar consulta de estoque por produto e por filial;
- implementar movimentação de estoque e controles específicos para medicamentos;
- adicionar importação de produtos/estoque por CSV;
- trocar a montagem direta de SQL por consultas parametrizadas/prepared statements;
- finalizar e validar o modelo entidade-relacionamento;
- criar testes automatizados para validações, bcrypt, casos de uso e repositórios.


# Comandos para execução

- make -f backend/config/Makefile
- ./pharmaStock
