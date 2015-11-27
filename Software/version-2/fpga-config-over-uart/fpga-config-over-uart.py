# *******************************************************************************
# * @file    fpga-config-over-uart.py
# * @author  Hampus Sandberg
# * @version 0.1
# * @date    2015-09-08
# * @brief
# *******************************************************************************
#  Copyright (c) 2015 Hampus Sandberg.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# *******************************************************************************

import serial
from serial import SerialException
import sys
import os
import time
import getopt
import binascii
from progressbar import ProgressBar, Bar, Percentage

class bcolors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'
  BOLD = '\033[1m'
  UNDERLINE = '\033[4m'

verboseMode = 0

def main(argv):
  serPort = ''
  bitFileNum = ''
  binFile = ''
  startConfigOfNum = ''
  shouldReadHeaders = 0
  try:
    opts, args = getopt.getopt(argv,"p:n:b:s:rvlh",["serialPort=","bitFileNum=","binFile=","startConfigOfNum="])
  except getopt.GetoptError:
    print "Unspecified parameter, use -h to see valid parameters"
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      print "Parameters:"
      print "  -p <serialPort>\tSpecifiy the serial port to use"
      print "  -n <bitFileNum>\tSpecifiy the position the bitfile should be saved"
      print "  -b <binFile>\t\tPath to the bitfile"
      print "  -s <bitFileNum>\tStart config of the specified bitfile"
      print "  -r\t\t\tRead the bitfile headers stored in flash"
      print "  -v\t\t\tVerbose mode, i.e. display all information"
      print "  -l\t\t\tList the available serial ports"
      print "  -h\t\t\tDisplay this help"
      sys.exit()
    elif opt == '-l':
      print "Here are the available serial ports:"
      os.system("ls /dev/tty.*")
    elif opt in ("-p", "--serialPort"):
      serPort = arg
    elif opt in ("-n", "--bitFileNum"):
      bitFileNum = arg
    elif opt in ("-b", "--binFile"):
      binFile = arg
    elif opt in ("-s", "--startConfigOfNum"):
      startConfigOfNum = arg
    elif opt == '-v':
      global verboseMode
      verboseMode = 1
    elif opt == '-r':
      shouldReadHeaders = 1

  # Send the config file
  if (serPort != '' and bitFileNum != '' and binFile != ''):
    serialSend(serPort, bitFileNum, binFile)
    sys.exit(2)

  # Start the configuration
  if (serPort != '' and startConfigOfNum != ''):
    startConfig(serPort, startConfigOfNum)
    sys.exit(2)

  # Read bit file headers
  if (shouldReadHeaders == 1 and serPort != ''):
    readHeaders(serPort)
    sys.exit(2)    


# =============================================================================
# Function to convert integer to hex string
# =============================================================================
def convertIntToHexString(int_value):
  encoded = format(int_value, 'x')
  length = len(encoded)
  encoded = encoded.zfill(8)
  return encoded.decode('hex')

# =============================================================================
# Function to extend a message with the checksum
# =============================================================================
def extendMessageWithChecksum(message):
  checksum = 0
  for n in message:
    checksum = checksum ^ n
  message.extend(chr(checksum))
  return message

# =============================================================================
# Function to wait for ack
# =============================================================================
def waitForAck(serialPort):
  response = serialPort.read(1)
  if (response and ord(response) == int(0xDD)):
    if (verboseMode == 1):
      print bcolors.OKGREEN + "ACK received!" + bcolors.ENDC
  else:
    print bcolors.FAIL + "\n****** ACK not received!!! ******" + bcolors.ENDC
    sys.exit()

