import socket
import struct
import time
from typing import Optional

import pygame

# =========================
# Constants (single block)
# =========================
MAGIC = 0x44524F4E
ESP_IP = "192.168.4.1"
ESP_PORT = 14550
SEND_HZ = 50
GAMEPAD_DEADZONE = 0.10
Z_STEP_KEY = 0.02
TAKEOFF_HOLD_S = 2.0
MAX_XY_YAW = 1.0
MIN_Z = 0.0
MAX_Z = 1.0

EVENT_NONE = 0
EVENT_INIT_COMPLETE = 1
EVENT_TESTS_PASS = 2
EVENT_ERROR = 3
EVENT_ARM_CMD = 4
EVENT_TAKEOFF_CMD = 5
EVENT_TOUCHDOWN = 6
EVENT_TARGET_ALT_REACHED = 7
EVENT_STABILIZE_CMD = 8
EVENT_ALT_HOLD_CMD = 9
EVENT_POS_HOLD_CMD = 10
EVENT_MISSION_CMD = 11
EVENT_ABORT_CMD = 12
EVENT_MISSION_COMPLETE = 13
EVENT_LAND_CMD = 14
EVENT_EXCEPTION = 15

EVENT_NAMES = {
    EVENT_NONE: "EV_NONE",
    EVENT_INIT_COMPLETE: "EV_INIT_COMPLETE",
    EVENT_TESTS_PASS: "EV_TESTS_PASS",
    EVENT_ERROR: "EV_ERROR",
    EVENT_ARM_CMD: "EV_ARM_CMD",
    EVENT_TAKEOFF_CMD: "EV_TAKEOFF_CMD",
    EVENT_TOUCHDOWN: "EV_TOUCHDOWN",
    EVENT_TARGET_ALT_REACHED: "EV_TARGET_ALT_REACHED",
    EVENT_STABILIZE_CMD: "EV_STABILIZE_CMD",
    EVENT_ALT_HOLD_CMD: "EV_ALT_HOLD_CMD",
    EVENT_POS_HOLD_CMD: "EV_POS_HOLD_CMD",
    EVENT_MISSION_CMD: "EV_MISSION_CMD",
    EVENT_ABORT_CMD: "EV_ABORT_CMD",
    EVENT_MISSION_COMPLETE: "EV_MISSION_COMPLETE",
    EVENT_LAND_CMD: "EV_LAND_CMD",
    EVENT_EXCEPTION: "EV_EXCEPTION",
}


def clamp(value: float, minimum: float, maximum: float) -> float:
    return max(minimum, min(maximum, value))


def apply_deadzone(value: float, deadzone: float) -> float:
    if abs(value) < deadzone:
        return 0.0
    sign = 1.0 if value >= 0 else -1.0
    scaled = (abs(value) - deadzone) / (1.0 - deadzone)
    return sign * clamp(scaled, 0.0, 1.0)


