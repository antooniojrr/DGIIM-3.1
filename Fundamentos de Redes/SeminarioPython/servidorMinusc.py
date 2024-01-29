import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("", 9999))
s.listen(1)
sc, addr = s.accept()
while True:
    recibido = sc.recv(1024)
    if (recibido.decode() == "MATAME"):
        break
    print("Mensaje recibido:\t" + recibido.decode() + " de " + addr[0])
    respuesta = recibido.decode().lower()
    print("Mensaje convertido:\t" + respuesta)
    sc.send(respuesta.encode())

print("Adios.")
sc.close()
s.close()