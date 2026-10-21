# Variáveis (Configurações)
CC = gcc
CFLAGS = -Wall -Wextra -g -I/opt/homebrew/opt/mysql-client/include
LDFLAGS = -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient -lncurses
TARGET = pharmaStock

# Caminhos atualizados com a pasta "backend" e a exata grafia das suas pastas
SRC = main.c backend/config/database_connector.c backend/src/infrasctructure/repository/query_executor.c

# Regra principal
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Limpeza
clean:
	rm -f $(TARGET)
