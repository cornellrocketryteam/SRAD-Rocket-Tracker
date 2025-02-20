/**
 * @file telemetry.hpp
 * @author sj728
 *
 * @brief Definitions for telemetry data. The data fields should
 * match the fields in the telemetry data packet sent from FSW
 */

#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

#include <cstdint>

// Use a packing directive to ensure that the compiler does not insert any padding
#pragma pack(push, 1)
struct Telemetry
{
    uint16_t metadata;
    uint32_t timestamp;
    uint32_t events;
    float altitude;
    int32_t gps_latitude;
    int32_t gps_longitude;
    uint8_t gps_num_satellites;
    uint32_t gps_utc_time;
    float imu_accel_x;
    float imu_accel_y;
    float imu_accel_z;
    float imu_gyro_x;
    float imu_gyro_y;
    float imu_gyro_z;
    float imu_orientation_x;
    float imu_orientation_y;
    float imu_orientation_z;
    float imu_gravity_x;
    float imu_gravity_y;
    float imu_gravity_z;
    float accel_x;
    float accel_y;
    float accel_z;
    float alt_temp;
    float voltage;
    float pressure_pt3;
    float pressure_pt4;
    float motor_position;
};
#pragma pack(pop)

#endif // TELEMETRY_HPP
