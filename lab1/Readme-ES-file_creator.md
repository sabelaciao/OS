# File Creator

Programa que permite generar las entradas esperadas del programa **combine.c** de la práctica 1 de sistemas operativos.


### Importante
Modificar el arreglo ```lista``` para generar listas de alumnos diferentes en cada archivo de salida.

**Ejemplos de listas**

```c
struct alumnos lista[] = {
        *{"Juan Perez", 5, 1},
        {"Maria Lopez", 9, 2},
        {"Carlos Garcia", 7, 1},
        {"Ana Fernandez", 4, 3}
    };
```

```c
struct alumnos lista[] = {
        {"Pedro Perez", 8, 1},
        {"Carlos Lopez", 9, 2},
        {"Alfonos Garcia", 7, 1},
        {"Maribel Fernandez", 5, 3}
    };
```

### Compilación
```bash
gcc file_creator.c -o file_creator
```

### Ejecución
```bash
./file_creator <nombre_archivo>
```

Reemplazar ```<nombre_archivo>``` con el nombre del archivo de salida.

### Salida
Un archivo binario que guarda estructuras de tipo ```struct alumnos```. 

> ***Importante***: generar al menos dos listas para validar el funcionamiento del programa ```combine.c```. Dado que este programa escribe una estructura en los archivos de salida, se espera que ```combine.c``` pueda leer desde el archivo cada estructura almacenada.