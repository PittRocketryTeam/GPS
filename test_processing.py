#*** USING Python 3.6.6 ***
# for Python 2 input() = raw_input()

import math
import Haversine

def main():
	tx_lat = 0
	tx_long = 0
	tx_alt = 0
	data_points = 0
	dist = 0

	filename = input("Enter the filename: ")
	fix = input("Care about fix (y/n)? ")

	# Read GPS data from user about receiver location
	rx_lat  = input("Enter receiver latitude (in DD): ")
	rx_long = input("Enter receiver longitude (in DD): ")
	# rx_alt  = input("Enter receiver altitude: ")

	file = open(filename, "r")
	for line in file:
		if line.startswith('Got'):
			data = line.split(',')

			# Exit if GPS didn't get a fix
			if (data[6] != 1) & (fix == "y"):
				print("Error: no fix")
				quit()
			
			# Read GPS data about transceiver location
			# tx_lat  = data[1] + data[2] #includes cardinal direction
			# tx_long = data[3] + data[4] #includes cardinal direction
			tx_lat = data[2]
			tx_long = data[4]
			tx_alt  = data[8]
			
			#convert cordinates from DMS to DD
			[tx_lat,tx_long]=to_DD(tx_lat,tx_long)
			# print("lat is "+str(tx_lat)+" long is "+str(tx_long)) #remove comment to display dec degree cordinates

			dist = calcDist(rx_lat, rx_long, tx_lat, tx_long)

		if line.startswith('RSSI'):
			data_points += 1
			print("Data point #" + str(data_points))
			print ("Distance: " + str(dist) + ", " + line)

	input("pause")

# Calculate distance	
def calcDist(rx_lat, rx_long, tx_lat, tx_long):
	# TODO: perform distance calculation
	


	return -1

#Convert from dms to decimal degrees
def to_DD(latitude,longitude):
	#gps outputs lat: DDMM.MMMM long: DDDMM.MMMM
    #DD = d + (min/60) + (sec/3600)

	#split strings
    lat_D= str(latitude)[:2]
    lat_M= str(latitude)[2:]

    long_D= str(longitude)[:3]
    long_M= str(longitude)[3:]

	#calc values
    lat_DD=float(lat_D)+float(lat_M)/60
    long_DD=float(long_D)+float(long_M)/60

    return lat_DD,long_DD

if __name__ == "__main__":
	main()
