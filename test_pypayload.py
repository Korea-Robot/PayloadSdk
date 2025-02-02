import pypayload
import time

# Create and initialize the SDK
sdk = pypayload.PyPayloadSDK()
sdk.initialize()

# (Optional) Set RC mode if needed:
# sdk.set_payload_camera_param("PAYLOAD_CAMERA_RC_MODE", PAYLOAD_CAMERA_RC_MODE_STANDARD, PARAM_TYPE_UINT32)

# Control gimbal with absolute angles:
print("Moving gimbal to 90 degrees yaw...")
sdk.set_gimbal_angle(0, 0, 90)
time.sleep(5)

print("Moving gimbal to -90 degrees yaw...")
sdk.set_gimbal_angle(0, 0, -90)
time.sleep(5)

print("Resetting gimbal to 0 degrees yaw...")
sdk.set_gimbal_angle(0, 0, 0)

# Alternatively, use speed (rate control):
print("Moving gimbal right at 20 deg/s...")
sdk.set_gimbal_speed(0, 0, 20)
time.sleep(5)

print("Moving gimbal left at 20 deg/s...")
sdk.set_gimbal_speed(0, 0, -20)
time.sleep(5)

print("Stopping gimbal movement...")
sdk.set_gimbal_speed(0, 0, 0)
