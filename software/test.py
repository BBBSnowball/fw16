import serial
from PIL import Image

def color_to_char(c):
    if c < 0 or c >= 7:
        raise Exception("The image must have a palette that exactly matches the ePaper!")
    return b'kwbgryoc'[c:c+1]

tty = serial.Serial("/dev/ttyACM0")
tty.write(b"I\nD\n")
with Image.open("nature_picture.png") as im:
    for y in range(400):
        for x in range(640):
            tty.write(color_to_char(im.getpixel((x, y))))
            #print(color_to_char(im.getpixel((x, y))))
        tty.write(b"\n")
    #tty.write(b"\n")
