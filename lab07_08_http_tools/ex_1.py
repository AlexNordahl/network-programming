import http.client
import sys

def get_server_name(host, use_http = False):
    con = None
    if use_http:
        con = http.client.HTTPConnection(host, 80)
    else:
        con = http.client.HTTPSConnection(host, 443)

    con.request('GET', '/')
    response = con.getresponse()
    name = response.getheader('Server')
    con.close()

    return name

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} <domain name>')
        sys.exit(1)

    http_serv_name = get_server_name(host=sys.argv[1], use_http=True)
    https_serv_name = get_server_name(host=sys.argv[1])

    print(f'port 80: {http_serv_name}')
    print(f'port 443: {https_serv_name}')