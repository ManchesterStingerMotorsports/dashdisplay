"""Send editable UI test values to fsdash.exe --preview-input."""

import json
import socket
import time


ADDRESS = ("127.0.0.1", 8765)

# The key is what you pass to dash.set(). The title matches the dashboard field.
FIELDS = {
    "rpm": "RPM",
    "speed_mph": "Speed",
    "gear": "Gear",
    "battery_v": "Battery Volts",
    "oil_temp_c": "Oil Temp.",
    "coolant_temp_c": "Coolant Temp.",
    "oil_pressure_psi": "Oil Pres.",
    "fuel_pressure_psi": "Fuel Pres.",
    "launch_control": "Launch Control Active",
}


class Dashboard:
    def __init__(self, sender):
        self.sender = sender
        self.values = dict(
            rpm=1500, speed_mph=0, gear=0, battery_v=13.8,
            oil_temp_c=85, coolant_temp_c=80, oil_pressure_psi=35,
            fuel_pressure_psi=42, launch_control=False,
        )

    def set(self, **values):
        snapshot = {**self.values, **values}
        packet = "\n".join(
            f"{json.dumps(FIELDS[name])} {float(value)}"
            for name, value in snapshot.items()
        )
        self.sender.sendto(packet.encode("ascii"), ADDRESS)
        self.values = snapshot


def demo(dash):
    # Edit these calls and pauses to test a different sequence. Gear 0 is neutral.
    dash.set()
    time.sleep(1)

    for gear in range(1, 6):
        dash.set(gear=gear, speed_mph=gear * 12)
        for rpm in range(4000, 12501, 500):
            dash.set(rpm=rpm)
            time.sleep(0.06)

    dash.set(rpm=6500, speed_mph=0, gear=1, launch_control=True)
    time.sleep(2)
    dash.set(coolant_temp_c=87)  # Amber with LC still active.
    time.sleep(2)
    dash.set(coolant_temp_c=96)  # HOT takes priority over LC.
    time.sleep(2)
    dash.set(coolant_temp_c=80)  # LC returns.
    time.sleep(2)
    dash.set(rpm=1500, gear=0, launch_control=False)


if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sender:
        demo(Dashboard(sender))
