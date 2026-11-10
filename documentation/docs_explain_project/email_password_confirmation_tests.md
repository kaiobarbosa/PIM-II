# Revisao da confirmacao de alteracao de senha

Data: 10/11/2026

## Resultado da revisao

O executavel `pharmaStock` encontrado antes da revisao tinha data de 08/11/2026, enquanto os fontes do fluxo eram de 10/11/2026. Recompilei o aplicativo com `make -B -f backend/config/Makefile`; a compilacao terminou com codigo de saida 0. Isso torna um executavel antigo uma explicacao provavel para o teste que permitiu trocar a senha sem pedir o codigo. O fluxo atual chama a solicitacao do codigo quando `Nova senha` esta preenchida e rejeita a gravacao direta de uma senha no caso de uso.

O Makefile agora acompanha os proprios arquivos `.h` e o Makefile como dependencias, entao uma mudanca em header ou nas opcoes de compilacao tambem relinka o executavel com o `make` normal.

O envio para uma caixa externa nao pode ser confirmado com a configuracao atual: o `.env` contem somente variaveis do MySQL e o ambiente nao define variaveis SMTP. Sem `SMTP_URL`, `SMTP_USERNAME`, `SMTP_PASSWORD` e `SMTP_FROM`, o servico encerra o envio com erro e remove o codigo temporario. Isso explica por que nenhuma mensagem chegou, inclusive na pasta de spam. Para enviar por um provedor real, essas quatro variaveis precisam ser configuradas com credenciais validas.

## Testes executados

| Teste | Resultado |
| --- | --- |
| Compilacao completa com `make -B -f backend/config/Makefile` | Passou, sem erros ou avisos do compilador. |
| Caso de uso de perfil em `tests/profile_password_change_test.c` | Passaram seis verificacoes: atualizacao direta rejeita senha; edicao sem troca de senha continua funcionando; codigo invalido bloqueia; codigo valido atualiza o hash e consome o codigo; o destinatario e o email atual; falha de envio remove o codigo salvo. Dependencias de banco e SMTP foram substituidas por stubs. |
| Esquema MySQL em `tests/password_confirmation_schema_test.c` | Passou contra o banco configurado: a tabela `recuperacao_senha` tem as quatro colunas esperadas e a view `vw_login_usuarios` esta disponivel. |
| Repositorio MySQL em `tests/password_confirmation_repository_test.c` | Passou contra o banco configurado: insercao, validacao do codigo correto, rejeicao do codigo errado, expiracao forcada para 11 minutos e consumo de uso unico. A linha de teste e removida ao final. |
| SMTP sem configuracao em `tests/email_missing_smtp_config_test.c` | Passou: o servico falha fechado quando nao ha configuracao SMTP; nenhuma senha e alterada por esse caminho. |
| Transporte SMTP em `tests/email_smtp_transport_test.c` | Passou contra um servidor local de teste com STARTTLS e certificado de teste confiado por `SMTP_CA_BUNDLE`. A mensagem capturada continha destinatario, remetente, codigo e prazo de expiracao esperados. |

O teste SMTP local confirma a conversa entre libcurl e um servidor SMTP, mas nao confirma autenticacao ou entrega por um provedor externo. Para verificar a chegada na caixa de entrada, ainda e necessario configurar o `.env` com uma conta SMTP real e repetir a solicitacao pelo perfil.

## Comandos dos testes C

Executados a partir da raiz do projeto:

```sh
gcc -Wall -Wextra -I. tests/profile_password_change_test.c \
  backend/src/use_case/profile_use_case.c \
  backend/src/utils/generate_id.c \
  backend/src/utils/verify_user_informations.c \
  -o /tmp/pim_profile_password_change_test
/tmp/pim_profile_password_change_test

gcc -Wall -Wextra -I. -I/opt/homebrew/opt/mysql-client/include \
  tests/password_confirmation_schema_test.c \
  backend/config/database_connector.c backend/config/env_parser.c \
  -o /tmp/pim_password_confirmation_schema_test \
  -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient
/tmp/pim_password_confirmation_schema_test

gcc -Wall -Wextra -I. -I/opt/homebrew/opt/mysql-client/include \
  tests/password_confirmation_repository_test.c \
  backend/src/infrastructure/repository/password_confirmation_repository.c \
  backend/src/utils/generate_id.c \
  backend/config/database_connector.c backend/config/env_parser.c \
  -o /tmp/pim_password_confirmation_repository_test \
  -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient
/tmp/pim_password_confirmation_repository_test

gcc -Wall -Wextra -I. tests/email_missing_smtp_config_test.c \
  backend/src/infrastructure/external_services/bcrypt/email/email.c \
  backend/config/env_parser.c -o /tmp/pim_email_missing_smtp_config_test \
  -lcurl
/tmp/pim_email_missing_smtp_config_test

gcc -Wall -Wextra -I. tests/email_smtp_transport_test.c \
  backend/src/infrastructure/external_services/bcrypt/email/email.c \
  backend/config/env_parser.c -o /tmp/pim_email_smtp_transport_test -lcurl
```

O teste de configuracao ausente roda sem variaveis SMTP. O teste de transporte exige um servidor SMTP local: defina `PIM_SMTP_LOCAL_TEST=1`, `SMTP_URL` apontando para `localhost`, as outras variaveis SMTP com valores de teste e `SMTP_CA_BUNDLE` para o certificado local. O proprio teste muda para um diretorio isolado sem `.env`, para impedir que credenciais reais sejam usadas. `SMTP_CA_BUNDLE` mantem a verificacao do certificado TLS ativa.

