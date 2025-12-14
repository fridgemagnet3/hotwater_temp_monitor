#!/usr/bin/python3
import socket
import time
import datetime

# create socket to listen for the broadcast packets
# sent periodically by the temperature sensor rpi
temp_sfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
temp_sfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
# make this non-blocking
temp_sfd.setblocking(0)
listen_address = ('0.0.0.0',52004)
temp_sfd.bind(listen_address)

# read latest temperature from rpi4 broadcast packet
def read_temp(sfd):
    pending = True
    temp_available = False
    # keep reading whilst there are datagrams so we
    # get the latest
    while pending:
        try:
            temperature, address = sfd.recvfrom(256)
            temp_available = True
        except socket.error:
            pending = False
            continue
    # return latest temperature if available
    if temp_available:
        return temperature
    else:
        return None

while True:
    packet = read_temp(temp_sfd)
    if packet != None:
        tank_data = packet.split()
        timestamp = datetime.datetime.fromtimestamp(int(tank_data[0]))
        tank_lower = float(tank_data[1])
        tank_upper = float(tank_data[2])
        print("%s upper=%dC, lower=%dC" % (timestamp.strftime("%H:%M:%S"),tank_upper,tank_lower))
    time.sleep(10)
    
