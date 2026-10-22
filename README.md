# PIM II — Sistema de Estoque Farmacêutico

Sistema de consulta e gerenciamento de estoque para uma rede farmacêutica com múltiplas filiais. A proposta é centralizar as informações dos produtos e permitir que os usuários consultem a disponibilidade de um medicamento em toda a rede, inclusive quando ele não estiver disponível na filial atual.

> **Status:** em desenvolvimento.

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
| **Diretor** | Administra a rede: cria filiais, gerencia usuários de qualquer unidade, acessa todo o estoque e consulta os logs da aplicação. |
| **Gerente** | Administra uma filial: gerencia seus produtos e estoque, consulta o estoque das demais unidades e cria contas locais de farmacêuticos e atendentes. |
| **Farmacêutico** | Consulta os produtos, acessa o estoque local, registra baixas autorizadas e libera medicamentos que exigem sua responsabilidade profissional, mediante CRF. |
| **Atendente** | Consulta os produtos da rede e registra a saída de medicamentos comuns da própria filial. Operações de maior severidade dependem de autorização do farmacêutico. |

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
│       ├── infrasctructure/
│       │   ├── external_services/
│       │   └── repository/
│       ├── presentation/
│       └── use_case/
├── database/
│   ├── database.md
│   └── tables.sql
└── documentation/
```

> O nome `infrasctructure` mantém a grafia do diretório existente no repositório.

Responsabilidades das camadas:

- `config`: configurações da aplicação e da conexão com o banco;
- `domain`: entidades e regras de validação;
- `infrasctructure/repository`: acesso direto e exclusivo ao banco de dados;
- `infrasctructure/external_services`: integração com serviços ou bibliotecas externas;
- `presentation`: rotas, entrada das requisições e middleware de autenticação/autorização;
- `use_case`: execução das regras de negócio a partir de uma solicitação.

## Arquivos de código-fonte

Em C, cada arquivo `.c` normalmente contém a implementação de uma parte do sistema, enquanto cada arquivo `.h` apresenta a interface pública que pode ser utilizada por outros arquivos. Essa separação evita a duplicação de código e permite que os módulos dependam de contratos bem definidos.

## Por que usamos um Makefile?

O `Makefile` automatiza a compilação do projeto. Sem ele, seria necessário lembrar todos os arquivos `.c`, diretórios de headers, bibliotecas e opções de compilação sempre que o programa fosse construído.

Neste projeto, o Makefile centraliza:

- o compilador utilizado (`gcc`);
- as opções de aviso e depuração (`-Wall`, `-Wextra` e `-g`);
- o caminho dos headers do MySQL (`-I...`);
- o caminho e as bibliotecas necessárias na etapa de linkedição (`-L...`, `-lmysqlclient` e `-lncurses`);
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
SRC     = main.c backend/config/database_connector.c backend/src/infrasctructure/repository/query_executor.c
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
  backend/src/infrasctructure/repository/query_executor.c \
  -o pharmaStock \
  -L/opt/homebrew/opt/mysql-client/lib \
  -lmysqlclient -lncurses
```

O `-I` informa onde encontrar headers, como `mysql/mysql.h`. O `-L` informa onde encontrar as bibliotecas compiladas. Já `-lmysqlclient` e `-lncurses` vinculam, respectivamente, o cliente MySQL e a biblioteca de terminal utilizada pelo programa.

## Regra para adicionar arquivos `.c`

Todo novo arquivo `.c` precisa ser adicionado à variável `SRC` do Makefile para participar da compilação. Colocar o arquivo dentro de `src` não é suficiente: o compilador só recebe os arquivos explicitamente listados em `SRC`.

Por exemplo, para incluir `backend/src/use_case/check_first_use.c`, a variável deve ficar semelhante a:

```make
SRC = main.c \
      backend/config/database_connector.c \
      backend/src/infrasctructure/repository/query_executor.c \
      backend/src/use_case/check_first_use.c
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

Antes de executar o script, é necessário ter um servidor MySQL disponível e selecionar o banco correto. O esquema ainda está em desenvolvimento e deverá ser revisado conforme as chaves estrangeiras e as tabelas de produtos forem finalizadas.

> **Atenção:** as credenciais de conexão atualmente estão diretamente em `database_connector.c`. Essa informação deve ser removida do código-fonte e substituída por variáveis de ambiente ou arquivo de configuração antes de qualquer uso real ou compartilhamento do projeto.

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

- finalizar e validar o modelo entidade-relacionamento;
- corrigir e completar o script SQL e suas dependências entre tabelas;
- definir a configuração segura das credenciais do MySQL;
- incluir os casos de uso no `SRC` conforme forem implementados;
- desenvolver autenticação e controle de permissões;
- implementar cadastro, consulta e movimentação de produtos;
- adicionar importação de estoque por CSV;
- criar testes e revisar a documentação conforme as funcionalidades forem concluídas.
