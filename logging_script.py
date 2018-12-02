#*** USING Python 2.7.10 ***
# Designed and developed on macOS Mojave version 10.14.1

# The purpose of this script is to make the process of range testing our GPS system easier. 
# 
# Testing procedure:
#	1. Power on ground and vehicle systems and ensure ground system is connected to a laptop via USB
#		- Ground system consists of Arduino + Adafruit LoRa transceiver + antenna + Laptop (this)
#		- Vehicle system consists of Arduino + Adafruit GPS + Adafruit LoRa transceiver + antenna +
#		  LiPo 3s battery
#	2. Make sure Test_Transmit_NMEA sketch is uploaded to the vehicle Arduino and the Arduino9x_RX
#	sketch is uploaded to the ground system Arduino
#	3. Make sure ground system remains stationary for the duration of the test (from the time this 
#	script is first run to the time the "exit" command is called)
#	4. Make sure Python 2.7.10 and geocoder (`pip install geocoder`) is installed on ground system 
#	laptop
#	5. Make sure the Arduino IDE serial monitor is NOT open when you run this script -- this will
#	render that USB port busy and this script won't work.
#	6. Run this script (`python logging_script.py`) 
#	7. Enter the USB port the ground system Arduino is connected to when prompted
#	8. Enter a filename for the logfile
#		- All logfiles are stored in `./logs` (which is created if it doesn't already exist)
#		- All logfiles have a timestamp appended to the filename specified
#	9. Enter the test number(s) that this test verifies and a brief description of the test(s)
#		- List of all GPS tests: https://docs.google.com/spreadsheets/d/19aqBz58Zo0jMhCm2MvR9Rple1NEmCtvDu1H1SuGdFbw/edit?usp=drive_web&ouid=112603863937581405437
#	10. At this point, the test has commenced and test data is being written out to the file. The 
#	script will prompt you to enter observations which are written to the logfile with a timestamp
#	11. When the test is complete and you want to stop logging, enter `exit` in the observation 
#	prompt
#	12. Check `./logs/` for the logfile for postprocessing (through test_processing.py, most likely)

import os
import fcntl
import datetime
import time
import geocoder
import signal

def main():

	# Get name of serial port arduino will write to
	usb_port = raw_input("USB port: ")

	# Check if port exists 
	if not os.path.exists("/dev/" + usb_port):
		print("USB port " + usb_port + " doesn't exist, exiting")
		exit(0)

	# TODO: Check if port is busy (although it shouldn't be unless you have the arduino serial 
	# monitor open)


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
	print(cmd)
	pid = os.spawnl(os.P_NOWAIT, cmd)
	print(pid)

	# Once setup is complete, wait on blocking raw_input call until user enters obstruction
	while 1:

		# Read in obstruction -- stop if user enters 'exit'
		write_out = raw_input("Obstruction: ")
		if write_out == "exit":
			print("Stopped logging, log: " + path)
			# Kill cat process from pid
			os.kill(pid, signal.SIGTERM)
			exit(0)
			
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