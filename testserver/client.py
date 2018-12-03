import socket

HOST = "127.0.0.1"
PORT = 8080

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    
    while True:
        cmd = input("testserver~$ ")
        if cmd == "":
            continue
        s.sendall(cmd.encode("utf-8"))
        data = s.recv(1024)
        print("server reply: " + data.decode("utf-8"))
    
if __name__ == "__main__":
    main();
