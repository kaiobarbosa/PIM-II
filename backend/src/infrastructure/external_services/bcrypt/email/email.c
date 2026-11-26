#include "backend/src/infrastructure/external_services/bcrypt/email/email.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend/config/env_parser.h"

typedef struct {
    const char *message;
    size_t length;
    size_t offset;
} EmailUpload;

static int has_supported_smtp_scheme(const char *url) {
    const char *host;

    if (url == NULL) return 0;
    if (strncmp(url, "smtp://", 7) == 0) {
        host = url + 7;
    } else if (strncmp(url, "smtps://", 8) == 0) {
        host = url + 8;
    } else {
        return 0;
    }

    return host[0] != '\0' && host[0] != '/' && host[0] != '?' &&
           host[0] != '#';
}

static size_t read_email_message(char *buffer, size_t size, size_t count,
                                 void *user_data) {
    EmailUpload *upload = (EmailUpload *)user_data;
    size_t capacity = size * count;
    size_t remaining = upload->length - upload->offset;
    size_t bytes_to_copy = remaining < capacity ? remaining : capacity;

    if (bytes_to_copy > 0) {
        memcpy(buffer, upload->message + upload->offset, bytes_to_copy);
        upload->offset += bytes_to_copy;
    }
    return bytes_to_copy;
}

int send_password_confirmation_email(const char *recipient,
                                     const char *verification_code) {
    const char *smtp_url;
    const char *smtp_username;
    const char *smtp_password;
    const char *sender;
    const char *smtp_ca_bundle;
    CURL *curl = NULL;
    struct curl_slist *recipients = NULL;
    char message[1024];
    int message_length;
    int success = 0;
    static int curl_initialized = 0;
    EmailUpload upload;
    CURLcode result;

    if (recipient == NULL || verification_code == NULL ||
        strchr(recipient, '\r') != NULL || strchr(recipient, '\n') != NULL ||
        strlen(verification_code) != 6) {
        return 0;
    }

    load_env_file(".env");
    smtp_url = getenv("SMTP_URL");
    smtp_username = getenv("SMTP_USERNAME");
    smtp_password = getenv("SMTP_PASSWORD");
    sender = getenv("SMTP_FROM");
    smtp_ca_bundle = getenv("SMTP_CA_BUNDLE");
    if (smtp_url == NULL || smtp_url[0] == '\0' ||
        smtp_username == NULL || smtp_username[0] == '\0' ||
        smtp_password == NULL || smtp_password[0] == '\0' ||
        sender == NULL || sender[0] == '\0' ||
        strchr(sender, '\r') != NULL || strchr(sender, '\n') != NULL) {
        fprintf(stderr, "Configuracao SMTP incompleta:");
        if (smtp_url == NULL || smtp_url[0] == '\0')
            fprintf(stderr, " SMTP_URL");
        if (smtp_username == NULL || smtp_username[0] == '\0')
            fprintf(stderr, " SMTP_USERNAME");
        if (smtp_password == NULL || smtp_password[0] == '\0')
            fprintf(stderr, " SMTP_PASSWORD");
        if (sender == NULL || sender[0] == '\0' ||
            strchr(sender, '\r') != NULL || strchr(sender, '\n') != NULL)
            fprintf(stderr, " SMTP_FROM");
        fprintf(stderr, ". Confira os nomes e valores no .env.\n");
        return 0;
    }

    if (!has_supported_smtp_scheme(smtp_url)) {
        fprintf(stderr,
                "SMTP_URL invalida: use smtp://servidor:587 para STARTTLS "
                "ou smtps://servidor:465 para TLS implicito, conforme o "
                "provedor.\n");
        return 0;
    }

    message_length = snprintf(
        message, sizeof(message),
        "To: %s\r\n"
        "From: %s\r\n"
        "Subject: Confirmacao de alteracao de senha - FarmaStock\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "\r\n"
        "Foi solicitada uma alteracao de senha para sua conta FarmaStock.\r\n"
        "Seu codigo de confirmacao e: %s\r\n"
        "O codigo expira em 10 minutos. Se voce nao solicitou esta alteracao, "
        "ignore esta mensagem.\r\n",
        recipient, sender, verification_code);
    if (message_length < 0 || (size_t)message_length >= sizeof(message)) {
        return 0;
    }

    upload.message = message;
    upload.length = (size_t)message_length;
    upload.offset = 0;

    if (!curl_initialized) {
        result = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (result != CURLE_OK) {
            fprintf(stderr, "Falha ao inicializar o cliente SMTP: %s\n",
                    curl_easy_strerror(result));
            return 0;
        }
        curl_initialized = 1;
    }

    curl = curl_easy_init();
    if (curl == NULL) {
        return 0;
    }

    recipients = curl_slist_append(recipients, recipient);
    if (recipients == NULL) {
        curl_easy_cleanup(curl);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, smtp_url);
    curl_easy_setopt(curl, CURLOPT_USERNAME, smtp_username);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, smtp_password);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    if (smtp_ca_bundle != NULL && smtp_ca_bundle[0] != '\0') {
        curl_easy_setopt(curl, CURLOPT_CAINFO, smtp_ca_bundle);
    }
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sender);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_email_message);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    result = curl_easy_perform(curl);
    if (result == CURLE_OK) {
        success = 1;
    } else if (result == CURLE_OPERATION_TIMEDOUT) {
        fprintf(stderr,
                "Timeout no envio SMTP. Ter acesso a internet nao garante "
                "acesso a porta SMTP; confira o servidor e a porta em "
                "SMTP_URL, e se a rede ou o provedor libera essa conexao.\n");
        fprintf(stderr, "Detalhe libcurl: %s\n", curl_easy_strerror(result));
    } else if (result == CURLE_COULDNT_RESOLVE_HOST) {
        fprintf(stderr,
                "Nao foi possivel localizar o servidor SMTP indicado em "
                "SMTP_URL. Confira o nome do host.\n");
        fprintf(stderr, "Detalhe libcurl: %s\n", curl_easy_strerror(result));
    } else if (result == CURLE_COULDNT_CONNECT) {
        fprintf(stderr,
                "Nao foi possivel conectar ao servidor SMTP. Confira a "
                "porta e o acesso de saida da rede.\n");
        fprintf(stderr, "Detalhe libcurl: %s\n", curl_easy_strerror(result));
    } else if (result == CURLE_LOGIN_DENIED) {
        fprintf(stderr,
                "O servidor SMTP recusou a autenticacao. Confira "
                "SMTP_USERNAME e SMTP_PASSWORD.\n");
        fprintf(stderr, "Detalhe libcurl: %s\n", curl_easy_strerror(result));
    } else {
        fprintf(stderr, "Falha no envio do e-mail de confirmacao.\n");
        fprintf(stderr, "Detalhe libcurl: %s\n", curl_easy_strerror(result));
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return success;
}
