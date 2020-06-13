# Servidor Turbo

## Construido con

* [C](https://en.wikipedia.org/wiki/C_%28programming_language%29)

## Developers

* **Javier Lougedo** - [JLougedo](https://github.com/JLougedo)
* **Arturo Morcillo** - [artiimor](https://github.com/artiimor)

## Introducción
Práctica 1 de la asignatura de Redes de Comunicaciones 2 de los alumnos Arturo Morcillo y Javier Lougedo, correspondiente a la creación de un Servidor Web sencillo en C. Hemos denominado a todos nuestos proyectos "Turbo" para tener una cierta linealidad y punto en común para todos los proyectos, por lo que este tiene el sobrenombre de Servidor Turbo.
Esta práctica la realizamos inicialmente en la universidad. Al tener ciertos problemas por estilos de programación conflictivos y ver que inicialmente nos costó organizarnos para trabajar solapándonos en esta práctica, decidimos comenzar a trabajar juntos en lo que sería el boceto inicial del servidor, para tomar las principales y más importantes decisiones de diseño en conjunto y sintonía, y que Arturo se encargase de los detalles finales, mientras que yo, Javier, me encargaba de lo referente a las pruebas y la documentación, ya que tuve problemas con la subida de ficheros a GitEPS (no me dejaba trabajar más allá que en el editor integrado online). Es una organización que ha dado sus frutos, ya que de esta forma hemos conseguido no pisarnos entre nosotros a la hora de programar, ni depender de los conocimientos o de un módulo que haya realizado el otro, sino que hemos podido funcionar en buena sintonía y trabajando adecuadamente, informándonos de los principales cambios producidos y de los problemas más graves que presentaba el servidor, para ir tomando soluciones entre los dos. Además, al haber dispuesto de un tiempo tan extenso para realizar las prácticas, no hemos necesitado trabajar de forma simultanea y solapada en ningún momento.
En las secciones de la WiKi a continuación, procuraremos comentar los detalles más relevantes de nuestro servidor y las decisiones de diseño más importantes tomadas, los distintos tests realizados y los resultados obtenidos a partir de los mismos. La WiKi comprende las siguientes secciones:

Funcionamiento, donde explicamos como funciona la práctica e incluimos un pequeño video demostrativo.
Decisiones de diseño.
Código, con una breve descripción del mismo.
Troubleshooting, donde explicamos los problemas más propensos de ocurrir y como solucionarlos.
Conclusiones.


## Fucionamiento
Para arrancar el servidor, todo lo que tenemos que hacer es realizar make en el directorio objetivo. Inicialmente presentará una serie de warnings, debido al uso de pedantic, pero ninguno representa un problema grave y son pequeños errores esperados. Tras esto, con podemos optar por la vía automatizada, lanzando make run, que lanzará un servidor en el puerto 1200, con un tamaño de cola de 5. Si queremos optar por la vía manual, deberemos escribir el comando siguiente:
./turbo-server <nºpuerto> <tamaño_cola>
con lo que abriremos un servidor en el puerto determinado y con el tamaño de cola indicado. Podremos acceder al mismo a través del navegador, introduciendo en la barra de navegación "localhost:<nºpuerto>". En nuestro caso, hemos facilitado una herramienta de prueba donde testear el funcionamiento de prácticamente todo lo que podría ser necesario transmitir a través del servidor, con lo que no deberían quedar lagunas. Podemos observar, sin ir más lejos, en el video a continuación todo el proceso y el funcionamiento del sistema.
En el video a continuación podemos observar un ejemplo de funcionamiento.

Por último, cuando querramos cerrar el servidor bastará con realizar make kill, que ejecutará el comando "killall turbo-server", de forma que mataremos todos los procesos vinculados a turbo server. Si quisiesemos matarlo de uno en uno, bastaría con hacer "ps aux | grep turbo-server", con lo que listaremos todos los procesos vinculados a turbo-server y podremos escoger cual de ellos finalizar, con "kill ".

Decisiones de diseño
En cuanto a las decisiones de diseño, no nos hemos visto obligados a tomar prácticamente ninguna, de gran importancia. Hemos seguido las indicaciones y consejos que se nos daban tanto en la práctica como en clase en su momento, y hemos optado por el diseño de hilos, ya que de es la forma que consideramos más interesante y eficiente. El funcionamiento que sigue es sencillo, y el observado en teoría: se recoge la petición de la cola, devolviendo un archivo si se solicita (imagen, video, gif, etc), ejecuta un script según sea GET o POST y devuelve la respuesta, pasando los argumentos por terminal o por argumento. La principal ventaja que tiene es que es extremadamente fácil y cómo de expandir para funcionar en una escala mayor. En cuanto a la mayor dificultad encontrada, han sido los strings, que se nos complicaron desde el principio y nos dieron los errores que ya nos comentaba Eloy en las clases de teoría.

## Código
En el Git encontramos el código organizado en 4 carpetas, además de los archivos separados, que corresponden a:


src carpeta que contiene el código fuente, siendo este el del daemon, el server en sí, el parser y los sockets

files contiene archivos de ejemplo para mostrar en las pruebas

include contiene los .h de los ficheros de la carpeta src

cliente que contiene un pequeño cliente en python

Por último, fuera de estas carpetas disponemos del makefile, en el que está la automatización para una ejecución más cómoda, además de todas las herramientas para compilar; dos ficheros para realizar pequeñas pruebas y el fichero correspondiente al readme del Git.

## Troubleshooting
A lo largo de la ejecución de esta práctica en un ordenador de cero, hemos encontrado pocos problemas. Más allá de la funcionalidad básica que podamos llegar a encontrar en un ordenador habitualmente (sease soporte a C, Python y demás), solo hemos encontrado dos problemas que puedan dificultar o impedir la ejecución tal y como lo hemos comentado, y son los siguientes:

Los archivos que se utilizan en la prueba y demás han de ser descargados a parte, ya que hemos puesto a propósito que se excluya de GitHub la carpeta files. De esta forma, lo que hemos de hacer es introducir la carpeta files con el contenido que encontramos en Moodle como página web de ejemplo en su interior.
Solucionado el problema de la carpeta files, el siguiente y prácticamente único problema que podemos tener es que, por algún otro motivo, al hacer localhost:<nºpuerto> en el navegador podemos encontrarnos con que no funciona adecuadamente. Hemos encontrado este problema en algunos dispositivos muy concretos, y es que en Google Chrome en alguna ocasión ha dado problemas, mientras que en Mozilla ha funcionado adecuadamente, por lo que recomendamos su testeo en Mozilla Firefox si en Google Chrome hay algún problema.


## Conclusiones
En esta práctica hemos aprendido a crear un servidor desde cero, de la forma más básica, mediante C, teniendo que pegarnos con lo que son las dificultades reales de crear un servidor cara a cara, en lugar de "escondernos" detrás de Python, que podría realizar todo esto de una forma ágil y mucho más cómoda y rápida. De esta forma, hemos descubierto las distintas decisiones que esto implica tomar en su desarrollo, las dificultades y problemas que nos podemos encontrar, como gestionar peticiones HTML de forma adecuada y como funciona, en resumidas cuentas, un servidor.
Personalmente, hemos encontrado la práctica algo falta de diversión, resultando trabajosa y poco interesante en ocasiones, pero que nos ha permitido aprender como lanzar adecuadamente un servidor, las dificultades técnicas que nos podemos encontrar y como solucionarlas. De esta forma, ha resultado altamente enriquecedora, pero costosa en lo que a ánimos se refiere.
