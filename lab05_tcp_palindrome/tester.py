import socket

HOST = '127.0.0.1'
PORT = 2020
FILE_NAME = 'test.txt'
OUTPUT_FILE = 'output.txt'

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:
        s.connect((HOST, PORT))

        with open(FILE_NAME, 'rb') as file:
            raw_data = file.read()
            s.sendall(raw_data)
            
            print(f"Wysłano surowe bajty: {raw_data}")
        
        s.settimeout(1.0)
        full_response = b""
        
        try:
            while True:
                chunk = s.recv(4096)
                if not chunk:
                    break
                full_response += chunk
        except socket.timeout:
            pass
            
        print(f"Otrzymano łącznie: {repr(full_response.decode('ascii'))}\n")
        
        with open(OUTPUT_FILE, 'wb') as out_file:
            out_file.write(full_response)
            print(f"Zapisano odpowiedź do pliku: {OUTPUT_FILE}")
            
    except Exception as e:
        print(f"Błąd: {e}")