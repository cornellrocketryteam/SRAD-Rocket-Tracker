#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"

#include "btstack_config.h"
#include "btstack.h"
#include "SRAD_RT_db.h"
#include <cmath>

// Hardware API's
#include "hardware/sync.h"
#include "pico/time.h"

// GAP and GATT
#include "GAP_Advertisement/gap_config.h"
#include "GATT_Service/service_implementation.h"

// Radio Parsing
#include "pins.hpp"
#include "telemetry.hpp"
#include "radio.hpp"
#include "tusb.h"

// BTstack objects
static btstack_packet_callback_registration_t hci_event_callback_registration;

// Buffers for sending the values as byte arrays
char lat_bytes[64]; 
char long_bytes[64];
char PT3_bytes[64]; 
char PT4_bytes[64]; 
char MAV_bytes[64]; 
char SV_bytes[64];
char FM_bytes[64];
char PT_bytes[64];
bool recived_first = false;

bool getBit(uint16_t metadata, int position) // position in range 0-15
{
    return (metadata >> position) & 0x1;
}

int getFM(uint16_t metadata) // position in range 0-15
{
    return (metadata >> 13) & 0x07; // 000 - Startup, 001 - Standbye, 010 - Ascent, 011 Drogue Deployed, 100 - Main Deployed, 101 - Fault - Finalized 13-15 bits = FM
}

bool compare_float(float x, float y, float epsilon = 0.001f){
    if(fabs(x - y) <= epsilon)
       return true; //they are same
    return false; //they are not same
 }


int main() {
    stdio_init_all();

    // while (!tud_cdc_connected()) // Comment out for final
    // {
    //     sleep_ms(500);
    // }
    // printf("Connected to computer\n");

    Radio radio;
    if (!radio.start())
    {
        printf("Radio failed to start\n");
        // TODO: report radio failing
        return 1;
    }

    sleep_ms(10000);
    // Initialise the Wi-Fi/BLE chip
    if (cyw43_arch_init()) {
        printf("Bluetooth init failed\n");
        return -1;
    }

    // Initialize L2CAP and security manager
    l2cap_init();
    sm_init();
    // Initialize ATT server, no general read/write callbacks because we set one up for each service
    att_server_init(profile_data, NULL, NULL);
    // Instantiate our custom service handler
    custom_service_server_init(lat_bytes, long_bytes, PT3_bytes, PT4_bytes, MAV_bytes, SV_bytes, FM_bytes, PT_bytes);
    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    // register for ATT event
    att_server_register_packet_handler(packet_handler);
    // turn on bluetooth and LED
    hci_power_control(HCI_POWER_ON);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    
    // define starting blank values
    int32_t lat_val = 42454985; // in micro degrees 10^-6
    int32_t long_val = -76477751;
    float PT3 = 0.00000f; // PT3 and 4 are to the 5th decimal point
    float PT4 = 0.00000f;
    bool MAV = 0;
    bool SV = 0;
    int FM = 0;
    int MAVtemp = -1;
    int SVtemp = -1;
    int FMtemp = -1;
    int PT = time_us_64()/1000000.0;
    int time_recived= time_us_64();


    printf("Should have intialized\n");
    set_latitude_value(&lat_val);
    set_longitude_value(&long_val);
    set_PT3_value(&PT3);
    set_PT4_value(&PT4);
    set_MAV_value(&MAV);
    set_SV_value(&SV);
    set_FM_value(&FM);
    set_PT_value(&FM); //Just setting to a 0 value
    sleep_ms(10000);
    int Old=-1;
    while (true) {
        std::vector<Telemetry> telemetry_packets;
        bool success = radio.read(telemetry_packets);
        if (success) {
            recived_first = true;
            printf("Success\n");
            time_recived=time_us_64();
            for (Telemetry &telemetry : telemetry_packets)
            {
                FMtemp = getFM(telemetry.metadata);
                MAVtemp = getBit(telemetry.metadata, 11);
                SVtemp = getBit(telemetry.metadata, 12);
                if (MAVtemp != MAV)
                {
                    MAV=MAVtemp;
                    set_MAV_value(&MAV);
                }
                if (SVtemp != SV)
                {
                    SV=SVtemp;
                    set_SV_value(&SV);
                }
                if (FMtemp != FM)
                {
                    FM=FMtemp;
                    set_FM_value(&FM);
                }
                if (!compare_float(telemetry.pressure_pt3, PT3))
                {
                    PT3=telemetry.pressure_pt3;
                    set_PT3_value(&PT3);
                }
                if (telemetry.gps_latitude!=lat_val)
                {
                    lat_val=telemetry.gps_latitude;
                    set_latitude_value(&lat_val);
                }
                if (telemetry.gps_longitude!=long_val)
                {
                    long_val=telemetry.gps_longitude;
                    set_longitude_value(&long_val);
                }
                PT=0;
                set_PT_value(&PT);
                printf("Lat: %d Long: %d PT3: %.3f PT4: %.3f MAV: %d SV: %d FM: %d PT: %d\n",telemetry.gps_latitude, telemetry.gps_longitude, telemetry.pressure_pt3, telemetry.pressure_pt4, MAV, SV, FM, PT);
            }
        }
        if (PT>Old && recived_first) {
            PT=absolute_time_diff_us(time_recived,time_us_64())/1000000.0;
            set_PT_value(&PT);
            Old=PT;
        }
        else if (!recived_first)
        {
            PT=0;
            set_PT_value(&PT);
        }
    }
}
