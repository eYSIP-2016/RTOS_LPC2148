import serial
import time
import datetime

ser = serial.Serial(port='COM6',timeout=5, baudrate= 9600)
flag = ser.isOpen()
saveFile = open('t2.txt','w')
count=0
time_stamp = time.time()
date_stamp = datetime.datetime.fromtimestamp(time_stamp).strftime('%Y-%m-%d %H:%M:%S')
saveFile.write(str(date_stamp) + "\n")

while (count<9):
 data = ser.read(17) ;
 co=0
 time_stamp = time.time()
 date_stamp = datetime.datetime.fromtimestamp(time_stamp).strftime('%Y-%m-%d %H:%M:%S')
 saveFile.write(str(time_stamp) + ":" + "\t")
 while (co<16):

   print(data[co])
   saveFile.write(str(data[co]))
   saveFile.write(",")
   co=co+1;
 
 count=count+1;
 
 saveFile.write("\n")
 
saveFile.close()
