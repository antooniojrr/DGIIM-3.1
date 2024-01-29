import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("", 9999))
s.listen(1)
sc, addr = s.accept()
while True:
    recibido = sc.recv(1024)
    if recibido.decode() == "close":
        break
    print(str(addr[0]) + " dice: ", recibido.decode())
    sc.send(recibido)
print("Adios.")
sc.close()
s.close()