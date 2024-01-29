import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 9999))
while True:
    mensaje = input("Mensaje a enviar >> ")
    s.send(mensaje.encode())
    if mensaje == "MATAME":
        break
    respuesta=s.recv(1024)
    print("Mensaje transformado: " + respuesta.decode())
print("Adios.")
s.close()