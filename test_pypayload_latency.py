import pypayload
import time

def send_command_with_latency(command_func, *args, **kwargs):
    """
    Wrapper function that sends a command using command_func with the provided arguments,
    and measures the time taken to execute the command function.

    Returns the result of command_func.
    """
    start_time = time.time()
    result = command_func(*args, **kwargs)
    end_time = time.time()

    latency_ms = (end_time - start_time) * 1000  # Convert seconds to milliseconds
    print(f"Command latency: {latency_ms:.2f} ms")

    return result

# Create and initialize the SDK
sdk = pypayload.PyPayloadSDK()
sdk.initialize()

# (Optional) Set RC mode if needed:
# sdk.set_payload_camera_param("PAYLOAD_CAMERA_RC_MODE", PAYLOAD_CAMERA_RC_MODE_STANDARD, PARAM_TYPE_UINT32)

# Control gimbal with absolute angles:
print("Moving gimbal to 90 degrees yaw...")
send_command_with_latency(sdk.set_gimbal_angle, 0, 0, 90)
time.sleep(5)

print("Moving gimbal to -90 degrees yaw...")
send_command_with_latency(sdk.set_gimbal_angle, 0, 0, -90)
time.sleep(5)

print("Resetting gimbal to 0 degrees yaw...")
send_command_with_latency(sdk.set_gimbal_angle, 0, 0, 0)

# Alternatively, use speed (rate control):
print("Moving gimbal right at 20 deg/s...")
send_command_with_latency(sdk.set_gimbal_speed, 0, 0, 20)
time.sleep(5)

print("Moving gimbal left at 20 deg/s...")
send_command_with_latency(sdk.set_gimbal_speed, 0, 0, -20)
time.sleep(5)

print("Stopping gimbal movement...")
send_command_with_latency(sdk.set_gimbal_speed, 0, 0, 0)
