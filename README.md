# NunezJ-Bash
Nombre: Jhonatan Núñez
# Intérprete Interactivo de Comandos en C

Este proyecto consiste en la implementación de un intérprete de comandos (tipo bash) para Linux, desarrollado en C utilizando exclusivamente llamadas al sistema (syscalls). El objetivo es comprender cómo funciona internamente un shell: cómo lee comandos, los interpreta, crea procesos, redirige la entrada y salida y ejecuta programas externos.

---

## Características principales
- Uso de **llamadas al sistema**:
  - `read`, `write`
  - `fork`, `execve`, `waitpid`
  - `open`, `close`, `dup2`
  - `chdir`, `getcwd`
  - `opendir`, `readdir`, `closedir`
  - `mkdir`, `unlink`, `rmdir`
- Soporte para **comandos internos (builtins)**:
  - `cd` – Cambia el directorio actual.
  - `pwd` – Muestra el directorio actual.
  - `ls` – Lista archivos y directorios del directorio actual.
  - `mkdir` – Crea un nuevo directorio.
  - `rm` – Elimina archivos o directorios vacíos.
- Ejecución de **comandos externos** (ej. `echo`, `cat`, `gcc`, `touch`, etc.).
- **Redirección de entrada y salida**:
  - `comando < archivo`
  - `comando > archivo`
- Ejecución de comandos en **background** con `&`.

---

## Compilación y ejecución

Desde una terminal en Linux:

```bash
gcc ShellProyecto.c -o shellProyecto
./shellProyecto
