Steps:
1. Cut to sensible sub-image with aspect ratio of 640:400.
2. Scale to 640x400.
3. Create palette with the right colors and in the right order.
   (See table in section 7 in [here](https://files.waveshare.com/upload/f/f0/4.01inch-ePaper-F-Reference-Design.pdf))
4. Convert image to mode "indexed" with that palette and Floyd-Steinberg dithering.
5. Save as PNG.

Steps 2-5 will be done by the Python script.

Sources
-------

The picture `nature_picture` is based on
[this picture](https://commons.wikimedia.org/wiki/File:Best_Nature_Picture_of_the_day.jpg)
(Wikimedia Commons, Author Guruspsingh, CC BY-SA).

The picture `CSIRO ScienceImage 3881 Five Antennas at Narrabri` is based on
[this picture](https://commons.wikimedia.org/wiki/File:CSIRO_ScienceImage_3881_Five_Antennas_at_Narrabri.jpg)
(Wikimedia Commons, Attribution: CSIRO, CC BY).

The picture `Vespula_germanica_Richard_Bartz` is based on
[this picture](https://commons.wikimedia.org/wiki/File:Vespula_germanica_Richard_Bartz.jpg)
(Wikimedia Commons, Richard Bartz, CC BY-SA).

The pictures in waveshare-example-pictures are
from [Waveshare's ZIP file](https://files.waveshare.com/upload/7/71/E-Paper_code.zip),
directory `RaspberryPi_JetsonNano/c/pic`.

