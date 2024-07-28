# TP Sistemas Operativos 2024 1C: C-Comenta

#### Grupo: Sindicato del Software

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante se guardará en la carpeta `bin` del módulo.

```bash
cd tp-so-sds/<nombre_modulo>
make
```

## Ejecución

Para ejecutar el proyecto, primero se deben compilar los módulos como anteriormente fue indicado y luego, por cada módulo hacer lo siguiente:

```bash
cd tp-so-sds/<nombre_modulo>
./bin/<nombre_modulo> <nombre_config>
```

> **Observación**: En "nombre_config" hay que poner solo nombre de la configuración sin el ".config". Cada prueba tiene su archivo de configuración correspondiente.

