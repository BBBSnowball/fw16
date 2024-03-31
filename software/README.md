The picture is based on this:
https://commons.wikimedia.org/wiki/File:Best_Nature_Picture_of_the_day.jpg

Steps:
1. Cut to sensible sub-image with aspect ratio of 640:400.
2. Scale to 640x400.
3. Create palette with the right colors and in the right order.
   (See table in section 7 in [here](https://files.waveshare.com/upload/f/f0/4.01inch-ePaper-F-Reference-Design.pdf))
4. Convert image to mode "indexed" with that palette and Floyd-Steinberg dithering.
5. Save as PNG.

Steps 2-5 will be done by the Python script.
