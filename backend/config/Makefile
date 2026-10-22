# Variáveis (Configurações)
CC = gcc
# Adicionado -I. para que o compilador encontre as pastas backend/ e src/ a partir da raiz
CFLAGS = -Wall -Wextra -g -I. -I/opt/homebrew/opt/mysql-client/include
LDFLAGS = -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient -lncurses
TARGET = pharmaStock

# Caminhos atualizados com a pasta "backend"
SRC = main.c \
      backend/config/database_connector.c \
      backend/src/infrasctructure/repository/query_executor.c \
      backend/src/use_case/director_use_case.c \
      backend/src/infrasctructure/repository/director_repository.c

# Regra principal
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Limpeza
clean:
	rm -f $(TARGET)
