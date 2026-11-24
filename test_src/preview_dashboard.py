"""Send editable UI test values to fsdash.exe --preview-input."""

import argparse
import json
import os
from pathlib import Path
import socket
import subprocess
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


class PreviewClosed(Exception):
    pass


class Dashboard:
    def __init__(self, sender, preview=None):
        self.sender = sender
        self.preview = preview
        self.values = dict(
            rpm=1500, speed_mph=0, gear=0, battery_v=13.8,
            oil_temp_c=85, coolant_temp_c=80, oil_pressure_psi=35,
            fuel_pressure_psi=42, launch_control=False,
        )

    def set(self, **values):
        if self.preview is not None and self.preview.poll() is not None:
            raise PreviewClosed
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


def launch_demo(sender):
    repo = Path(__file__).resolve().parents[1]
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 1  # SW_SHOWNORMAL
    with subprocess.Popen(
        [str(repo / "fsdash.exe"), "--preview-input"],
        cwd=repo,
        stdout=subprocess.PIPE,
        text=True,
        creationflags=subprocess.CREATE_NO_WINDOW,
        startupinfo=startup,
    ) as preview:
        try:
            if not preview.stdout.readline().startswith("Preview input listening"):
                print("DashDisplay could not start preview input.")
                return 1
            print("Demo running. Close the dashboard window to stop.", flush=True)
            while preview.poll() is None:
                demo(Dashboard(sender, preview))
                time.sleep(1)
        except PreviewClosed:
            pass
        except KeyboardInterrupt:
            return 0
        finally:
            if preview.poll() is None:
                preview.terminate()
        return preview.returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--launch", action="store_true", help="launch DashDisplay and loop the demo")
    args = parser.parse_args()
    if args.launch and os.name != "nt":
        parser.error("--launch requires Windows")
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sender:
        if args.launch:
            return launch_demo(sender)
        demo(Dashboard(sender))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
