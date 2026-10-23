CC = gcc
CFLAGS = -Wall -Wextra -g -I. -I/opt/homebrew/opt/mysql-client/include
LDFLAGS = -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient -lncurses
TARGET = pharmaStock

SRC = main.c \
      backend/config/database_connector.c \
      backend/src/infrasctructure/repository/query_executor.c \
      backend/src/use_case/director_use_case.c \
      backend/src/infrasctructure/repository/director_repository.c\
      frontend/screens/create_account.c \
      backend/src/presentation/director_presentation.c\
      backend/src/domain/director_domain.c\
      backend/src/infrasctructure/external_services/bcrypt/bcrypt.c\
      backend/src/infrasctructure/external_services/bcrypt/crypt_blowfish.c\
      backend/src/utils/generate_id.c
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
