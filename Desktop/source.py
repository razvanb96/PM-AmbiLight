# Author: Frak Al-Nuaimy 
# email: frakman@hotmail.com

from PIL import ImageGrab
import time
import os
from colour import Color
import serial

#//////////////////////////////////////////////////////////////////////////////////////////////////////////
# GLOBAL DEFINES
#//////////////////////////////////////////////////////////////////////////////////////////////////////////
#HEIGHT         = 1920   #now using image.size[1] dynamically
#WIDTH          = 1200   #now using image.size[0] dynamically
LOOP_INTERVAL  = 0.5  # how often we calculate screen colour (in seconds)
DURATION       = 3    # how long it takes bulb to switch colours (in seconds)
DECIMATE       = 5   # skip every DECIMATE number of pixels to speed up calculation
#get your unit-unique token from http://developer.lifx.com/ and use it here
TOKEN          = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" 
BULB_NAME      = "all"  # you can use any label you've assigned your bulb here, but if you do, make sure you use the curl syntax alternative with "label:" instead. I provided a commented out version of it below.
#//////////////////////////////////////////////////////////////////////////////////////////////////////////
ser = serial.Serial()
ser.baudrate = 9600
ser.port = 'COM4'
ser.parity = 'E'
ser.open()
# while True:
# 	ser.write(bytes(b'abc'));
# 	s = ser.read(10);
# 	print (s);
# # run loop
while True:
    red   = 0
    green = 0
    blue  = 0
    colors= []

    time.sleep(LOOP_INTERVAL) #wake up ever so often and perform this ...
    #//////////////////////////////////////////////////////////////////////////////////////////////////////////
    # CALCULATE AVERAGE SCREEN COLOUR
    #//////////////////////////////////////////////////////////////////////////////////////////////////////////
    image = ImageGrab.grab()  # take a screenshot
    #print image.size
    counter = 0
    for y in range(0, int(round(image.size[1]/10))):  #loop over the height
        for i in range (0 , 38):
            mininterval = i *image.size[0] / 40
            maxinterval = (i+1) * image.size[0] / 40
            for x in range(int(round(mininterval)),int(round( maxinterval))):  #loop over the width
                #print "\n coordinates   x:%d y:%d \n" % (x,y)
                color = image.getpixel((x, y))  #grab a pixel
                # calculate sum of each component (RGB)
                red = red + color[0]
                green = green + color[1]
                blue = blue + color[2]
                counter+=1
                #print red + " " +  green + " " + blue
                #print "\n totals   red:%s green:%s blue:%s\n" % (red,green,blue)
                #print color
                #print(time.clock())
            red = (int(round(red/ counter)))
            green = (int(round(green/ counter)))
            blue = (int(round(blue/ counter)))

            colors.append(red)
            colors.append(green)
            colors.append(blue)

    print(colors)
    ser.write(colors)
    # red = (( red / ( (image.size[1]/DECIMATE) * (image.size[0]/DECIMATE) ) ) )/255.0
    # green = ((green / ( (image.size[1]/DECIMATE) * (image.size[0]/DECIMATE) ) ) )/255.0
    # blue = ((blue / ( (image.size[1]/DECIMATE) * (image.size[0]/DECIMATE) ) ) )/255.0
    # c= Color(rgb=(red, green, blue))
    # print ("red is" + str(red))

    # print "\n average   red:%s green:%s blue:%s" % (red,green,blue)
    #print "\n average   hue:%f saturation:%f luminance:%f" % (c.hue,c.saturation,c.luminance)
    # print "\n average  (hex) "+  (c.hex)
    #//////////////////////////////////////////////////////////////////////////////////////////////////////////

    #//////////////////////////////////////////////////////////////////////////////////////////////////////////
    # PROGRAM LIFX BULB WITH COLOUR
    #//////////////////////////////////////////////////////////////////////////////////////////////////////////
    # cmd = " c:\\curl\\curl.exe -u \""+TOKEN+":\" -X PUT -d \"color=" + str(c.hex) + "\" -d \"duration=" +str(DURATION)+ "\" \"https://api.lifx.com/v1beta1/lights/"+BULB_NAME+"/color\""
    # #cmd = " c:\\curl\\curl.exe -u \""+TOKEN+":\" -X PUT -d \"color=" + str(c.hex) + "\" -d \"duration=" +str(DURATION)+ "\" \"https://api.lifx.com/v1beta1/lights/label:"+BULB_NAME+"/color\""
    # print cmd
    # os.system(cmd)