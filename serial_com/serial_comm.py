import serial
ser = serial.Serial(port='COM6',timeout=None, baudrate= 9600)
flag = ser.isOpen()

while 1:
 data = ser.read(14) ;
 print(data)
