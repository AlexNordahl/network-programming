from pathlib import Path
import socket
import subprocess

HOST = "127.0.0.1"
PORT = 2020

folder = Path("tests")
output_dir = Path("output")
output_dir.mkdir(exist_ok=True)

files = sorted(
    [f for f in folder.iterdir() if f.is_file()],
    key=lambda f: int(f.stem.replace("test", ""))
)

for file in files:
    print(f"Wysyłam (TCP): {file}")
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(1.0)

    try:
        sock.connect((HOST, PORT))
        data = file.read_bytes()
        sock.sendall(data)

        response_bytes = b""
        sock.settimeout(0.5) 
        
        while True:
            try:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                response_bytes += chunk
            except socket.timeout:
                break 

        if not response_bytes:
            od_output = "[NO DATA RECEIVED]"
        else:
            result = subprocess.run(
                ["od", "-A", "d", "-t", "u1", "-t", "c"],
                input=response_bytes,
                capture_output=True,
                check=True
            )
            od_output = result.stdout.decode("utf-8", errors="replace")

    except (socket.timeout, ConnectionRefusedError) as e:
        od_output = f"[{type(e).__name__}]"
    finally:
        sock.close()

    with open(output_dir / file.name, "w", encoding="utf-8") as f_out:
        f_out.write(od_output)