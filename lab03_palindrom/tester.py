import socket

HOST = "127.0.0.1"
PORT = 2020
INPUT = "test-dane.txt"
OUTPUT = "wynik-z-serwera.txt"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(5.0)

with open(INPUT, "r") as f_in, open(OUTPUT, "w") as f_out:
    for line in f_in:
        line = line.rstrip("\n")

        print(f"Wysyłam: {line}")

        sock.sendto(line.encode(), (HOST, PORT))

        try:
            data, _ = sock.recvfrom(65535)
            response = data.decode(errors="replace")
        except socket.timeout:
            response = "[TIMEOUT]"

        f_out.write(response + "\n")

sock.close()