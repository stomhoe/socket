usar sockets udp para la comunicación entre control sim y render

puede q toque añadir un hilo mas para leer del socket sin bloquear

si el control es el q emite los paquetes udp, no tiene problemas

el sim sí va a sufrir bloqueos al leer del socket(bloqueante), puede q requiera un nuevo hilo

aunq puede q no haga falta uno de los hilos extra (en no se q caso), revisar

ahora hay tres programas distintos (no hay bibliotecas)

añadir docker