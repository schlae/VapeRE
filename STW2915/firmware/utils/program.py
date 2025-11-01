import serial
import struct

s = serial.Serial('/dev/ttyUSB0', 500000, timeout=0.5)
f = open('test_bat2.bmp', 'rb')
da = f.read()
f.close()

def do_stat():
    s.write(b's')
    print(s.read(6))

def page_read(page_addr):
    s.write(b'r')
    s.write(struct.pack("<L", page_addr))
    dby = s.read(256)
    return dby    

def page_erase(page_addr):
    s.write(b'w')
    do_stat()
    s.write(b'E')
    s.write(struct.pack("<L", page_addr))
    ack = s.read(1)
    print(ack)
    do_stat()
    return ack

def page_program(page_addr, page_dat):
    s.write(b'w')
    do_stat()
    s.write(b'P')
    s.write(struct.pack("<LL", page_addr, len(page_dat))) # start addr, len
    s.write(page_dat)
    ack = s.read(1)
    print(ack)
    do_stat()
    return ack

data_len = len(da)
pages = data_len / 256
page_addr = 0

if 0:
    for i in range(20):
        ddd = page_read(256*i)
        print([ddd])
    quit()

while(1):
    a = page_erase(page_addr)
    if a != b'A': break
    print("Page erase %x" % page_addr)
    if data_len > 256:
        print("Page addr %x" % page_addr)
        a = page_program(page_addr, da[page_addr:page_addr + 256])
        print("Prog ", [da[page_addr:page_addr + 256]])
        if a != b'A': break
        page_addr += 256
        data_len -= 256
    else:
        print("Page addr %x" % page_addr)
        a = page_program(page_addr, da[page_addr:page_addr+data_len])
        if a != b'A': break
        print("Prog ", [da[page_addr:page_addr + data_len]])
        break

s.close()
