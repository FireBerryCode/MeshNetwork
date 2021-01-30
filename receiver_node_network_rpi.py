from __future__ import print_function
import time
from struct import *
from RF24 import *
from RF24Network import *

"""
Codigo para la Raspberry como Nodo raiz. Este ejemplo espera un struct del tipo

struct payloadRF24 {    
  char id[3];   
  float temperature;  
  float humidity;
  uint16_t visible;
  uint16_t ir;
  float lux;
  int irFlame;
  int mq2;
  int mq7;
};

"""

def unpack_c_struct(payload):
	print(bytes(payload))
	id_node_1, id_node_2, _,  temperature, humidity, \
               visible, ir, lux, ir_flame, mq2, mq7 \
               = unpack('<3cffHHfhhh', bytes(payload))
	id_node = id_node_1 + id_node_2
	return id_node, temperature, humidity, visible, ir, \
           lux, ir_flame, mq2, mq7

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
network.begin(120, this_node)
radio.printDetails()
packets_sent = 0
last_sent = 0

while 1:
    network.update()
    while network.available():
        print('===========================')
        print("Network available")
        header, payload = network.read(25)
        print("payload length ", len(payload))
        measure = unpack_c_struct(payload)
        print('ID: ', measure[0])
        print('Temperature: ', measure[1])
        print('Humidity: ', measure[2])
        print('Visible Light: ', measure[3])
        print('Infrared Light: ', measure[4])
        print('Lux: ', measure[5])
        print('Flame Detector: ', measure[6])
        print('MQ2: ', measure[7])
        print('MQ7: ', measure[8])
        print('===========================')
    time.sleep(1)

