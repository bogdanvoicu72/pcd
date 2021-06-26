import socket

HOST = '127.0.0.1'
PORT = 8888


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST,PORT))
    s.sendall(b'Hello, I am a python client')
    data = s.recv(1024)

print('Am primit', repr(data))