# =============================================================================
# Function to read the bit file headers in the flash
# =============================================================================
def readHeaders(serialPort):
  # Try to open the serial port
  try:
    ser = serial.Serial(serialPort, 115200, timeout=10)
  except:
    print bcolors.FAIL + "Invalid serial port. Is it connected?" + bcolors.ENDC
    sys.exit()

  if (not ser.isOpen()):
    print bcolors.FAIL + "Serial port not open" + bcolors.ENDC
    sys.exit()
  else:
    print bcolors.OKGREEN + "Serial port is open!" + bcolors.ENDC


  # Bit file 1
  readHeaderCommand = bytearray([0xAA, 0xBB, 0xCC, 0x40, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00, 0x04])
  readHeaderCommand = extendMessageWithChecksum(readHeaderCommand)
  ser.write(readHeaderCommand)
  
  # Wait for the data to arrive
  while (ser.inWaiting() < 5):
    time.sleep(0.1)

  data1 = ser.read(1);
  data2 = ser.read(1);
  data3 = ser.read(1);
  data4 = ser.read(1);
  data5 = ser.read(1);
  sizeOfBitFile1 = data1 + data2 + data3 + data4;
  sizeOfBitFile1 = int(sizeOfBitFile1.encode('hex'), 16)

  # Bit file 2
  readHeaderCommand = bytearray([0xAA, 0xBB, 0xCC, 0x40, 0x00, 0x05, 0x00, 0x0C, 0x00, 0x00, 0x04])
  readHeaderCommand = extendMessageWithChecksum(readHeaderCommand)
  ser.write(readHeaderCommand)
  
  # Wait for the data to arrive
  while (ser.inWaiting() < 5):
    time.sleep(0.1)

  data1 = ser.read(1);
  data2 = ser.read(1);
  data3 = ser.read(1);
  data4 = ser.read(1);
  data5 = ser.read(1);
  sizeOfBitFile2 = data1 + data2 + data3 + data4;
  sizeOfBitFile2 = int(sizeOfBitFile2.encode('hex'), 16)


  # Printout
  if (sizeOfBitFile1 == 4294967295):
    print "Size of bitfile 1 is: " + bcolors.FAIL + "ERASED" + bcolors.ENDC
  else:
    print "Size of bitfile 1 is: " + bcolors.OKGREEN + str(sizeOfBitFile1) + " bytes" + bcolors.ENDC

  if (sizeOfBitFile2 == 4294967295):
    print "Size of bitfile 2 is: " + bcolors.FAIL + "ERASED" + bcolors.ENDC
  else:
    print "Size of bitfile 2 is: " + bcolors.OKGREEN + str(sizeOfBitFile2) + " bytes" + bcolors.ENDC



# =============================================================================
# Function to start the config
# =============================================================================
def startConfig(serialPort, number):
  # Try to open the serial port
  try:
    ser = serial.Serial(serialPort, 115200, timeout=10)
  except:
    print bcolors.FAIL + "Invalid serial port. Is it connected?" + bcolors.ENDC
    sys.exit()

  if (not ser.isOpen()):
    print bcolors.FAIL + "Serial port not open" + bcolors.ENDC
    sys.exit()
  else:
    print bcolors.OKGREEN + "Serial port is open!" + bcolors.ENDC
    raw_input(bcolors.OKBLUE + "Press Enter to start fpga config..." + bcolors.ENDC)

  print "Sending start config of bit file number " + number + "...",
  startConfigCommand = bytearray([0xAA, 0xBB, 0xCC, 0x50, 0x00, 0x01, int(number)])
  startConfigCommand = extendMessageWithChecksum(startConfigCommand)
  #print "\n" + bcolors.WARNING + binascii.hexlify(startConfigCommand) + bcolors.ENDC
  ser.write(startConfigCommand)
  # Wait for ack
  waitForAck(ser)
  print bcolors.OKGREEN + "Done configuring bit file" + bcolors.ENDC


