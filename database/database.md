# Database
   - o banco de dados será feito com mysql, sua escolha se deu pelo fato de ser um banco completo para os requisitos e backlog do projeto. 

## arquitetura das tabelas

   - No banco,  inicialmente teremos 6 tabelas: Administrador do sistema -> equipe de TI responsavel por adicionar filiais e sanar panes no sistema.
                                                Gerência -> Cargo maximo em uma filial. Responsavel pelo CRUD dos medicamentos e adicionar/autorizar novas contas de atendentes e farmaceuticos
                                                Farmaceutico -> Pode liberar medicamentos que necessitam de receita; seu CRF pode autorizar a venda de um atendente da mesma filial.
                                                Atendente -> Pode liberar medicamentos sem receita na própria filial; para medicamentos com receita, informa o CRF de um farmaceutico cadastrado na mesma filial.
                                                Gerente e diretor -> Podem liberar medicamentos com ou sem receita.
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
