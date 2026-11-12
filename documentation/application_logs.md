# Logs da aplicação

As operações concluídas são registradas em `application_logs.csv`, em UTF-8 e no formato CSV compatível com RFC 4180. Campos entre aspas têm aspas internas duplicadas. O cabeçalho identifica o horário ISO 8601 com fuso, evento, entidade, usuário afetado, autor, filial, CEP, quantidade e detalhes.

O programa acrescenta cada linha ao arquivo e usa bloqueio de arquivo para serializar gravações concorrentes. O local padrão é `documentation/application_logs.csv`, relativo à raiz do projeto. `APP_LOG_CSV` pode apontar para outro arquivo, por exemplo para testes.

Eventos registrados:

- `user_created`: usuário novo, perfil, identificador de quem cadastrou e filial vinculada;
- `branch_created`: filial nova, autor, CEP e endereço informado;
- `product_created`: produto e estoque inicial;
- `stock_incremented` / `stock_decreased`: variação manual do estoque;
- `product_sale`: quantidade vendida, autor e filial.

Senhas e hashes de senha não são incluídos no arquivo.
