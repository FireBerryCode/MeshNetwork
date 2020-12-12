from __future__ import print_function
import time
from struct import *
from RF24 import *
from RF24Network import *

"""
Codigo para la Raspberry como Nodo raiz. Este ejemplo espera un struct del tipo

{
  char id[3];   
  float temperature; 
}

Habria que cambiarlo para el caso final y modificar la funcion unpack_c_struct(payload)
El id no hay que cambiarlo porque en principio los ids de los nodos hijos seran siempre 
01, 02, 03, 04, 05 (se puede establecer comunicacion desde nodos nietos, pero miedo 
me da)
"""

def unpack_c_struct(payload):
	id_node_1, id_node_2, _,  temperature = unpack('<3cf', bytes(payload))
	id_node = id_node_1 + id_node_2
	return id_node, temperature

radio = RF24(22, 0)
network = RF24Network(radio)

millis = lambda: int(round(time.time() * 1000))
octlit = lambda n:int(n, 8)

# Address of our node in Octal format (01, 021, etc)
this_node = octlit("00")

# Address of the other node
other_node = octlit("01")

radio.begin()
time.sleep(0.1)
network.begin(90, this_node)    # channel 90
radio.printDetails()
packets_sent = 0
last_sent = 0

while 1:
    network.update()
    while network.available():
        print("Network available")
        header, payload = network.read(8)
        print("payload length ", len(payload))
        id_node, temperature = unpack(payload)
        print('Temperature ', temperature, ' ºC at node ', id_node)
    time.sleep(1)

