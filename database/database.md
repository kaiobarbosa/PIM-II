# Database
   - o banco de dados será feito com mysql, sua escolha se deu pelo fato de ser um banco completo para os requisitos e backlog do projeto. 

## arquitetura das tabelas

   - No banco,  inicialmente teremos 6 tabelas: Administrador do sistema -> equipe de TI responsavel por adicionar filiais e sanar panes no sistema.
                                                Gerência -> Cargo maximo em uma filial. Responsavel pelo CRUD dos medicamentos e adicionar/autorizar novas contas de atendentes e farmaceuticos
                                                Farmaceutico -> Cargo responsável por autorizar a liberação de medicamentos controlados
                                                Atendente -> Cargo base da hierarquia... terá autorização de liberar medimentos comuns, dando baixa deles no estoque.
                                                Filial -> Entidade no qual engloba todos os cargos, exceto os administradores, citados anteriormente.
                                                Estoque -> Unico na rede inteiro, englobando todas as filiais e seus medicamentos.

    - SGDB -> MySQL
    - Database name -> pharma_database
    - Table's name -> adm -> administrators
                      gerência -> managers
                      farmaceuticos -> pharmaceuts
                      atendente -> attendant
                      filiais -> branches
                      estoque -> stock