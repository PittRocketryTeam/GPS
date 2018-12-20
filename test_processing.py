#*** USING Python 3.6.6 ***
# for Python 2 input() = raw_input()
# This script assumes that the static receiver latitude and longitude are stored at
# the stop of the logfile in the following format:
# 	Latitude: 39.9925
# 	Longitude: -75.1415

import math
import sys
from Haversine import Haversine

def main():
	data_points = 0

	logname = sys.argv[1]
	outputFile = open(logname + "_processed", "x")

	inputFile = open(logname, "r")
	for line in inputFile:
		if (line.startswith("Test")):
			outputFile.write(line + "\n")


		# Read in data point
		if line.startswith('Got'):
			data = line.split(',')

			# Exit if GPS didn't get a fix
			if (not hasFix(data)):
				print("Error: no fix")
				quit()
			
			# Read transmitter GPS coordinates
			tx_lat = data[2]
			tx_long = data[4]
			tx_alt  = data[8]
			
			#convert cordinates from DMS to DD
			[tx_lat,tx_long]=to_DD(tx_lat,tx_long)

			# Use first coordinates as starting coordinates
			if (data_points == 0):
				rx_lat = tx_lat
				rx_long = tx_long

			dist = calcDist(rx_lat, rx_long, tx_lat, tx_long)

		if line.startswith('RSSI'):
			data_points += 1
			outputFile.write("Data point #" + str(data_points))
			outputFile.write("\nDistance: " + str(dist) + " m, " + line)

def hasFix(data):
	return (data[2] and data[4] and data[8])


# Calculate distance	
def calcDist(rx_lat, rx_long, tx_lat, tx_long):

	# TODO: perform distance calculation
	dist = Haversine([rx_long, rx_lat], [tx_long, tx_lat])
	return dist.meters

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
