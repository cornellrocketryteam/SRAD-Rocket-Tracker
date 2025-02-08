#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "stdint.h"

#include "btstack_config.h"
#include "btstack.h"
#include "SRAD_RT_db.h"

// Hardware API's
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/sync.h"

// GAP and GATT
#include "GAP_Advertisement/gap_config.h"
#include "GATT_Service/service_implementation.h"

// Low-level alarm infrastructure we'll be using to send data perodically
#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

// // DDS parameters
// #define two32 4294967296.0 // 2^32 
// #define Fs 50000
// #define DELAY 20 // 1/Fs (in microseconds)
// #define two32overFs 85899
// // the DDS units:
// volatile unsigned int phase_accum_main;
// volatile float freq = 400.0 ;
// volatile unsigned int phase_incr_main = (400*two32)/Fs ; ;//

// // DAC parameters
// // A-channel, 1x, active
// #define DAC_config_chan_A 0b0011000000000000
// // B-channel, 1x, active
// #define DAC_config_chan_B 0b1011000000000000

// // DDS sine table
// #define sine_table_size 256
// volatile int sin_table[sine_table_size] ;

// Period with which we'll enter the BTstack timer callback
#define CALL_PERIOD_MS 250

// BTstack objects
static btstack_timer_source_t heartbeat;
static btstack_packet_callback_registration_t hci_event_callback_registration;

// int lat_value = 33132860 ; // my home address
// int32_t long_value = -96883190 ; 
// float PT3_value = 25.6f ;
// float PT4_value = 30.5f ;
// uint8_t MAV_value = 1 ;
// uint8_t SV_value = 0 ;
// uint8_t Flightmode_value = 2;

// Buffers for sending the values as byte arrays
char lat_bytes[100];    // 4 bytes for an int32_t
char long_bytes[100];   // 4 bytes for an int32_t
char PT3_bytes[100];    // 4 bytes for a float
char PT4_bytes[100];    // 4 bytes for a float
char MAV_bytes[100];    // 1 byte for uint8_t
char SV_bytes[100];     // 1 byte for uint8_t
char FM_bytes[100];     // 1 byte for uint8_t


int main() {
    stdio_init_all();

    // Initialise the Wi-Fi/BLE chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Initialize L2CAP and security manager
    l2cap_init();
    sm_init();
    // Initialize ATT server, no general read/write callbacks
    // because we'll set one up for each service
    att_server_init(profile_data, NULL, NULL);
    
    // Instantiate our custom service handler
    custom_service_server_init(lat_bytes, long_bytes, PT3_bytes, PT4_bytes, MAV_bytes, SV_bytes, FM_bytes) ;
    
    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for ATT event
    att_server_register_packet_handler(packet_handler);

    // turn on bluetooth!
    hci_power_control(HCI_POWER_ON);
    
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    int lat_val = 33132860; // in micro degrees 10^-6
    int long_val = -96883190;
    float PT3 = 128.6f;
    float PT4 = 33.689f;
    bool MAV = 0;
    bool SV = true;
    int FM = 3;

    printf("Should have intialized\n");

    while (true) {
        sleep_ms(10000);
        set_latitude_value(lat_val);
        set_longitude_value(long_val);
        set_PT3_value(PT3);
        set_PT4_value(PT4);
        set_MAV_value(MAV);
        set_SV_value(SV);
        set_FM_value(FM);
        printf("Latitude: %d Longitude: %d PT3: %f PT4: %f MAV: %d SV: %d FM: %d\n" , lat_val, long_val, PT3, PT4, MAV, SV, FM);
    }

}
