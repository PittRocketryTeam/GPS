import socket

HOST = "127.0.0.1"
PORT = 8080

running = True

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))
    s.setblocking(0);
    s.listen()

    print("waiting for connection...")
    conn = None
    addr = None
    ready = False
    while not ready:
        try:
            conn, addr = s.accept()
        except:
            continue

        ready = True

    print("connected!")

    data = []
    while running:
        try:
            data = conn.recv(1024)
            response = parse_cmd(data.decode("utf-8"))
            conn.send(response.encode("utf-8"))
        except:
            pass

        # do other work here
        # logging, etc...

def parse_cmd(cmd):
    global running
    print("got command: " + cmd)
    if cmd.startswith("q"):
        running = False
        return "server is shutting down..."
    elif cmd == "":
        return " "

    else:
        return "unknown command"


if __name__ == "__main__":
    main();
