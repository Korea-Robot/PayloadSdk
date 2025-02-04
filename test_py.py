import time
import pypayload

def test_get_payload_stats():
    try:
        # Step 1: Create an instance of the payload SDK wrapper.
        sdk = pypayload.PyPayloadSDK()
        print("[INFO] SDK instance created.")

        # Step 2: Initialize the connection to the payload.
        sdk.initialize()
        print("[INFO] SDK connection initialized.")

        # Step 3: Wait a short time to let the payload gather initial data.
        time.sleep(1)  # Optional wait time to let the SDK receive telemetry.

        # Step 4: Fetch payload telemetry stats using the binding.
        while True:
            payload_stats = sdk.get_payload_stats()

            # Step 5: Display the telemetry data in a user-friendly way.
            print("\n[INFO] Payload telemetry stats retrieved:")
            for key, value in payload_stats.items():
                print(f"{key}: {value}")
            time.sleep(0.1)  # Optional wait time to let the SDK receive telemetry.


    except Exception as e:
        print(f"[ERROR] Exception occurred: {e}")

# Run the test
if __name__ == "__main__":
    print("[INFO] Starting payload SDK test.")
    test_get_payload_stats()
    print("[INFO] Test completed.")
