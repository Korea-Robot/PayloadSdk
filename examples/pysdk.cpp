#include <pybind11/pybind11.h>
#include "payloadSdkInterface.h"
#include <cstdio>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <signal.h>

namespace py = pybind11;

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

// Define a wrapper class for PayloadSdkInterface.
class PyPayloadSDK {
public:
    PyPayloadSDK() {
        std::cout << "[DEBUG] PyPayloadSDK constructor entered" << std::endl;
        // Create the underlying SDK object using the global connection settings.
        sdk = new PayloadSdkInterface(s_conn);
        std::cout << "[DEBUG] PayloadSdkInterface created" << std::endl;
        std::cout << "[DEBUG] PyPayloadSDK constructor exited" << std::endl;
    }

    ~PyPayloadSDK() {
        std::cout << "[DEBUG] PyPayloadSDK destructor entered" << std::endl;
        // Clean up the connection.
        sdk->sdkQuit();
        delete sdk;
        std::cout << "[DEBUG] PyPayloadSDK destructor exited" << std::endl;
    }

    // Initialize connection and check connection status.
    void initialize() {
        std::cout << "[DEBUG] PyPayloadSDK::initialize() entered" << std::endl;
        try {
            sdk->sdkInitConnection();
            std::cout << "[DEBUG] sdk->sdkInitConnection() called" << std::endl;
            sdk->checkPayloadConnection();
            std::cout << "[DEBUG] sdk->checkPayloadConnection() called" << std::endl;
        } catch (int error) {
            throw std::runtime_error("initialize() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::initialize() exited" << std::endl;
    }

    // Capture an image.
    void captureImage() {
        std::cout << "[DEBUG] PyPayloadSDK::captureImage() entered" << std::endl;
        try {
            // Ensure payload is in IMAGE mode.
            sdk->setPayloadCameraMode(CAMERA_MODE_IMAGE);
            std::cout << "[DEBUG] sdk->setPayloadCameraMode(CAMERA_MODE_IMAGE) called" << std::endl;
            sdk->setPayloadCameraCaptureImage();
            std::cout << "[DEBUG] sdk->setPayloadCameraCaptureImage() called" << std::endl;
        } catch (int error) {
            throw std::runtime_error("captureImage() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::captureImage() exited" << std::endl;
    }

    // Start video recording.
    void startRecordingVideo() {
        std::cout << "[DEBUG] PyPayloadSDK::startRecordingVideo() entered" << std::endl;
        try {
            // Ensure payload is in VIDEO mode.
            sdk->setPayloadCameraMode(CAMERA_MODE_VIDEO);
            std::cout << "[DEBUG] sdk->setPayloadCameraMode(CAMERA_MODE_VIDEO) called" << std::endl;
            sdk->setPayloadCameraRecordVideoStart();
            std::cout << "[DEBUG] sdk->setPayloadCameraRecordVideoStart() called" << std::endl;
        } catch (int error) {
            throw std::runtime_error("startRecordingVideo() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::startRecordingVideo() exited" << std::endl;
    }

    // Stop video recording.
    void stopRecordingVideo() {
        std::cout << "[DEBUG] PyPayloadSDK::stopRecordingVideo() entered" << std::endl;
        try {
            sdk->setPayloadCameraRecordVideoStop();
            std::cout << "[DEBUG] sdk->setPayloadCameraRecordVideoStop() called" << std::endl;
        } catch (int error) {
            throw std::runtime_error("stopRecordingVideo() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::stopRecordingVideo() exited" << std::endl;
    }

    // Send GPS data.
    void sendGPS(double lat, double lon, double alt, double heading) {
        std::cout << "[DEBUG] PyPayloadSDK::sendGPS() entered" << std::endl;
        try {
            mavlink_global_position_int_t gps;
            gps.time_boot_ms = 0; // The caller could pass system time if needed.
            gps.lat = lat * 1e7;    // Convert degrees to the integer value (per docs).
            gps.lon = lon * 1e7;
            gps.alt = alt * 1e3;    // e.g. alt in millimeters.
            gps.relative_alt = 0;
            gps.vx = 0;
            gps.vy = 0;
            gps.vz = 0;
            gps.hdg = heading;
            sdk->sendPayloadGPSPosition(gps);
            std::cout << "[DEBUG] sdk->sendPayloadGPSPosition() called" << std::endl;
        } catch (int error) {
            throw std::runtime_error("sendGPS() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::sendGPS() exited" << std::endl;
    }

    // Send system time.
    void sendSystemTime(long long unix_usec, int boot_ms) {
        std::cout << "[DEBUG] PyPayloadSDK::sendSystemTime() entered" << std::endl;
        try {
            mavlink_system_time_t sys_time;
            sys_time.time_boot_ms = boot_ms;
            sys_time.time_unix_usec = unix_usec;
            sdk->sendPayloadSystemTime(sys_time);
            std::cout << "[DEBUG] sdk->sendPayloadSystemTime() called" << std::endl;
        } catch (int error) {
            throw std::runtime_error("sendSystemTime() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::sendSystemTime() exited" << std::endl;
    }

    // Set camera zoom.
    void setCameraZoom(uint32_t zoomType, double zoomValue) {
        std::cout << "[DEBUG] PyPayloadSDK::setCameraZoom() entered" << std::endl;
        try {
            sdk->setCameraZoom(zoomType, zoomValue);
            std::cout << "[DEBUG] sdk->setCameraZoom() called" << std::endl;
        } catch (int error) {
            throw std::runtime_error("setCameraZoom() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::setCameraZoom() exited" << std::endl;
    }

    // Set gimbal angle (position control).
    // This method calls the SDK function with INPUT_ANGLE mode.
    void setGimbalAngle(double pitch, double roll, double yaw) {
        std::cout << "[DEBUG] PyPayloadSDK::setGimbalAngle() entered" << std::endl;
        try {
            sdk->setGimbalSpeed(pitch, roll, yaw, INPUT_ANGLE);
            std::cout << "[DEBUG] sdk->setGimbalSpeed() called with INPUT_ANGLE" << std::endl;
        } catch (int error) {
            throw std::runtime_error("setGimbalAngle() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::setGimbalAngle() exited" << std::endl;
    }

    // Set gimbal speed (rate control).
    // This method calls the SDK function with INPUT_SPEED mode.
    void setGimbalSpeed(double pitch, double roll, double yaw) {
        std::cout << "[DEBUG] PyPayloadSDK::setGimbalSpeed() entered" << std::endl;
        try {
            sdk->setGimbalSpeed(pitch, roll, yaw, INPUT_SPEED);
            std::cout << "[DEBUG] sdk->setGimbalSpeed() called with INPUT_SPEED" << std::endl;
        } catch (int error) {
            throw std::runtime_error("setGimbalSpeed() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::setGimbalSpeed() exited" << std::endl;
    }

    // Set payload camera parameter with detailed logging.
    void setPayloadCameraParam(const std::string& param_id, uint32_t param_value, uint8_t param_type) {
        std::cout << "[DEBUG] PyPayloadSDK::setPayloadCameraParam() entered" << std::endl;
        try {
            std::cout << "[DEBUG] setPayloadCameraParam called with param_id: " << param_id
                      << ", param_value: " << param_value
                      << ", param_type: " << static_cast<int>(param_type) << std::endl;
            std::cout << "[DEBUG] About to call sdk->setPayloadCameraParam" << std::endl;
            sdk->setPayloadCameraParam(const_cast<char*>(param_id.c_str()), param_value, param_type);
            std::cout << "[DEBUG] Completed call to sdk->setPayloadCameraParam" << std::endl;
        } catch (int error) {
            throw std::runtime_error("setPayloadCameraParam() error: " + std::to_string(error));
        }
        std::cout << "[DEBUG] PyPayloadSDK::setPayloadCameraParam() exited" << std::endl;
    }

private:
    PayloadSdkInterface* sdk;
};

PYBIND11_MODULE(pypayload, m) {
    m.doc() = "Python bindings for the PayloadSDK with modular abstraction";

    // Expose the PyPayloadSDK class and its methods.
    py::class_<PyPayloadSDK>(m, "PyPayloadSDK")
        .def(py::init<>())
        .def("initialize", &PyPayloadSDK::initialize, "Initialize the payload connection")
        .def("capture_image", &PyPayloadSDK::captureImage, "Capture an image")
        .def("start_recording_video", &PyPayloadSDK::startRecordingVideo, "Start video recording")
        .def("stop_recording_video", &PyPayloadSDK::stopRecordingVideo, "Stop video recording")
        .def("send_gps", &PyPayloadSDK::sendGPS,
             "Send GPS data",
             py::arg("lat"), py::arg("lon"), py::arg("alt"), py::arg("heading"))
        .def("send_system_time", &PyPayloadSDK::sendSystemTime,
             "Send system time",
             py::arg("unix_usec"), py::arg("boot_ms"))
        .def("set_camera_zoom", &PyPayloadSDK::setCameraZoom, "Set camera zoom",
             py::arg("zoomType"), py::arg("zoomValue"))
        .def("set_gimbal_angle", &PyPayloadSDK::setGimbalAngle, "Set gimbal angle (position control)",
             py::arg("pitch"), py::arg("roll"), py::arg("yaw"))
        .def("set_gimbal_speed", &PyPayloadSDK::setGimbalSpeed, "Set gimbal speed (rate control)",
             py::arg("pitch"), py::arg("roll"), py::arg("yaw"))
        .def("set_payload_camera_param", &PyPayloadSDK::setPayloadCameraParam,
             "Set payload camera parameter with detailed logging",
             py::arg("param_id"), py::arg("param_value"), py::arg("param_type"));
}
