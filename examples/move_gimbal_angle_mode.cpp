#include <stdio.h>
#include <pthread.h>
#include <cstdlib>
#include <string>
#include <chrono>    // For high resolution clock
#include <csignal>   // For signal()
using namespace std;

#include "payloadSdkInterface.h"

// Define connection info based on control method:
#if (CONTROL_METHOD == CONTROL_UART)
T_ConnInfo s_conn = {
    CONTROL_UART,
    payload_uart_port,
    payload_uart_baud
};
#else
T_ConnInfo s_conn = {
    CONTROL_UDP,
    udp_ip_target,
    udp_port_target
};
#endif

PayloadSdkInterface* my_payload = nullptr;

pthread_t thrd_recv;
pthread_t thrd_gstreamer;

bool gstreamer_start();
void gstreamer_terminate();
void* start_loop_thread(void* threadid);

bool all_threads_init();
void quit_handler(int sig);

// Helper template function to measure command latency:
template<typename Func, typename... Args>
void sendCommandWithLatency(const char* commandDescription, Func func, Args&&... args)
{
    // Record start time:
    auto start = std::chrono::steady_clock::now();

    // Call the provided function with forwarded arguments:
    func(std::forward<Args>(args)...);

    // Record end time:
    auto end = std::chrono::steady_clock::now();

    // Calculate elapsed time in milliseconds as a floating point value:
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    printf("%s latency: %.3f ms\n", commandDescription, elapsed);
}

int main(int argc, char* argv[]){
    printf("Starting Set gimbal mode example with latency measurements...\n");
    signal(SIGINT, quit_handler);

    // Create payload SDK object
    my_payload = new PayloadSdkInterface(s_conn);

    // Initialize payload
    my_payload->sdkInitConnection();
    printf("Waiting for payload signal! \n");

    my_payload->checkPayloadConnection();
    usleep(100000);

    // Set gimbal RC mode with latency measurement:
    printf("Set gimbal RC mode\n");
    sendCommandWithLatency("setPayloadCameraParam",
                           [=]() {
                               my_payload->setPayloadCameraParam(PAYLOAD_CAMERA_RC_MODE,
                                                                 PAYLOAD_CAMERA_RC_MODE_STANDARD,
                                                                 PARAM_TYPE_UINT32);
                           });
    usleep(100000);

    // Move gimbal yaw to 90 degrees:
    printf("Move gimbal yaw to 90 deg, delay 5 secs\n");
    sendCommandWithLatency("setGimbalSpeed",
                           [=]() {
                               my_payload->setGimbalSpeed(0, 0, 90, INPUT_ANGLE);
                           });
    usleep(5000000);

    // Move gimbal yaw to -90 degrees:
    printf("Move gimbal yaw to -90 deg, delay 5 secs\n");
    sendCommandWithLatency("setGimbalSpeed",
                           [=]() {
                               my_payload->setGimbalSpeed(0, 0, -90, INPUT_ANGLE);
                           });
    usleep(5000000);

    // Move gimbal yaw to 0 degrees:
    printf("Move gimbal yaw to 0 deg, short delay\n");
    sendCommandWithLatency("setGimbalSpeed",
                           [=]() {
                               my_payload->setGimbalSpeed(0, 0, 0, INPUT_ANGLE);
                           });
    usleep(500000);

    // Close payload interface
    try {
        my_payload->sdkQuit();
    }
    catch (int error){
        // Error handling here if needed.
    }

    return 0;
}

void quit_handler(int sig){
    printf("\nTERMINATING AT USER REQUEST\n\n");

    // Close payload interface
    try {
        my_payload->sdkQuit();
    }
    catch (int error){
        // Error handling here if needed.
    }

    // End program
    exit(0);
}
