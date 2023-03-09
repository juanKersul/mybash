## Introducción:
Elaboración de un shell, llamado "Mybash". 

El cúal es creado a partir de tres módulos principales:
- Parser
- Execute
- Builtin

## Modularización: 

### Descripición de módulos

* **execute.c:** Es el encargado de ejecutar los pipelines y comandos simples.
* **builtin.c:** Es el encargado de ejecutar los comandos internos, tanto `cd` cómo `exit`, y brinda varias funciones para detectar comandos internos. Hace uso de las bibliotecas `string.h`, `assert.h`, `stdlib.h`, `stdio.h` y `unistd.h`. No posee funciones privadas. Una decisión de diseño en éste, es el funcionamiento del comando `cd` al ejecutarlo sin argumentos, te lleva al `HOME` del sistema

* **command.c:** Es el encargado de brindar funcionalidad para el TAD `pipeline` y el TAD `scommand`. Se elaboró todo por separado y luego se juntó para probar su funcionalidad. Luego de haber pasado los test se pasó a eliminar leaks. Se trató, en lo posible, de no usar precondiciones o postcondiciones que impliquen llamadas a funciones del mismo TAD, también se evitó retornar información que rompa la encapsulación del TAD. Hace uso de las bibliotecas `stdio.h`, `glib.h`, `assert.h`, `string.h`, `strextra.h`.

* **prompt.c:** Es el encargado de mostrar el prompt de nuestra shell. Sólo posee la función `show_prompt`. Hace uso de `stdlib.h`,`string.h`,`assert.h` y `strextra.h`.

* **strextra.c:** Este archivo es de suma importancia para que funcione `command.c`, más en específico para las funciones `scommand_to_string` y `pipeline_to_string` a la hora de concatenar strings y no tener pérdida de memoria. Posee dos funciones públicas `strmerge` y `merge_and_free`, la útlima hace uso de la primera para unir strings y además liberar la memoria que haga falta. No posee funciones privadas. Hace uso de las bibliotecas `stdlib.h`,`string.h` y `assert.h`.

### Bibliotecas usadas:

- ```glib.h```

- ```assert.h```

- ```string.h```

- ```unistd.h ```

### Estilo de código:

Para pedir memoria fue utilizada la función ```calloc()```, se tomó la decisión de apuntar a ```NULL``` todo puntero liberado.

Las variables son todas en formato snake_case.

Las funciones decidimos que las llaves van pegadas al paréntesis de los argumentos, de ésta forma:

```C
void function(void){
    .
    .
}
```



## Herramientas de Programación: 

### Compilación

1. Compilar todo.

> make all

2. Ejecución de mybash

> ./mybash

3. Tests generales para mybash.

> make test

4. Test de memoria para mybash.

> valgrind --leak-check=full  ./mybash

5. Test del módulo de comandos.

> make test-command

6. Test de memoria para el módulo de comandos. 

> make memtest

7. Para borrar los archivos generados por la compilación.

> make clean

### Herramientas de desarrollo

Se utilizó Visual Studio Code para su desarrollo.

Para debugging ver [`gdb`](https://drive.google.com/file/d/1XSperdoS3aue-omF06fgnaS7cFHHhNmR/view).


### Comandos probados que funcionan
``` Bash
* cd ../..
* sleep 10 | echo "hola"
* ls > output 
* ls | wc -l
* ls -l > tmp < out
* evince -f file.pdf
* wc -l > out.txt < in.txt
* ls -l | sort -n +4 > tmp
* ls -l > mifile
* sort -n +4 < mifile
* evince -f file.pdf &
* ls -l | grep -i glibc &
* ls | grep u | wc
* xeyes &
```

### Comandos probados que no funcionan
```Bash
* ls > out | wc
* cd | ls
```

### Notas

En el caso de:

* `ls > out | wc `

A diferencia de bash, en ves de cortar el pipeline creará el archivo pero no volcará información en él y ejecutará wc del directorio actual.

En el caso de: 

* `cd | ls` 

El modulo de builtins no esta implementado con pipes por lo que sólo hará el `cd`.

Por otro lado al usar VSCode hace falta incluír ésto en el archivo `c_cpp_properties.json` la ruta para la librería glib.

```JSON
"includePath": [
    "${workspaceFolder}/**",
    "/usr/include/glib-2.0",
    "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
]
```


## Conclusiones.

Se obtuvo un nuevo shell que cumple con las siguientes funcionalidades:
    
* Ejecuta comandos simples con sus respectivos parámetros en modo foreground y
background.
* Soporta redirección de entrada y salida de los comandos.
* Soporta el uso de pipes `|` (hasta n).
* Es robusto ante entradas incompletas y/o inválidas.
* Es posible terminar la ejecución del shell utilizando `Ctrl + D`.
* Posee un prompt con el path relativo.

## Leaks.

Existen Memory Leaks, causados por la librería `glib.h`.

Son aproximadamente 18,612 / 18,753 bytes, (still reachable).
