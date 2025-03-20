import random
import math
import sys
import os


class DataField:
	def __init__(self, field_name, gain, offset, start_byte, byte_len, lower_bound, upper_bound, packet, variation_method):
		self.field_name = field_name
		self.packet = packet
		self.start_byte = start_byte
		self.byte_len = byte_len
		self.changes = 0
		self.value = 0
		self.gain = gain
		self.offset = offset
		self.low_b = lower_bound
		self.upp_b = upper_bound
		self.v_method = variation_method

########### Getters & Setters ############
	def set_value(self, new_value):
		self.value = new_value

	def get_value(self):
		return self.value 

	def set_gain(self, new_value):
		self.gain = new_value

	def get_gain(self):
		return self.gain 
	
	def set_offset(self, new_value):
		self.offset = new_value

	def get_offset(self):
		return self.offset 

	def set_low_b(self, new_value):
		if new_val > self.upp_b:
			print("Error: Lower bound must be less than upper bound")
		else:
			self.low_b = new_value

	def get_low_b(self):
		return self.low_b

	def set_upp_b(self, new_value):
		if new_val < self.low_b:
			print("Error: Upper bound must be more than lower bound")
		else:
			self.upp_b = new_value

	def get_upp_b(self):
		return self.upp_b

	def set_v_method(self, new_method):
		self.v_method = new_method

########### Variation Methods ############
	
	def refresh_value(self):
		self.changes += 1
		if self.v_method == 0:
			new_val = self.oscillate()
		elif self.v_method == 1:
			new_val = self.randomise()
		elif self.v_method == 2:
			new_val = self.linear()
		elif self.v_method == 3:
			new_val = self.fixed()
		self.value = round(new_val, 2)

	def oscillate(self):
		return (0.5*(self.upp_b - self.low_b)) * (1 + math.sin(2*self.changes*(math.pi/180)))

	def randomise(self):
		return self.low_b + random.random()*(self.upp_b - self.low_b)

	def linear(self):
		return self.low_b + (self.changes % (self.upp_b - self.low_b))

	def fixed(self):
		return self.value 

	def encode(self):
		raw_val = int((self.value - self.offset) // self.gain)
		mask  = 0xFF
		for b in range(self.byte_len):
			self.packet.data[self.start_byte + self.byte_len - (b+1)] = int((raw_val >> b*8) & mask)

		



########### Other Methods ############

	def __str__(self):
		return f"{self.field_name:<20}:{self.value:>9}"

	
class CAN_Packet:
	def __init__(self, p_id):
		self.p_id = p_id
		self.data = [0, 0, 0, 0, 0, 0, 0, 0]

	def send_packet(self):
		string_form = ""
		for byte in self.data:
			string_form += f"{byte:0{2}x}"
		#print(f"./canusb -d /dev/ttyUSB0 -s 1000000 -n 1 -i {self.p_id:0{3}x} -j {string_form}")
		os.system(f"./canusb -d /dev/ttyUSB0 -s 1000000 -n 1 -i {self.p_id:0{3}x} -j {string_form}")

	def clear_packet(self):
		self.data = [0, 0, 0, 0, 0, 0, 0, 0]


def main():
	packets = {}

	packets[0x360] = CAN_Packet(0x360)
	packets[0x361] = CAN_Packet(0x361)
	packets[0x370] = CAN_Packet(0x370)
	packets[0x372] = CAN_Packet(0x372)
	packets[0x3E0] = CAN_Packet(0x3E0)
	packets[0x3EB] = CAN_Packet(0x3EB)
	packets[0x469] = CAN_Packet(0x469)
	packets[0x470] = CAN_Packet(0x470)
	packets[0x477] = CAN_Packet(0x477)

	vals = []

	vals.append(DataField("RPM", 1, 0, 0, 2, 0, 13500, packets[0x360], 0))
	vals.append(DataField("MAP", 0.1, 0, 2, 2, 0, 1000, packets[0x360], 2))
	vals.append(DataField("Throttle Pos.", 0.1, 0, 4, 2, -1, 101, packets[0x360], 0))

	vals.append(DataField("Fuel Pres.", 0.0145, -14.7, 0, 2, 20, 50, packets[0x361], 2))
	vals.append(DataField("Oil Pres.", 0.0145, -14.7, 2, 2, 0, 500, packets[0x361], 2))

	vals.append(DataField("Speed", 0.0621, 0, 0, 2, -1, 100, packets[0x370], 0))

	vals.append(DataField("Battery Volts", 0.1, 0, 0, 2, 9, 15.5, packets[0x372], 1))

	vals.append(DataField("Coolant Temp.", 0.1, -273, 0, 2, 0, 120, packets[0x3E0], 0))
	vals.append(DataField("Air Temp.", 0.1, -273, 2, 2, 0, 50, packets[0x3E0], 0))
	vals.append(DataField("Oil Temp.", 0.1, -273, 6, 2, 30, 105, packets[0x3E0], 0))

	vals.append(DataField("IGN Angle", 0.1, 0, 4, 2, 0, 25, packets[0x3EB], 1))

	vals.append(DataField("Gear", 1, 0, 7, 1, 0, 5, packets[0x470], 0))

	vals.append(DataField("Limiter", 1, 0, 0, 2, 8000, 15000, packets[0x477], 0))



	while 1:
		for packet in packets.values():
			packet.send_packet()
			packet.clear_packet()
		for val in vals:
			val.refresh_value()
			val.encode()
		


if __name__ == "__main__":
	main()

