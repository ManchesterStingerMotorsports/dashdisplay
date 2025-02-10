import random
import math
import sys
import os


class DataField:
	def __init__(self, field_name, packet, sp, ep, gain, offset, lower_bound, upper_bound, variation_method):
		self.field_name = field_name
		self.packet = packet
		self.sp = sp
		self.ep = ep
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
		return (0.5*(self.upp_b - self.low_b)) * (1 + math.sin(self.changes*(math.pi/180)))

	def randomise(self):
		return self.low_b + random.random()*(self.upp_b - self.low_b)

	def linear(self):
		return self.low_b + (self.changes % (self.upp_b - self.low_b))

	def fixed(self):
		return self.value 

	def encode(self):
		raw_val = int((self.value - self.offset) // self.gain)
		max_b = self.ep - self.sp + 1
		if raw_val > (16**(2*max_b) - 1):
			print("Value will not fit in allocated space in the packet!")
		else:
			self.packet.data |= (raw_val << self.sp*8)




		

########### Other Methods ############

	def __str__(self):
		return f"{self.field_name:<20}:{self.value:>9}"

	
class CAN_Packet:
	def __init__(self, p_id):
		self.p_id = p_id
		self.data = 0

	def send_packet(self):
		#print(f"./canusb -d /dev/ttyUSB0 -s 1000000 -n 1 -i {self.p_id:0{3}x} -j {self.data:0{16}x}")
		os.system(f"./canusb -d /dev/ttyUSB0 -s 1000000 -n 1 -i {self.p_id:0{3}x} -j {self.data:0{16}x}")

	def clear_packet(self):
		self.data = 0


def main():
	packets = {}

	packets[0x360] = CAN_Packet(0x360)
	packets[0x3E0] = CAN_Packet(0x3E0)
	

	vals = []

	vals.append(DataField("RPM", packets[0x360], 0, 1, 1, 0, 0, 13000, 1))
	vals.append(DataField("Manifold Pressure", packets[0x360], 2, 3, 0.1, 0, 0, 10, 0))
	vals.append(DataField("Throttle Position", packets[0x360], 4, 5, 0.1, 0, 0, 100, 2))

	vals.append(DataField("Coolant Temperature", packets[0x3E0], 0, 1, 0.1, 0, 0, 120, 0))
	vals.append(DataField("Air Temperature", packets[0x3E0], 2, 3, 0.1, 0, 0, 50, 0))
	vals.append(DataField("Oil Temperature", packets[0x3E0], 6, 7, 0.1, 0, 0, 100, 0))


	for i in range(100):
		for packet in packets.values():
			packet.send_packet()
			packet.clear_packet()
		for val in vals:
			val.refresh_value()
			val.encode()
		


if __name__ == "__main__":
	main()

