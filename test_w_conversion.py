#*** USING Python 3.6.6 ***
# for Python 2 input() = raw_input()
#fixed error in assigning cordinates using NMEA data

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
	rx_lat  = input("Enter receiver latitude: ")
	rx_long = input("Enter receiver longitude: ")
	rx_alt  = input("Enter receiver altitude: ")

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
			# tx_long = data[3] + data[4]
			tx_lat = data[2]
			tx_long = data[4]
			tx_alt  = data[8]
			
			#convert cordinates from DMS to DD
			[tx_lat,tx_long]=to_DD(tx_lat,tx_long)
			#print("lat is "+str(tx_lat)+" long is "+str(tx_long)) #remove comment to display dec degree cordinates

			dist = calcDist(rx_lat, rx_long, rx_alt, tx_lat, tx_long, tx_alt)

		if line.startswith('RSSI'):
			data_points += 1
			print("Data point #" + str(data_points))
			print ("Distance: " + str(dist) + ", " + line)

	input("pause")

# Calculate distance	
def calcDist(rx_lat, rx_long, rx_alt, tx_lat, tx_long, tx_alt):
	# TODO: perform distance calculation
	

	return -1

#Convert from dms to decimal degrees
def to_DD(tx_lat,tx_long):
	#gps outputs lat: DDMM.MMMM long: DDDMM.MMMM
    #DD = d + (min/60) + (sec/3600)

	#split strings
    tx_lat_D= str(tx_lat)[:2]
    tx_lat_M= str(tx_lat)[2:]

    tx_long_D= str(tx_long)[:3]
    tx_long_M= str(tx_long)[3:]

	#calc values
    tx_lat_DD=float(tx_lat_D)+float(tx_lat_M)/60
    tx_long_DD=float(tx_long_D)+float(tx_long_M)/60

    return tx_lat_DD,tx_long_DD

if __name__ == "__main__":
	main()
