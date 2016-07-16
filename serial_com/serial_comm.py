import serial

ser = serial.Serial(port='COM6',timeout=None, baudrate= 9600)
flag = ser.isOpen()
saveFile = open('t2.txt','w')
count=0

while (count<9):
 data = ser.read(16) ;
 co=0

 while (co<16):

   print(data[co])
   saveFile.write(str(data[co]))
   co=co+1;
 #print(data)
 count=count+1;
 
 #saveFile.write("\n")
saveFile.close()
