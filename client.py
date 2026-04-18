import socket

HOST = '127.0.0.1'
PORT = 3000

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    print("Connected to server")

    while True:
        msg = input(">>> ")
        if msg:
            s.sendall((msg + "\n").encode("utf-8"))
        
        data = s.recv(4096)
        print("Server:", data.decode("utf-8"))