# PIM II — Sistema de Estoque Farmacêutico

Sistema de consulta e gerenciamento de estoque para uma rede farmacêutica com múltiplas filiais. A proposta é centralizar as informações dos produtos e permitir que os usuários consultem a disponibilidade de um medicamento em toda a rede, inclusive quando ele não estiver disponível na filial atual.

> **Status:** em desenvolvimento e fase de modelagem.

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

Estrutura arquitetural prevista para o backend:

```text
backend/
├── config/
└── src/
    ├── domain/
    │   ├── entities/
    │   └── validation/
    ├── infrastructures/
    │   ├── external_services/
    │   └── repository/
    ├── presentation/
    │   ├── middlewares/
    │   └── routes/
    └── use_cases/
```

Responsabilidades principais:

- `config`: configurações da aplicação e da conexão com o banco;
- `domain`: entidades e regras de validação;
- `infrastructures/repository`: acesso direto e exclusivo ao banco de dados;
- `infrastructures/external_services`: integração com serviços ou bibliotecas externas;
- `presentation`: rotas, entrada das requisições e middleware de autenticação/autorização;
- `use_cases`: tratamento das requisições e execução das regras de negócio.

## Modelo de dados

O modelo prevê entidades para usuários, filiais, produtos e estoque. Os usuários são associados às filiais conforme seu perfil, enquanto os produtos possuem informações gerais e tabelas específicas para atributos próprios de cada categoria.

Entidades e tabelas inicialmente previstas:

- `directors` — diretores da rede;
- `managers` — gerentes associados a uma filial;
- `pharmacists` — farmacêuticos e seus registros profissionais;
- `attendants` — atendentes associados a uma filial;
- `branches` — filiais e seus endereços;
- `products` — dados gerais dos produtos;
- tabelas específicas de medicamentos, dermocosméticos, suplementos e equipamentos;
- `stock` — relação entre produtos, filiais e quantidades disponíveis;
- categorias e subcategorias de produtos.

As chaves estrangeiras deverão preservar os vínculos entre usuários, filiais, produtos, categorias e movimentações de estoque. O esquema ainda poderá ser refinado durante a implementação do banco e a validação do DER.

## Documentação do projeto

- [Documento de estruturação do PIM II](documentation/Estruturação%20do%20PIM%20II%20.pdf)
- [Modelo de dados do banco](database/database.md)
- [Diagrama entidade-relacionamento](documentation/der%20-%20pim%20II.brM3)

## Próximas etapas

- finalizar e validar o modelo entidade-relacionamento;
- definir o esquema SQL e as restrições de integridade;
- implementar a conexão com o MySQL;
- desenvolver autenticação e controle de permissões;
- implementar cadastro, consulta e movimentação de produtos;
- adicionar importação de estoque por CSV;
- criar testes e revisar a documentação conforme as funcionalidades forem concluídas.
