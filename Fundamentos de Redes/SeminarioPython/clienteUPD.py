import socket
s_client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s_client.sendto(b'Hola clase', ('localhost',12345))
s_client.close()