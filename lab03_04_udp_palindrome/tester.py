from pathlib import Path
import socket
import subprocess

HOST = "127.0.0.1"
PORT = 2020

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(5.0)

folder = Path("tests")
output_dir = Path("output")
output_dir.mkdir(exist_ok=True)

files = sorted(
    [f for f in folder.iterdir() if f.is_file()],
    key=lambda f: int(f.stem.replace("test", ""))
)

for file in files:
    print(f"Wysyłam: {file}")

    data = file.read_bytes()
    sock.sendto(data, (HOST, PORT))

    try:
        response_bytes, _ = sock.recvfrom(65535)

        result = subprocess.run(
            ["od", "-A", "d", "-t", "u1", "-t", "c"],
            input=response_bytes,
            capture_output=True,
            check=True
        )

        od_output = result.stdout.decode("utf-8", errors="replace")

    except socket.timeout:
        od_output = "[TIMEOUT]"

    with open(output_dir / file.name, "w", encoding="utf-8") as f_out:
        f_out.write(od_output)

sock.close()