# =============================================================================
# Function to send the bit file
# =============================================================================
def serialSend(serialPort, bitFileNumber, binaryFile):
  # Make sure the bit file number is valid
  if int(bitFileNumber) != 1 and int(bitFileNumber) != 2:
    print bcolors.FAIL + "Bit file number can only be 1 or 2" + bcolors.ENDC
    sys.exit()
  # Calculate the start address from the bit file number
  startAddress = int(bitFileNumber) * 393216
  startAddressAsHexString = convertIntToHexString(startAddress)

  # Convert to bytearray
  startAddressAsByteArray = bytearray(startAddressAsHexString)
  #print "\n" + bcolors.WARNING + binascii.hexlify(startAddressAsByteArray) + bcolors.ENDC

  # Try to open the serial port
  try:
    ser = serial.Serial(serialPort, 115200, timeout=10)
  except:
    print bcolors.FAIL + "Invalid serial port. Is it connected?" + bcolors.ENDC
    sys.exit()

  if (not ser.isOpen()):
    print bcolors.FAIL + "Serial port not open" + bcolors.ENDC
    sys.exit()
  else:
    print bcolors.OKGREEN + "Serial port is open!" + bcolors.ENDC
    raw_input(bcolors.OKBLUE + "Press Enter to start sending data..." + bcolors.ENDC)

  # ===========================================================================
  # Erase any old bit files in the flash at this position
  if (verboseMode == 1):
    print "Sending erase bit file command",
  eraseCommand = bytearray([0xAA, 0xBB, 0xCC, 0x22, 0x00, 0x01, int(bitFileNumber)])
  eraseCommand = extendMessageWithChecksum(eraseCommand)
  ser.write(eraseCommand)
  # Wait for ack
  waitForAck(ser)

  # ===========================================================================
  # Set the flash write address to the start address
  if (verboseMode == 1):
    print "Sending write address command for info",
  writeAddressCommand = bytearray([0xAA, 0xBB, 0xCC, 0x10, 0x00, 0x04])
  writeAddressCommand.extend(startAddressAsByteArray)
  writeAddressCommand = extendMessageWithChecksum(writeAddressCommand)
  ser.write(writeAddressCommand)
  # Wait for ack
  waitForAck(ser)
  
  # ===========================================================================
  # Get the filesize of the bitfile -> Number of bytes
  byteCount = os.path.getsize(binaryFile)
  # Write the filesize to the first 4 bytes of the address
  if (verboseMode == 1):
    print "Will send bitfile size of " + str(byteCount) + " (" + hex(byteCount) + ") bytes...",
  msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, 0x04])
  msg.extend(bytearray(convertIntToHexString(byteCount)))
  msg = extendMessageWithChecksum(msg)
  #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC
  ser.write(msg)
  # Wait for ack
  waitForAck(ser)

  # ===========================================================================
  # Write the next 64 bytes with the name of the bitfile
  # fileName = os.path.basename(binaryFile)
  # msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, 0x40])
  # msg.extend(bytearray(data))


  # ===========================================================================
  # Move the write address forward to the next page (256 bytes forward from the start)
  newAddressAsByteArray = bytearray(convertIntToHexString(startAddress + 256))
  if (verboseMode == 1):
    print "Sending write address command for data",
  writeAddressCommand = bytearray([0xAA, 0xBB, 0xCC, 0x10, 0x00, 0x04])
  writeAddressCommand.extend(newAddressAsByteArray)
  writeAddressCommand = extendMessageWithChecksum(writeAddressCommand)
  ser.write(writeAddressCommand)
  # Wait for ack
  waitForAck(ser)

  # ===========================================================================
  # Variables used to display a progress bar of the transmitted data
  bytesSent = 0
  pbar = ProgressBar(widgets=[Percentage(), Bar()], maxval=byteCount).start()
  if (verboseMode == 0):
    print "Sending data:"

  # Read the bitfile
  with open(binaryFile, "rb") as f:
    byte = 1
    checksum = 0
    count = 0
    data = bytearray()
    iterationCount = 0
    while byte:
      # Progress bar
      if (verboseMode == 0):
        pbar.update(bytesSent)

      # Do stuff with byte.
      byte = f.read(1)
      if (byte):
        data.extend(byte)
        count += 1

        if (count == 256):
          bytesSent += 256.0
          iterationCount += 1
          if (verboseMode == 1):
            print str(iterationCount) + " : " + str(count) + "bytes read...",

          # Add the header, command and data count to the checksum
          # Construct the message
          msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x01, 0x00])
          msg.extend(data)
          msg = extendMessageWithChecksum(msg)
          if (verboseMode == 1):
            print "checksum: 0x" + binascii.hexlify(msg[len(msg)]) + "...",
          # DEBUG: Print the message
          #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC
          #raw_input(bcolors.OKBLUE + "Stop a bit" + bcolors.ENDC)

          # Send the message
          if (verboseMode == 1):
            print "sending data...",
          ser.write(msg)

          # Wait for ack
          waitForAck(ser)

          # Clear the data that has been sent
          del data[0:count]
          count = 0

    # Send the last bytes if there are any
    if (count != 0):
      if (verboseMode == 1):
        print str(count) + " bytes left to send...",
      # Progress bar
      elif (verboseMode == 0):
        pbar.update(bytesSent)

      # Construct the message
      msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, count])
      msg.extend(data)
      msg = extendMessageWithChecksum(msg)
      if (verboseMode == 1):
        print "checksum: 0x" + binascii.hexlify(msg[len(msg)]) + "...",
      # DEBUG: Print the message
      #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC

      # Send the message
      if (verboseMode == 1):
        print "sending data...",
      ser.write(msg)

      # Wait for ack
      waitForAck(ser)

    if (verboseMode == 0):
      pbar.finish()
    print bcolors.OKGREEN + "Done sending " + str(byteCount) + " bytes of data" + bcolors.ENDC


if __name__ == "__main__":
  main(sys.argv[1:])
