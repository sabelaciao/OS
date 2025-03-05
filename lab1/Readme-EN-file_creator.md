# File Creator

Program that allows to generate the expected entries of the **combine.c** program of the operating systems practice 1.


### Important
Modify the ``list`` array to generate different student lists in each output file.

**Examples of lists

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

### Compilation
```bash
gcc file_creator.c -o file_creator
```

### Execution
```bash
./file_creator <file_name>
```

Replace ````<filename>``` with the name of the output file.

### Output
A binary file that stores structures of type ````struct students```. 

> ***Importante***: you must generate at least two lists to test ```combine.c``` program. This program writes a binary file with a list of structures, thus, it is expected that you program  ```combine.c``` reads these structures.