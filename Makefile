# Compilador a utilizar
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -Wextra -std=c11

# Nombre del ejecutable
TARGET = shellProyecto

# Archivo fuente
SRC = ShellProyecto.c

# Regla por defecto: compilar el programa
all: $(TARGET)

# Cómo construir el ejecutable a partir del .c
$(TARGET): $(SRC)
  $(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Regla para limpiar: borrar el ejecutable
clean:
  rm -f $(TARGET)
