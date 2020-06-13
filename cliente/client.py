from socket import *
import urllib.request
import urllib3
import http
#serverName='localhost'
#serverPort=1200
#clientSocket=socket(AF_INET, SOCK_STREAM)
#clientSocket.connect((serverName, serverPort))
#sentence=('Ingrese texto en minusculas:')
#clientSocket.send(bytes(sentence, encoding='utf8'))
#modifiedSentence=clientSocket.recv(1024)
#print ('Desde el servidor:',modifiedSentence)
#clientSocket.close()
i = 0
while (1):
    #contents = urllib.request.urlopen("http://localhost:1200").read()
    http = urllib3.PoolManager()
    response = http.request('GET', 'http://localhost:1200', headers={
    'key1': 'value1',
    'key2': 'value2'
    })
    print (response)
    print (i)
    i = i+1