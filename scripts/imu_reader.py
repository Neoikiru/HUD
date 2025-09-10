# /scripts/imu_reader.py

import time
import signal
import sys
import board
import busio
from adafruit_bno08x import BNO_REPORT_ROTATION_VECTOR
from adafruit_bno08x.i2c import BNO08X_I2C

signal.signal(signal.SIGPIPE, signal.SIG_DFL)

i2c = busio.I2C(board.SCL, board.SDA)
bno = BNO08X_I2C(i2c, address=0x4B)

bno.enable_feature(BNO_REPORT_ROTATION_VECTOR)

while True:
    try:
        quat_i, quat_j, quat_k, quat_real = bno.quaternion
        print(f"X:{quat_i:.3f} Y:{quat_j:.3f} Z:{quat_k:.3f} W:{quat_real:.3f}", flush=True)
        time.sleep(0.05)
    except BrokenPipeError:
        # The C++ parent process has closed.
        sys.exit(0)
    except Exception as e:
        time.sleep(0.5)

