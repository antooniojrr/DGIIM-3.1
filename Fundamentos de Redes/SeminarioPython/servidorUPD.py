import socket
s_server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s_server.bind(('',12345))
data, clientaddr = s_server.recvfrom(4096)
s_server.close()