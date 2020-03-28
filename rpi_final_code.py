#! /usr/bin/python
import serial
import time
import sys
import urllib2
import Adafruit_DHT
sleep = 4#in seconds

# Enter Your API key here
myAPI = '2Q0EW533M1TISQ0V' 
# URL where we will send the data, Don't change it
baseURL = 'https://api.thingspeak.com/update?api_key=%s' % myAPI 

bluetoothSerial = serial.Serial("/dev/rfcomm0",baudrate=9600)#if not change to 115200
print("Bluetooth connected")

def uploaddata():
    while True:
        try:
            # Sending the data to thingspeak
            conn = urllib2.urlopen(baseURL + '&field1=%s&field2=%s&field3=%s&field4=%s&field5=%s&field6=%s' % (dataarray[0], dataarray[1], dataarray[2], dataarray[3], room_humidity, room_temperature))
            print"connection ok"
            print"sent data=>"
            print("dist=%s,bin_temp=%s,bin_moist=%s,angle=%s,room_humidity=%s,room_temperature=%s"%(dataarray[0], dataarray[1], dataarray[2], dataarray[3], room_humidity, room_temperature))
            # Closing the connection
            conn.close()
        except:
            print 'connection failed'
        break

try:
	while 1:
		data = bluetoothSerial.readline()
		dataarray=data.replace(' ','').split(',')
		room_humidity, room_temperature = Adafruit_DHT.read_retry(11, 14)
        #dataarray[0]=dist
        #dataarray[1]=temp
        #dataarray[2]=moist
        #dataarray[3]=angle
		print(data)
		#time.sleep(sleep)
		if __name__ == "__main__":
                while True:
                        uploaddata()
                        time.sleep(sleep)
		
except KeyboardInterrupt:
	print("Quit")