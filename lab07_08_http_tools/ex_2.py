import http.client
import sys

con = http.client.HTTPSConnection('th.if.uj.edu.pl')
con.request('GET', '/')
response = con.getresponse()

if response.status != 200:
    sys.exit(1)

header = response.getheader('Content-Type')

if not header.startswith('text/html'):
    sys.exit(1)

body = response.read()
body_text = body.decode('utf-8', 'replace')

if 'Institute of Theoretical Physics' not in body_text:
    sys.exit(1)

sys.exit(0)