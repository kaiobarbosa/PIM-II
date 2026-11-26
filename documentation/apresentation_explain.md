# Explicação do código para a apresentação

Este material acompanha o código atual do FarmaStock e pode servir como roteiro para explicar os quatro pontos da apresentação. O projeto é escrito em C: a interface chamada de “front” é a tela de terminal, e ela chama as funções do backend diretamente.

## 1. Envio do código para confirmar a troca de senha

O fluxo começa na tela de perfil, em [`profile_screen.c`](../frontend/screens/profile_screen.c). Quando a pessoa preenche uma nova senha e sua confirmação, a tela primeiro compara os dois valores. Se forem iguais, chama `request_profile_password_change_code_presentation`. A senha ainda não é alterada: essa etapa apenas pede e envia um código para o e-mail que já está cadastrado.

O caminho entre as camadas é:

```text
tela de perfil
  → presentation
  → use case (regras do processo)
  → repository (banco de dados)
  → serviço de e-mail SMTP
```

No caso de uso, em [`profile_use_case.c`](../backend/src/use_case/profile_use_case.c), o programa valida os dados do perfil e confere no banco se o e-mail informado pertence àquele usuário e perfil. Depois gera dois identificadores de seis caracteres: um ID para o pedido de recuperação e o código que será digitado pela pessoa. A função `generate_id`, em [`generate_id.c`](../backend/src/utils/generate_id.c), usa letras maiúsculas e números.

O repositório de confirmação, em [`password_confirmation_repository.c`](../backend/src/infrastructure/repository/password_confirmation_repository.c), inicia uma transação, remove códigos anteriores daquele e-mail e grava o novo código na tabela `recuperacao_senha`. A data e hora são preenchidas pelo banco. Isso faz com que, para cada e-mail, o pedido mais recente substitua o anterior.

Em seguida, `send_password_confirmation_email`, em [`email.c`](../backend/src/infrastructure/external_services/bcrypt/email/email.c), monta a mensagem e a envia usando libcurl como cliente SMTP. Os parâmetros de servidor, usuário, senha e remetente vêm do `.env` (`SMTP_URL`, `SMTP_USERNAME`, `SMTP_PASSWORD` e `SMTP_FROM`); `SMTP_CA_BUNDLE` é opcional. A conexão exige SSL/TLS. Se o envio falhar, o caso de uso apaga o código recém-gravado e informa erro à tela.

Depois que a pessoa digita o código recebido, a tela chama `confirm_profile_password_change_presentation`. O caso de uso valida o formato de seis caracteres (letras maiúsculas e números), confere novamente se o e-mail corresponde ao perfil e pede ao repositório que verifique se código e e-mail coincidem e se o registro foi criado nos últimos dez minutos. Se passar, a senha nova é transformada em hash bcrypt. O código é apagado para não ser reutilizado e, então, o hash é salvo no registro do usuário. A senha original não é guardada em texto puro.

**Uma forma simples de apresentar:** “Primeiro verificamos que o e-mail é o do perfil e enviamos um código temporário. O banco permite validar esse código por dez minutos. Só depois da confirmação a senha nova vira um hash bcrypt e é salva; o código é consumido para não poder ser usado novamente.”

**Detalhe de implementação:** o gerador atual usa `rand()` com semente baseada no horário e também é usado para IDs do sistema. Portanto, embora o código seja de seis caracteres, esse gerador não é um gerador criptográfico.

## 2. Por que algumas funções são `static`

Em C, `static` antes de uma função definida fora de outra função limita sua visibilidade ao arquivo `.c` em que ela foi escrita. Isso marca a função como auxiliar interna: outro arquivo não pode chamá-la diretamente, e ela não faz parte da interface pública do módulo. As funções que outros módulos precisam chamar ficam declaradas nos arquivos `.h` e são definidas sem esse `static`.

Exemplos no projeto:

- `validate_profile_info` e `valid_confirmation_code`, em [`profile_use_case.c`](../backend/src/use_case/profile_use_case.c), são verificações internas do caso de uso;
- `write_csv_field`, `make_timestamp` e `lock_file`, em [`csv_logger.c`](../backend/src/infrastructure/logging/csv_logger.c), são ferramentas usadas para gravar o CSV;
- `escape_value`, em [`password_confirmation_repository.c`](../backend/src/infrastructure/repository/password_confirmation_repository.c), só ajuda aquele repositório a preparar valores para consultas.

Assim, o módulo expõe funções com propósito de negócio — por exemplo, pedir o código ou registrar uma linha de log — e mantém seus passos auxiliares encapsulados. Isso também reduz conflitos de nomes entre arquivos. `static` não torna uma função mais rápida nem significa que ela execute uma única vez.

