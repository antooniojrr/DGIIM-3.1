import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 9999))
mensaje = input("Mensaje a enviar >> ")
while True:
    
    s.send(mensaje.encode())
    respuesta=s.recv(1024)
    if respuesta == "otra vez":
        print("Envio fallido, se mandara otra vez")
    else:
        print("Envio completado")
        break
print("Adios.")
s.close()