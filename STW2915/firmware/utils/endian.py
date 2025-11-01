import struct

fi = open('test_bat.bmp', 'rb')
fo = open('test_bat2.bmp', 'wb')
hdr = fi.read(14)
(magic, file_size, res1, pix_offset) = struct.unpack('<HLLL', hdr)
print (hex(magic), hex(file_size), hex(res1), hex(pix_offset))
fi.seek(0)
hdr = fi.read(pix_offset)
fo.write(hdr)

for i in range((file_size - pix_offset) >> 1):
    d = fi.read(2)
    d2 = struct.pack('>H', struct.unpack('<H', d)[0])
    fo.write(d2) 

fo.close()
fi.close()
