import serial

s = serial.Serial('/dev/ttyUSB0', 500000, timeout=0.1)
f = open('out.bin', 'wb')
s.read(100) # dummy
s.write(b'D')
while(1):
    data = s.read(2048)
    f.write(data)
    if len(data) != 2048:
        break
    

f.close()
s.close()
