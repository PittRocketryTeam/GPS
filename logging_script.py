#*** USING Python 2.7.10 ***
# be sure to install geocoder (`pip install geocoder`)

import os
import fcntl
import datetime
import time
import geocoder


def main():

	# Get name of serial port arduino will write to
	usb_port = raw_input("USB port: ")

	# Check if port exists 
	if not os.path.exists("/dev/" + usb_port):
		print("USB port " + usb_port + " doesn't exist, exiting")
		exit(0)

	# TODO: Check if port is busy 


	# Create file and append timestamp to end of filename
	st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d_%H:%M:%S')
	filename = raw_input("Log filename: ") + "_" + st
	
	# Create logs directiory if it doesn't exist
	path = os.getcwd() + "/logs/" + filename
	if not os.path.exists(os.getcwd() + "/logs/"):
		os.makedirs(os.getcwd() + "/logs/")

	# Open file in append mode
	file = open(path, "a")

	# Write test number(s) to file
	test_nums = raw_input("Enter test number(s): ")
	file.write("Test number(s): " + test_nums + "\n")

	# Write brief test description to file
	description = raw_input("Test description: ")
	file.write("Test description: " + description + "\n")

	# Write current GPS coordinates to start of file
	g = geocoder.ip('me')
	(latitude, longitude) = g.latlng
	file.write("Latitude: " + str(latitude) + "\n")
	file.write("Longitude: " + str(longitude) + "\n\n")

	# Run command to start logging serial data to file
	cmd = 'cat /dev/' + usb_port + ' > ' + path
	# print(cmd)
	os.spawnl(os.P_NOWAIT, cmd)

	# Once setup is complete, wait on blocking raw_input call until user enters obstruction
	while 1:

		# Read in obstruction -- stop if user enters 'exit'
		write_out = raw_input("Obstruction: ")
		if write_out == "exit":
			print("Stopped logging, log: " + path)
			break

		# Lock file
		fcntl.flock(file, fcntl.LOCK_EX | fcntl.LOCK_NB)

		# Write timestamp and obstruction to file
		st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')
		file.write(str(st) + " obstruction: " + write_out + "\n")

		# Flush file buffer
		file.flush()

		# Unlock file
		fcntl.flock(file, fcntl.LOCK_UN)



if __name__ == "__main__":
	main()