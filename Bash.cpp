#include <unistd.h> //Para poder utilizar write, read
#include <string.h> //Para poder utilizar funciones que lean los caracteres que tiene un texto (strlen, strlock)
#include <stdlib.h> //exit
#include <dirent.h> //Para poder utilizar opendir, readdir, closedir
#include <sys/stat.h> //Para poder utilizar mkdir
#include <sys/types.h> //Para poder utilizar pid_t
#include <sys/wait.h> //Para poder utilizar waitpid
#include <fcntl.h>  //Para poder utilizar open

#define MAX_LINE 1024
#define MAX_ARGS 64
extern char* environ[];

//Funcion para saber si el comando que se escribe contiene un '/'
int contains_slash(const char *s){
    for(; *s; s++){
        if(*s == '/'){
            return 1; //Si contiene un '/'
        }
    }

    return 0; //No contiene un '/'
}

//Funcion para llamar a imprimir
void print(int fd, const char *s){
    if(s){
        write(fd, s,strlen(s));
    }
}

//Funcion para llamar a imprimir con salto de linea
void println(int fd, const char *s){
    if(s){
        write(fd, s,strlen(s));
    }

    write(fd, "\n", 1);
}

//Funcion para usar cd y entrar a un directorio o archivo
int cmd_cd(int argc, char* argv[]){
    //cd sin ruta
    if(argc < 2){
        println(STDERR_FILENO, "cd: Falta la ruta");
        return -1;
    }

    //intentar cambiar de directorio
    if(chdir(argv[1]) < 0){
        print(STDERR_FILENO, "cd: no puedo ir a ");
        println(STDERR_FILENO, argv[1]);
        return -1;
    }

    return 0;
}

//Funcion para usar pwd y saber el directorio actual
int cmd_pwd(void){
    char buf[MAX_LINE];

    if(getcwd(buf, sizeof(buf)) == NULL){
        println(STDERR_FILENO, "pwd: error al obtener directorio");
        return -1;
    }

    println(STDOUT_FILENO, buf);
    return 0;
}

//Funcion para usar ls y mostrar el contenido actual
int cmd_ls(void){
    DIR *d = opendir(".");
    if(d == NULL){
        println(STDERR_FILENO, "ls: no se puede abrir .");
        return -1;
    }

    struct dirent *ent;
    while((ent = readdir(d)) != NULL){
        println(STDOUT_FILENO, ent->d_name);
    }

    closedir(d);
    return 0;
}

//Funcion para usar mkdir y crear un directorio
int cmd_mkdir(int argc, char* argv[]){
    if(argc < 2){
        println(STDERR_FILENO, "mkdir: falta el nombre del directorio");
        return -1;
    }

    if(mkdir(argv[1], 0777) < 0){
        print(STDERR_FILENO, "mkdir: error al crear el directorio");
        println(STDERR_FILENO, argv[1]);
        return -1;
    }

    return 0;
}

//Funcion para usar rm y eliminar un archivo o directorio
int cmd_rm(int argc, char* argv[]){
    if(argc < 2){
        println(STDERR_FILENO, "rm: falta el nombre del archivo o directorio");
        return -1;
    }

    if(unlink(argv[1]) < 0){ //Aqui primero se intenta borrar como archivo
        if(rmdir(argv[1]) < 0){  //Aqui segundo lo intenta borrar como directorio si esta vacio
            print(STDERR_FILENO, "rm: no se pudo borrar el archivo o directorio");
            println(STDERR_FILENO, argv[1]);
            return -1;
        }
    }

    return 0;
}

//Funcion para decidir que funcion llamar segun el comando interno a utilizar
int run_builtin(int argc, char* argv[]){
    if(argc == 0){  //Si no escribio nada
        return 0;
    }

    if(strcmp(argv[0], "cd") == 0){  //Si se llama a cd
        cmd_cd(argc, argv);
        return 1;
    }

    if(strcmp(argv[0], "pwd") == 0){  //Si se llama a pwd
        cmd_pwd();
        return 1;
    }

    if(strcmp(argv[0], "ls") == 0){  //Si se llama a ls
        cmd_ls();
        return 1;
    }

    if(strcmp(argv[0], "mkdir") == 0){  //Si se llama a mkdir
        cmd_mkdir(argc, argv);
        return 1;
    }

    if(strcmp(argv[0], "rm") == 0){  //Si se llama a rm
        cmd_rm(argc, argv);
        return 1;
    }

    return 0;  //No se encuentra el comando
}

