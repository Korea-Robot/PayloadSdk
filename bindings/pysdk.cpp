#include <pybind11/pybind11.h>
#include "payloadSdkInterface.h"
#include <chrono>
#include <thread>
#include <stdexcept>
#include <signal.h>
#define FMT_HEADER_ONLY
#include <spdlog/spdlog.h>
#include "vio_sdk.h"
#include <mutex>
#include <condition_variable>
#include <sstream>       // Added for string formatting
#include <iomanip>       // Added for i/o manipulators
#include <map>
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
        sdk = std::make_shared<PayloadSdkInterface>(s_conn);
               // Set the global instance pointer (for the callback forwarding).
        s_instance = this;
        sdk->regPayloadStatusChanged(&PyPayloadSDK::sdkStatusCallback);

        spdlog::debug("PayloadSdkInterface created");
        spdlog::debug("PyPayloadSDK constructor exited");
    }

    ~PyPayloadSDK() = default;
    void initialize() {
        spdlog::debug("PyPayloadSDK::initialize() entered");
        sdk->sdkInitConnection();
        sdk->setParamRate(PARAM_EO_ZOOM_LEVEL, 1000);
        sdk->setParamRate(PARAM_IR_ZOOM_LEVEL, 1000);
        sdk->setParamRate(PARAM_LRF_RANGE, 1000);
        sdk->setParamRate(PARAM_LRF_OFSET_X, 1000);
        sdk->setParamRate(PARAM_LRF_OFSET_Y, 1000);
        sdk->setParamRate(PARAM_TARGET_COOR_LON, 2000);
        sdk->setParamRate(PARAM_TARGET_COOR_LAT, 2000);
        sdk->setParamRate(PARAM_TARGET_COOR_ALT, 2000);
        spdlog::debug("sdk->sdkInitConnection() called");
        spdlog::debug("PyPayloadSDK::initialize() exited");
    }

    // Send GPS data.
    void sendPayloadGPS(double lat, double lon, double alt, double heading) {
        spdlog::debug("PyPayloadSDK::sendPayloadGPS() entered");
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
            spdlog::error("sendPayloadGPS() error: {}", error);
            throw std::runtime_error("sendPayloadGPS() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::sendPayloadGPS() exited");
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
    void setCameraZoom(uint32_t zoomType, int zoomValue) {
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
    void setOSDMode(uint32_t mode) {
        spdlog::debug("PyPayloadSDK::setOSDMode() entered");
        sdk->setPayloadCameraParam(PAYLOAD_CAMERA_VIDEO_OSD_MODE, mode, PARAM_TYPE_UINT32);
        spdlog::debug("sdk->setPayloadCameraParam() called");
        spdlog::debug("PyPayloadSDK::setOSDMode() exited");
    }

    // Set gimbal angle (position control).
    // This method calls the SDK function with INPUT_ANGLE mode.
    void setGimbalAngle(double pitch, double roll, double yaw) {
        spdlog::debug("PyPayloadSDK::setGimbalAngle() entered");
        try {
            sdk->setGimbalSpeed(pitch, roll, yaw, INPUT_ANGLE);
            spdlog::debug("sdk->setGimbalAngle() called with INPUT_ANGLE");
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
            spdlog::debug("sdk->setGimbal Speed() called with INPUT_SPEED");
        } catch (int error) {
            spdlog::error("setGimbalSpeed() error: {}", error);
            throw std::runtime_error("setGimbalSpeed() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setGimbalSpeed() exited");
    }
    void setGimbalMode(uint32_t mode) {
        spdlog::debug("PyPayloadSDK::setGimbalMode() entered");
        try {
            sdk->setPayloadCameraParam(PAYLOAD_CAMERA_GIMBAL_MODE, mode, PARAM_TYPE_UINT32);
            spdlog::debug("sdk->setPayloadCameraParam() called");
        } catch (int error) {
            spdlog::error("setGimbalMode() error: {}", error);
            throw std::runtime_error("setGimbalMode() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setGimbalMode() exited");
    }
    // Set payload camera parameter with detailed logging.
    void setChangeViewSource(uint32_t view_source) {
        spdlog::debug("PyPayloadSDK::setChangeViewSource() entered");
        try {
            sdk->setPayloadCameraParam(PAYLOAD_CAMERA_VIEW_SRC, view_source, PARAM_TYPE_UINT32);
            spdlog::debug("sdk->setPayloadCameraParam() called");
        } catch (int error) {
            spdlog::error("setChangeViewSource() error: {}", error);
            throw std::runtime_error("setChangeViewSource() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setChangeViewSource() exited");
    }
    void setPayloadCameraParam(const std::string& param_id, uint32_t param_value, uint8_t param_type) {
        spdlog::debug("PyPayloadSDK::setPayloadCameraParam() entered");
        try {
            spdlog::debug("setPayloadCameraParam called with param_id: {}, param_value: {}, param_type: {}",
                          param_id, param_value, static_cast<int>(param_type));
            spdlog::debug("About to call sdk->setPayloadCameraParam");
            sdk->setPayloadCameraParam(const_cast<char*>(param_id.c_str()), param_value, PARAM_TYPE_UINT32);
            spdlog::debug("Completed call to sdk->setPayloadCameraParam");
        } catch (int error) {
            spdlog::error("setPayloadCameraParam() error: {}", error);
            throw std::runtime_error("setPayloadCameraParam() error: " + std::to_string(error));
        }
        spdlog::debug("PyPayloadSDK::setPayloadCameraParam() exited");
    }

    py::dict getPayloadStats() {
        std::lock_guard<std::mutex> lock(stats_mutex);
        py::dict result;
        for (const auto& kv : stats) {
            result[py::str(kv.first)] = kv.second;
        }
        return result;
    }

private:
    std::shared_ptr<PayloadSdkInterface> sdk;
    std::mutex stats_mutex;
    std::map<std::string, double> stats;

    // We use a static pointer so that our static callback can forward to the proper instance.
    // (This simple approach assumes only one instance of PyPayloadSDK is active.)
    static PyPayloadSDK* s_instance;

    // This function will be called (via the static callback) when the SDK has a status update.
    void handleStatusChange(int event, double* param) {
    std::lock_guard<std::mutex> lock(stats_mutex);
    if (event == PAYLOAD_GB_ATTITUDE) {
        // attitude 데이터: param[0]=pitch, param[1]=roll, param[2]=yaw.
        stats["pitch"] = param[0];
        stats["roll"]  = param[1];
        stats["yaw"]   = param[2];
    } else if (event == PAYLOAD_PARAMS) {
        // 파라미터 업데이트의 경우:
        // param[0]는 파라미터 ID, param[1]은 해당 값입니다.
        static const std::map<int, std::string> paramNames = {
            { PARAM_EO_ZOOM_LEVEL,    "eo_zoom_level" },
            { PARAM_IR_ZOOM_LEVEL,    "ir_zoom_level" },
            { PARAM_LRF_RANGE,        "lrf_range" },
            { PARAM_TRACK_POS_X,      "track_pos_x" },
            { PARAM_TRACK_POS_Y,      "track_pos_y" },
            { PARAM_TRACK_STATUS,     "track_status" },
            { PARAM_LRF_OFSET_X,      "lrf_offset_x" },
            { PARAM_LRF_OFSET_Y,      "lrf_offset_y" },
            { PARAM_TARGET_COOR_LON,  "target_coor_lon" },
            { PARAM_TARGET_COOR_LAT,  "target_coor_lat" },
            { PARAM_TARGET_COOR_ALT,  "target_coor_alt" }
        };

        int id = static_cast<int>(param[0]);
        std::string key;
        auto it = paramNames.find(id);
        if (it != paramNames.end()) {
            key = it->second;
        } else {
            key = "param_" + std::to_string(id);
        }
        stats[key] = param[1];
    }
}

    // Static callback function that matches the SDK callback signature.
    // It simply forwards the call to the current instance.
    static void sdkStatusCallback(int event, double* param) {
        if (s_instance) {
            s_instance->handleStatusChange(event, param);
        }
    }
};

// Define the static instance pointer.
PyPayloadSDK* PyPayloadSDK::s_instance = nullptr;
PYBIND11_MODULE(pypayload, m) {
    m.doc() = "Python bindings for the PayloadSDK with modular abstraction";

    // Bind using shared_ptr as the holder type.
    py::class_<PyPayloadSDK, std::shared_ptr<PyPayloadSDK>>(m, "PyPayloadSDK")
        .def(py::init<>())
        .def("initialize", &PyPayloadSDK::initialize, "Initialize the payload connection")
        .def("send_payload_gps", &PyPayloadSDK::sendPayloadGPS,
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
        .def("set_gimbal_mode", &PyPayloadSDK::setGimbalMode, "Set gimbal mode",
             py::arg("mode"))
        .def("set_change_view_source", &PyPayloadSDK::setChangeViewSource, "Set change view source",
             py::arg("view_source"))
        .def("set_osd_mode", &PyPayloadSDK::setOSDMode, "Set OSD mode",
             py::arg("mode"))
        .def("set_payload_camera_param", &PyPayloadSDK::setPayloadCameraParam,
             "Set payload camera parameter with detailed logging",
             py::arg("param_id"), py::arg("param_value"), py::arg("param_type"))
        .def("get_payload_stats", &PyPayloadSDK::getPayloadStats, "Retrieve the current payload statistics");
}