def crc16_ibm(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def build_control_packet(seq: int, x: float, y: float, z: float, yaw: float, mode: int) -> bytes:
    timestamp_ms = int(time.time() * 1000) & 0xFFFFFFFF
    packet_without_crc = struct.pack(
        "<I H I f f f f B",
        MAGIC,
        seq & 0xFFFF,
        timestamp_ms,
        x,
        y,
        z,
        yaw,
        mode & 0xFF,
    )
    packet_with_zero_crc = packet_without_crc + struct.pack("<H", 0)
    crc = crc16_ibm(packet_with_zero_crc)
    return packet_without_crc + struct.pack("<H", crc)


def init_joystick(joystick: Optional["pygame.joystick.Joystick"]) -> Optional["pygame.joystick.Joystick"]:
    if joystick is not None:
        try:
            if joystick.get_init() and joystick.get_attached():
                return joystick
        except Exception:
            pass
        joystick.quit()

    if pygame.joystick.get_count() <= 0:
        return None

    new_joystick = pygame.joystick.Joystick(0)
    new_joystick.init()
    print(f"Gamepad connected: {new_joystick.get_name()}")
    return new_joystick


def read_gamepad_axes(joystick: Optional["pygame.joystick.Joystick"]) -> dict:
    result = {
        "connected": False,
        "x": 0.0,
        "y": 0.0,
        "yaw": 0.0,
        "z_delta": 0.0,
        "x_active": False,
        "y_active": False,
        "yaw_active": False,
        "z_active": False,
        "lb": False,
        "rb": False,
    }

    if joystick is None or not joystick.get_init():
        return result

    result["connected"] = True

    num_axes = joystick.get_numaxes()
    num_buttons = joystick.get_numbuttons()

    # Left stick (X, Y)
    if num_axes >= 2:
        x_val = apply_deadzone(joystick.get_axis(0), GAMEPAD_DEADZONE)
        y_val = apply_deadzone(-joystick.get_axis(1), GAMEPAD_DEADZONE)
        result["x"] = x_val
        result["y"] = y_val
        result["x_active"] = abs(x_val) > 0.0
        result["y_active"] = abs(y_val) > 0.0

    # Right stick X (Yaw) - Xbox: Axis 3, fallback to Axis 2
    lt = 0.0
    rt = 0.0
    
    # Try Xbox layout first (LT=Axis 2, RightStickX=Axis 3, RT=Axis 4, RightStickY=Axis 5)
    if num_axes >= 4:
        yaw_val = apply_deadzone(joystick.get_axis(3), GAMEPAD_DEADZONE)
        result["yaw"] = yaw_val
        result["yaw_active"] = abs(yaw_val) > 0.0
    elif num_axes >= 3:
        # Fallback for other controller layouts
        yaw_val = apply_deadzone(joystick.get_axis(2), GAMEPAD_DEADZONE)
        result["yaw"] = yaw_val
        result["yaw_active"] = abs(yaw_val) > 0.0

    # Triggers (LT, RT) - Xbox: LT=Axis 2, RT=Axis 4
    if num_axes >= 5:
        lt = (joystick.get_axis(2) + 1.0) * 0.5  # LT on Xbox
        rt = (joystick.get_axis(4) + 1.0) * 0.5  # RT on Xbox
    elif num_axes >= 3:
        # Fallback: use buttons for triggers
        if num_buttons > 6:
            lt = 1.0 if joystick.get_button(6) else 0.0
        if num_buttons > 7:
            rt = 1.0 if joystick.get_button(7) else 0.0

    z_delta = (rt - lt) * Z_STEP_KEY
    result["z_delta"] = z_delta
    result["z_active"] = abs(z_delta) > 0.0

    if num_buttons > 4:
        result["lb"] = bool(joystick.get_button(4))
    if num_buttons > 5:
        result["rb"] = bool(joystick.get_button(5))

    return result


def main() -> None:
    esp = (ESP_IP, ESP_PORT)

    print("Starting Bodenstation...")
    print(f"Sending control packets to ESP at {ESP_IP}:{ESP_PORT}")
    print(
        "XBox: left stick x/y, right stick yaw, LT/RT z, special cmds with keys, "
        "takeoff with LB+RB hold 2s"
    )
    print("Keyboard: W/S=y A/D=x ArrowUp/Down=z ArrowLeft/Right=yaw")
    print("Special: R=arm/disarm event, L=land event")
    print("Press Ctrl+C to stop.")

    pygame.init()
    pygame.display.set_caption("Bodenstation Input Window")
    pygame.display.set_mode((360, 120))
    pygame.joystick.init()

    joystick = init_joystick(None)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # IPv4 UDP socket 

    seq = 0
    x = 0.0
    y = 0.0
    z = 0.0
    yaw = 0.0

    lb_rb_hold_started = None
    lb_rb_takeoff_sent = False

    last_event = EVENT_NONE
    packets_this_second = 0
    packets_per_second = 0
    second_start = time.monotonic()
    last_debug_print = 0.0

    period_s = 1.0 / SEND_HZ
    next_tick = time.monotonic()

    try:
        while True:
            now = time.monotonic()
            event_mode = EVENT_NONE

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    raise KeyboardInterrupt
                if event.type in (pygame.JOYDEVICEADDED, pygame.JOYDEVICEREMOVED):
                    joystick = init_joystick(joystick)
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_r:
                        event_mode = EVENT_ARM_CMD
                    elif event.key == pygame.K_l:
                        event_mode = EVENT_LAND_CMD

            keys = pygame.key.get_pressed()
            kbd_x = float(keys[pygame.K_d]) - float(keys[pygame.K_a])
            kbd_y = float(keys[pygame.K_w]) - float(keys[pygame.K_s])
            kbd_yaw = float(keys[pygame.K_RIGHT]) - float(keys[pygame.K_LEFT])
            kbd_z_delta = (Z_STEP_KEY if keys[pygame.K_UP] else 0.0) - (
                Z_STEP_KEY if keys[pygame.K_DOWN] else 0.0
            )

            gamepad = read_gamepad_axes(joystick)

            x = gamepad["x"] if gamepad["x_active"] else kbd_x
            y = gamepad["y"] if gamepad["y_active"] else kbd_y
            yaw = gamepad["yaw"] if gamepad["yaw_active"] else kbd_yaw
            z_delta = gamepad["z_delta"] if gamepad["z_active"] else kbd_z_delta

            x = clamp(x, -MAX_XY_YAW, MAX_XY_YAW)
            y = clamp(y, -MAX_XY_YAW, MAX_XY_YAW)
            yaw = clamp(yaw, -MAX_XY_YAW, MAX_XY_YAW)
            z = clamp(z + z_delta, MIN_Z, MAX_Z)

            if gamepad["connected"] and gamepad["lb"] and gamepad["rb"]:
                if lb_rb_hold_started is None:
                    lb_rb_hold_started = now
                    lb_rb_takeoff_sent = False
                elif not lb_rb_takeoff_sent and (now - lb_rb_hold_started) >= TAKEOFF_HOLD_S:
                    event_mode = EVENT_TAKEOFF_CMD
                    lb_rb_takeoff_sent = True
            else:
                lb_rb_hold_started = None
                lb_rb_takeoff_sent = False

            packet = build_control_packet(seq, x, y, z, yaw, event_mode)
            sock.sendto(packet, esp)
            seq = (seq + 1) & 0xFFFF

            if event_mode != EVENT_NONE:
                last_event = event_mode

            packets_this_second += 1
            if now - second_start >= 1.0:
                packets_per_second = packets_this_second
                packets_this_second = 0
                second_start = now

            if now - last_debug_print >= 0.2:
                hold_s = 0.0
                if lb_rb_hold_started is not None:
                    hold_s = min(now - lb_rb_hold_started, TAKEOFF_HOLD_S)
                source = "GAMEPAD" if gamepad["connected"] else "KEYBOARD"
                debug_line = (
                    f"seq={seq:5d} pps={packets_per_second:3d} src={source:<8} "
                    f"x={x:+.2f} y={y:+.2f} z={z:.2f} yaw={yaw:+.2f} "
                    f"evt={EVENT_NAMES.get(event_mode, str(event_mode)):<16} "
                    f"last={EVENT_NAMES.get(last_event, str(last_event)):<16} hold={hold_s:.1f}s"
                )
                print("\r" + debug_line.ljust(180), end="", flush=True)
                last_debug_print = now

            next_tick += period_s
            sleep_for = next_tick - time.monotonic()
            if sleep_for > 0:
                time.sleep(sleep_for)
            else:
                next_tick = time.monotonic()

    except KeyboardInterrupt:
        print("\nStopping Bodenstation...")
    finally:
        sock.close()
        pygame.quit()


if __name__ == "__main__":
    main()
