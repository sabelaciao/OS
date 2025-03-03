#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

struct alumnos{
    char nombre[50];
    int nota;
    int convocatoria;
};

int main(int argc, char ** argv) {
    if(argc != 2){
        perror("Usage: ./file_creator <filename>");
        return 1;
    }

    int fd = -1;
    int ret = -1;
    // struct alumnos lista[] = {
    //     {"Juan Perez", 5, 1},
    //     {"Maria Lopez", 9, 2},
    //     {"Carlos Garcia", 7, 3},
    //     {"Ana Fernandez", 4, 3}
    // };
    struct alumnos lista[] = {
        {"", 7, 5},
    };

    int num_alumnos = sizeof(lista) / sizeof(lista[0]);

    // Abrir archivo en modo escritura (crea o sobrescribe)
    fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Escribir los datos en el archivo
    for (int i = 0; i < num_alumnos; i++){
        if (write(fd, &lista[i], sizeof(struct alumnos)) == -1) {
            perror("Error al escribir en el archivo");
            close(fd);
            return 1;
        }
    }


    // Cerrar el archivo
    ret = close(fd);
    if(ret == -1) {
        perror("Error al cerrar el archivo");
        return 1;
    }

    printf("Datos guardados en '%s' correctamente.\n", argv[1]);
    return 0;
}
