import sys
import serial
from PIL import Image, ImagePalette, ImageDraw

width = 280
height = 480

picture_path = "nature_picture2.png"
if len(sys.argv) == 2:
    picture_path = sys.argv[1]

def color_to_char(c):
    if c < 0 or c >= 3:
        raise Exception("The image must have a palette that exactly matches the ePaper! We got c=%r." % (c,))
    return b' -+#'[c:c+1]

#palette = ('RGB', ImagePalette.ImagePalette(b'\x00\x00\x00\xff\xff\xff\x00\xff\x00\x00\x00\xff\xff\x00\x00\xff\xff\x00\xff\x80\x00'))
palette_image = Image.new(mode="P", size=(1,1))
# Theoretical palette with full colors.
#palette_image.putpalette(b'\x00\x00\x00\xff\xff\xff\x00\xff\x00\x00\x00\xff\xff\x00\x00\xff\xff\x00\xff\x80\x00')
# More realistic palette. Colors are extract from a photo of the actual display.
#palette_image.putpalette(b'5-O\xcb\xc4\xb9[\x81vRJ\x80\xc0WS\xd7\xc0n\xd1~`')
palette_image.putpalette(b'\0\0\0\xff\xff\xff')

tty = serial.Serial("/dev/ttyACM0")
#tty.write(b"\x03I\nC\nD\n")
tty.write(b"\x03I\nD\n")
with Image.open(picture_path) as im:
    if im.size[0] > width or im.size[1] > height:
        im = im.resize((width, height))

    #print(repr(im.palette.colors))
    #print(repr(im.palette.getdata()))

    # This won't work because pillow ignores our palette. It only accepts WEB or ADAPTIVE.
    #im = im.convert(mode="P", dither=Image.Dither.FLOYDSTEINBERG, palette=palette, colors=6)

    # We can use quantize(), which does support a palette; but we have to convert to RGB first
    # because quantize() won't work with "P" or "RGBA" images.
    if True:
        im = im.convert(mode="RGB")
        im = im.quantize(colors=2, palette=palette_image)
    #print(repr(im.palette.colors))

    if False:
        draw = ImageDraw.Draw(im)
        draw.ellipse(((610, 350), (630, 370)), fill=2, outline=2)

    for y in range(height):
        row = b""
        for x in range(width):
            if x < im.size[0] and y < im.size[1]:
                row += color_to_char(im.getpixel((x, y)))
            else:
                row += ' '
        tty.write(row + b"\n")
        #print(row + b"\n")