Há ainda outro uso de `static` para **variáveis**. `curl_initialized`, dentro de `send_password_confirmation_email`, preserva o valor entre chamadas à função; assim o código sabe se a inicialização global do libcurl já foi feita. `semente_iniciada`, declarada no escopo de arquivo em `generate_id.c`, mantém esse estado durante a execução e só pode ser acessada naquele arquivo. Ela registra se a semente do gerador já foi inicializada.

## 3. Por que usar `struct` para transportar dados

Uma `struct` reúne valores relacionados em um tipo com campos nomeados. Em vez de passar muitos argumentos soltos — ou tentar retornar vários valores de uma função — o código agrupa os dados em um único objeto. A função recebe um ponteiro para esse objeto e pode preenchê-lo; seu `int` de retorno informa o resultado da operação ou a quantidade de itens encontrados.

Por exemplo, `ProfileInfo`, definido em [`profile_repository.h`](../backend/src/infrastructure/repository/profile_repository.h), agrupa ID, perfil, nome, e-mail, filial e outros dados. A tela chama `get_profile_presentation(..., &profile)`. As camadas encaminham o ponteiro até o repositório, que preenche os campos; o retorno `1` indica que encontrou o perfil. De modo semelhante, `ProdutoItem` representa os dados resumidos de um produto, `BranchItem` representa uma filial e `UserInfoItem` representa um usuário.

Para listas, a interface aloca um vetor de structs e informa sua capacidade. Por exemplo, a tela passa `ProdutoItem sales[MAX_PROFILE_SALES]` e sua capacidade para buscar vendas. O repositório preenche os elementos do vetor e retorna quantos preencheu; `-1` indica falha. A tela então lê cada item por campos como `sales[i].nome` e `sales[i].quantidade`.

Esse formato deixa a comunicação entre as camadas explícita e organizada: cada função sabe quais campos espera preencher ou receber, e o compilador ajuda a verificar os tipos. **Neste projeto, isso não é uma resposta JSON de uma API web**: a tela de terminal e o backend são módulos C do mesmo programa e compartilham esses tipos definidos nos headers. O retorno estruturado é feito preenchendo as structs por ponteiro.

Também há uma struct `ApplicationLogEntry`, em [`csv_logger.h`](../backend/src/infrastructure/logging/csv_logger.h), que reúne os campos de uma ocorrência antes de ela ser gravada. Ela ilustra o mesmo benefício de agrupar informação relacionada, embora seja uma entrada para o logger, e não um dado que a tela recebe.

## 4. Como o arquivo CSV de logs é montado

As operações que devem ser auditadas montam uma `ApplicationLogEntry` e chamam `append_application_log`, implementada em [`csv_logger.c`](../backend/src/infrastructure/logging/csv_logger.c). Atualmente os casos de uso registram, entre outros eventos, criação de usuários e filiais, criação de produtos, alterações de estoque e vendas. A chamada acontece depois que a operação principal no banco foi concluída.

O logger segue estes passos:

1. Escolhe o caminho do arquivo: usa `APP_LOG_CSV` se essa variável estiver definida; caso contrário, usa `documentation/application_logs.csv`, relativo ao diretório de execução (normalmente a raiz do projeto).
2. Cria o horário atual no formato ISO 8601 com fuso horário.
3. Abre o arquivo para acrescentar dados e obtém um bloqueio de escrita. O bloqueio ajuda a impedir que gravações simultâneas misturem linhas.
4. Se o arquivo estiver vazio, escreve o cabeçalho. Depois escreve uma linha com horário, evento, entidade, ID e nome da entidade, autor, filial, CEP, quantidade e detalhes.
5. Coloca cada campo entre aspas e duplica aspas que já existam no conteúdo, para preservar valores que contenham vírgulas ou aspas. Ao fim, descarrega os dados, pede ao sistema operacional para sincronizá-los no disco e libera o bloqueio.

Por exemplo, ao criar uma filial o caso de uso registra o evento `branch_created`, o ID do usuário que fez o cadastro, o CEP e o endereço nos detalhes. Ao vender produto, registra `product_sale`, o produto, o funcionário, a filial e a quantidade. A lista de eventos está em [`application_logs.md`](application_logs.md). Senhas e hashes de senha não são campos do CSV.

Se a gravação do log falhar, o logger retorna erro e o caso de uso mostra um aviso; em geral, a operação que já foi concluída no banco não é desfeita por essa falha. O arquivo CSV complementa os registros de domínio existentes no banco — por exemplo, algumas operações de produto também criam registros nas tabelas de log do banco.

**Uma forma simples de apresentar:** “Depois de concluir uma operação importante, o caso de uso monta uma struct com os dados relevantes e entrega ao logger. O logger acrescenta uma linha padronizada ao CSV, criando o cabeçalho quando necessário e protegendo a escrita contra colisões.”
