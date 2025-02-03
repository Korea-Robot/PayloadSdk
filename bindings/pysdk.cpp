#include <pybind11/pybind11.h>
#include "payloadSdkInterface.h"
#include <chrono>
#include <thread>
#include <stdexcept>
#include <signal.h>
#define FMT_HEADER_ONLY
#include <spdlog/spdlog.h>

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
        spdlog::debug("PyPayloadSDK constructor entered");
        // Create the underlying SDK object using the global connection settings.
        sdk = new PayloadSdkInterface(s_conn);
        spdlog::debug("PayloadSdkInterface created");
        spdlog::debug("PyPayloadSDK constructor exited");
    }

    ~PyPayloadSDK() {
        spdlog::debug("PyPayloadSDK destructor entered");
        // Clean up the connection.
        sdk->sdkQuit();
        delete sdk;
        spdlog::debug("PyPayloadSDK destructor exited");
    }

    // Initialize connection and check connection status.
    void initialize() {
        spdlog::debug("PyPayloadSDK::initialize() entered");
        try {
            sdk->sdkInitConnection();
            spdlog::debug("sdk->sdkInitConnection() called");
            sdk->checkPayloadConnection();
            spdlog::debug("sdk->checkPayloadConnection() called");
        } catch (int error) {
            spdlog::error("initialize() error: {}", error);
            throw std::runtime_error("initialize() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::initialize() exited");
    }

    // Capture an image.
    void captureImage() {
        spdlog::debug("PyPayloadSDK::captureImage() entered");
        try {
            // Ensure payload is in IMAGE mode.
            sdk->setPayloadCameraMode(CAMERA_MODE_IMAGE);
            spdlog::debug("sdk->setPayloadCameraMode(CAMERA_MODE_IMAGE) called");
            sdk->setPayloadCameraCaptureImage();
            spdlog::debug("sdk->setPayloadCameraCaptureImage() called");
        } catch (int error) {
            spdlog::error("captureImage() error: {}", error);
            throw std::runtime_error("captureImage() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::captureImage() exited");
    }

    // Start video recording.
    void startRecordingVideo() {
        spdlog::debug("PyPayloadSDK::startRecordingVideo() entered");
        try {
            // Ensure payload is in VIDEO mode.
            sdk->setPayloadCameraMode(CAMERA_MODE_VIDEO);
            spdlog::debug("sdk->setPayloadCameraMode(CAMERA_MODE_VIDEO) called");
            sdk->setPayloadCameraRecordVideoStart();
            spdlog::debug("sdk->setPayloadCameraRecordVideoStart() called");
        } catch (int error) {
            spdlog::error("startRecordingVideo() error: {}", error);
            throw std::runtime_error("startRecordingVideo() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::startRecordingVideo() exited");
    }

    // Stop video recording.
    void stopRecordingVideo() {
        spdlog::debug("PyPayloadSDK::stopRecordingVideo() entered");
        try {
            sdk->setPayloadCameraRecordVideoStop();
            spdlog::debug("sdk->setPayloadCameraRecordVideoStop() called");
        } catch (int error) {
            spdlog::error("stopRecordingVideo() error: {}", error);
            throw std::runtime_error("stopRecordingVideo() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::stopRecordingVideo() exited");
    }

    // Send GPS data.
    void sendGPS(double lat, double lon, double alt, double heading) {
        spdlog::debug("PyPayloadSDK::sendGPS() entered");
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
            spdlog::debug("sdk->sendPayloadGPSPosition() called");
        } catch (int error) {
            spdlog::error("sendGPS() error: {}", error);
            throw std::runtime_error("sendGPS() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::sendGPS() exited");
    }

    // Send system time.
    void sendSystemTime(long long unix_usec, int boot_ms) {
        spdlog::debug("PyPayloadSDK::sendSystemTime() entered");
        try {
            mavlink_system_time_t sys_time;
            sys_time.time_boot_ms = boot_ms;
            sys_time.time_unix_usec = unix_usec;
            sdk->sendPayloadSystemTime(sys_time);
            spdlog::debug("sdk->sendPayloadSystemTime() called");
        } catch (int error) {
            spdlog::error("sendSystemTime() error: {}", error);
            throw std::runtime_error("sendSystemTime() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::sendSystemTime() exited");
    }

    // Set camera zoom.
    void setCameraZoom(uint32_t zoomType, double zoomValue) {
        spdlog::debug("PyPayloadSDK::setCameraZoom() entered");
        try {
            sdk->setCameraZoom(zoomType, zoomValue);
            spdlog::debug("sdk->setCameraZoom() called");
        } catch (int error) {
            spdlog::error("setCameraZoom() error: {}", error);
            throw std::runtime_error("setCameraZoom() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setCameraZoom() exited");
    }

    // Set gimbal angle (position control).
    // This method calls the SDK function with INPUT_ANGLE mode.
    void setGimbalAngle(double pitch, double roll, double yaw) {
        spdlog::debug("PyPayloadSDK::setGimbalAngle() entered");
        try {
            sdk->setGimbalSpeed(pitch, roll, yaw, INPUT_ANGLE);
            spdlog::debug("sdk->setGimbalSpeed() called with INPUT_ANGLE");
        } catch (int error) {
            spdlog::error("setGimbalAngle() error: {}", error);
            throw std::runtime_error("setGimbalAngle() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setGimbalAngle() exited");
    }

    // Set gimbal speed (rate control).
    // This method calls the SDK function with INPUT_SPEED mode.
    void setGimbalSpeed(double pitch, double roll, double yaw) {
        spdlog::debug("PyPayloadSDK::setGimbalSpeed() entered");
        try {
            sdk->setGimbalSpeed(pitch, roll, yaw, INPUT_SPEED);
            spdlog::debug("sdk->setGimbalSpeed() called with INPUT_SPEED");
        } catch (int error) {
            spdlog::error("setGimbalSpeed() error: {}", error);
            throw std::runtime_error("setGimbalSpeed() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setGimbalSpeed() exited");
    }

    // Set payload camera parameter with detailed logging.
    void setPayloadCameraParam(const std::string& param_id, uint32_t param_value, uint8_t param_type) {
        spdlog::debug("PyPayloadSDK::setPayloadCameraParam() entered");
        try {
            spdlog::debug("setPayloadCameraParam called with param_id: {}, param_value: {}, param_type: {}",
                          param_id, param_value, static_cast<int>(param_type));
            spdlog::debug("About to call sdk->setPayloadCameraParam");
            sdk->setPayloadCameraParam(const_cast<char*>(param_id.c_str()), param_value, param_type);
            spdlog::debug("Completed call to sdk->setPayloadCameraParam");
        } catch (int error) {
            spdlog::error("setPayloadCameraParam() error: {}", error);
            throw std::runtime_error("setPayloadCameraParam() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setPayloadCameraParam() exited");
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
