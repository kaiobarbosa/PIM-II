CC = gcc
CFLAGS = -Wall -Wextra -g -I. -I/opt/homebrew/opt/mysql-client/include
LDFLAGS = -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient -lncurses
TARGET = pharmaStock

SRC = main.c \
      backend/config/database_connector.c \
      backend/src/infrastructure/repository/query_executor.c \
      backend/src/use_case/director_use_case.c \
      backend/src/infrastructure/repository/director_repository.c\
      frontend/screens/create_account.c \
      backend/src/presentation/director_presentation.c\
      backend/src/domain/director_domain.c\
      backend/src/infrastructure/external_services/bcrypt/bcrypt.c\
      backend/src/infrastructure/external_services/bcrypt/crypt_blowfish.c\
      backend/src/utils/generate_id.c\
      backend/src/domain/validation/password_crypt.c\
      frontend/screens/login_account.c\
      backend/src/presentation/login_presentation.c\
      backend/src/use_case/login_use_case.c\
      backend/src/utils/verify_user_informations.c\
      backend/src/infrastructure/repository/login_repository.c\
      frontend/screens/home_screen.c\
      frontend/components/sidebar.c\
      frontend/screens/employees_screen.c\
      backend/src/presentation/manager_presentation.c\
      backend/src/use_case/manager_use_case.c\
      frontend/screens/branch_screen.c\
      backend/src/presentation/branch_presentation.c\
      backend/src/use_case/branch_use_case.c\
      backend/src/domain/branch_domain.c\
      backend/src/infrastructure/repository/branch_repository.c\
      backend/src/infrastructure/repository/manager_repository.c\
      backend/src/domain/manager_domain.c\

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
