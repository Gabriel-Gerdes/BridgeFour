# Modified from
# Super Simple Serial Monitor written in Python
# https://gist.github.com/maxwiese/db51e4a60d0c09727413e7f5f45af03f
# Dependancies
#    pip install pyserial

import subprocess
proc = subprocess.call(["pip", "install", "pyserial"])
proc.wait()

try:
    import serial
except ImportError:
    subprocess.call(["pip", "install", "serial"])
    import serial

from time import sleep
def search_for_ports():
    ports = list(serial.tools.list_ports.comports())
    return ports

if __name__ == "__main__":
    print('available ports')
    for index, port in enumerate(search_for_ports()):
        print('[{}] {}'.format(index, port.description))

    print('\nselect port to connect (use index number)')
    
    while True:
        try:
            ser_device = input('> ')
            port = search_for_ports()[ser_device].device
            break
        except:
            print('no valid port')

    while True:
        try:
            print('\nenter baudrate')
            baudrate = int(input('> '))
            break
        except:
            print('\nno valid baudrate')
    try:
        serial_conn = serial.Serial(port, baudrate)
    except:
        print('\nCant connect to port {}'.format(port))
        exit(0)

    count = 0
    while not serial_conn.is_open:
        sleep(0.1)
        if count == 10:
            print('\nTimed out')
            exit(0)

    print('\nconnection established') 

    while serial_conn.is_open:
        try:
            print(serial_conn.readline())
        except:
            break

    print('\nconnection lost')
    exit(0) 