//Funcion para ejecutar comandos externos
int run_external(int argc, char* argv[], int bg, char *infile, char *outfile){
    pid_t pid = fork();

    if(pid < 0){  //Si existe un error al crear el hijo
        println(STDERR_FILENO, "error: no se pudo crear el hijo");
        return -1;
    }

    if(pid == 0){  //Si se encuentra en el proceso hijo
        char path[256];

        //Redirección de entrada si hay '<'
        if(infile != NULL){
            int fdIn = open(infile, O_RDONLY);  //Abre el archivo para leer
            if(fdIn < 0){  //No encuentra el archivo
                print(STDERR_FILENO, "no puedo abrir la entrada ");
                println(STDERR_FILENO, infile);
                _exit(1);
            }

            dup2(fdIn, STDIN_FILENO);  //Ahora leera del archivo
            close(fdIn);
        }

        //Redirección de entrada si hay '>'
        if(outfile != NULL){
            int fdOut = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);  //Abre el archivo para escribir
            if(fdOut < 0){
                print(STDERR_FILENO, "no puedo abrir la salida ");
                println(STDERR_FILENO, outfile);
                _exit(1);
            }

            dup2(fdOut, STDOUT_FILENO);  //Ahora escribira en el archivo
            close(fdOut);
        }

        if(contains_slash(argv[0])){  //Cuando si contiene un '/'
            execve(argv[0], argv, environ);
        }else{  //Cuando no contiene un '/'
            strcpy(path, "/bin/");  //probar con /bin/comando
            strcat(path, argv[0]);
            execve(path, argv, environ);

            strcpy(path, "/usr/bin/"); //probar con /usr/bin/comando
            strcat(path, argv[0]);
            execve(path, argv, environ);
        }

        //Cuando ningun execve funciono se manda un mensaje
        print(STDERR_FILENO, "comando no encontrado: ");
        println(STDERR_FILENO, argv[0]);

        _exit(1); // sale del proceso hijo
    }else{  //Si se encuentra en el proceso padre
        if(bg == 0){  //Si no se realiza en background
            int status;
            if(waitpid(pid, &status, 0) < 0){
                println(STDERR_FILENO, "error: no funciono waitpid");
                return -1;
            }
        }else{  //Si se realiza en background
            println(STDOUT_FILENO, "[proceso en background]");
        }

        return 0;
    }
}

int main(void){

    char line[MAX_LINE]; //Se guarda lo que el usuario escriba
    char *argv[MAX_ARGS];  //Se guarda las palabras que se escriban

    while(1){

        write(STDOUT_FILENO, "Bash>", 5);  //Llamada write para el prompt del shell

        ssize_t n = read(STDIN_FILENO, line, MAX_LINE - 1); //Llamada a read para leer texto del teclado

        //Si el usuario hace Ctrl + D o hay un error sale del shell
        if(n <= 0){
            write(STDOUT_FILENO, "\n", 1);
            break;
        }

        line[n] = '\0';

        //If para no contar al enter como un caracter
        if(n > 0 && line[n - 1] == '\n'){
            line[n - 1] = '\0';
        }

        int argc = 0; //contador de palabras
        int bg = 0;  //Int para poder identificar si se realizar en background
        char *infile = NULL;  //Arreglo que almacena los caracteres despues de '<'
        char *outfile = NULL;  //Arreglo que almacena los caracteres despues de '>'
        char *tok = strtok(line, " \t"); //Puntero para tokenizar las palabras

        while(tok != NULL && argc < MAX_ARGS - 1){
            if(strcmp(tok, "&") == 0){
                bg = 1;  //El comando se ejecutara en background
            }else if(strcmp(tok, "<") == 0){
                tok = strtok(NULL, " \t");
                if(tok != NULL){
                    infile = tok;  //Guarda el nombre del archivo
                }
            }else if(strcmp(tok, ">") == 0){
                tok = strtok(NULL, " \t");
                if(tok != NULL){
                    outfile = tok;  //Guarda el nombre del archivo de salido
                }
            }else{
                // 🔧 CORRECCIÓN: primero guardar, luego incrementar argc
                argv[argc] = tok;  //Para cuando es una palabra normal de comando
                argc++;
            }

            tok = strtok(NULL, " \t");
        }

        argv[argc] = NULL; //Poner un NULL para identificar el final del comando

        if(argc == 0){ //Cuando el usuario solo aplasta ENTER
            continue;
        }

        if(run_builtin(argc, argv)){
            continue; //Aqui se acepta cuando es un comando de los que se implemento
        }

        run_external(argc, argv, bg, infile, outfile);
    }

    return 0;

}