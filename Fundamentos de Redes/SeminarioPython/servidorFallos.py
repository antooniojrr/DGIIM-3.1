import socket
import random
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("", 9999))
s.listen(1)
sc, addr = s.accept()
while True:
    recibido = sc.recv(1024)
    if (random.random()<0.33):
        recibido = 0
        print("Envio fallido")
        sc.send(b"Mandalo otra vez")
    else:
        print("Recibido mensaje: " + recibido.decode())
        sc.send(b"Recibido")
        break

print("Adios.")
sc.close()
s.close()