# Author: Buhaianu Razvan
# email: razvanb96@outlook.com

from PIL import ImageGrab
import time
import os
from colour import Color
import serial

#//////////////////////////////////////////////////////////////////////////////////////////////////////////
# GLOBAL DEFINES
#//////////////////////////////////////////////////////////////////////////////////////////////////////////
DECIMATE       = 5   # skip every DECIMATE number of pixels to speed up calculation
LEDNO          = 41 # number of LEDS
#//////////////////////////////////////////////////////////////////////////////////////////////////////////
# open the serial as on the ATMega config on the discovered port
ser = serial.Serial()
ser.baudrate = 9600
ser.port = 'COM4'
ser.parity = 'E'
ser.open()
ser.timeout = 0.3
while True:
    red   = 0
    green = 0
    blue  = 0
    colors= []

    #//////////////////////////////////////////////////////////////////////////////////////////////////////////
    # CALCULATE AVERAGE SCREEN COLOUR
    #//////////////////////////////////////////////////////////////////////////////////////////////////////////
    image = ImageGrab.grab()  # take a screenshot
    #print image.size
    flag = True

    for i in range(0, LEDNO): #the number of LED-s
        counter = 0
        red = 0
        green = 0
        blue = 0
        mininterval = i * image.size[0] / 41
        maxinterval = (i + 1) * image.size[0] / 41
        for y in range(0, int(round(image.size[1]/10))):  #loop over the height area
            for x in range(int(round(mininterval)), int(round( maxinterval)), 4):  #loop over the width of the area
                color = image.getpixel((x, y))  #grab a pixel
                # calculate sum of each component (RGB)
                red = red + color[0]
                green = green + color[1]
                blue = blue + color[2]
                counter += 1
        red = (int(round(red / counter)))
        green = (int(round(green / counter)))
        blue = (int(round(blue / counter)))
        colors.append(red)
        colors.append(green)
        colors.append(blue)

    ser.write(colors)
    # send to serial the data of this screenshot
    s = ser.read(10)
    # confirm that the data is received and prepared for next screen data

