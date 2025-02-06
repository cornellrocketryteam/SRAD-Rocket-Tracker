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

int32_t lat_value = 33132860 ; // my home address
int32_t long_value = -96.883190 ; 
float PT3_value = 25.6f ;
float PT4_value = 30.5f ;
uint8_t MAV_value = 1 ;
uint8_t SV_value = 0 ;
uint8_t Flightmode_value = 2;

// Buffers for sending the values as byte arrays
uint8_t lat_bytes[4];    // 4 bytes for an int32_t
uint8_t long_bytes[4];   // 4 bytes for an int32_t
uint8_t PT3_bytes[4];    // 4 bytes for a float
uint8_t PT4_bytes[4];    // 4 bytes for a float
uint8_t MAV_bytes[1];    // 1 byte for uint8_t
uint8_t SV_bytes[1];     // 1 byte for uint8_t
uint8_t FM_bytes[1];     // 1 byte for uint8_t

void convert_int32_to_bytes(int32_t value, uint8_t bytes[]) {
    // Convert lat_value (int32_t) to byte array
    bytes[0] = (uint8_t) (value & 0xFF);
    bytes[1] = (uint8_t) ((value >> 8) & 0xFF);
    bytes[2] = (uint8_t) ((value >> 16) & 0xFF);
    bytes[3] = (uint8_t) ((value >> 24) & 0xFF);
}

 void convert_PT_to_bytes(float PT, uint8_t bytes[]) {
    memcpy(bytes, &PT, sizeof(PT));
 }

int main()
{
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
    
    // convert_int32_to_bytes(lat_value, lat_bytes);
    // convert_int32_to_bytes(long_value, long_bytes);
    // convert_PT_to_bytes(PT3_value, PT3_bytes);
    // convert_PT_to_bytes(PT4_value, PT4_bytes);
    // Instantiate our custom service handler
    custom_service_server_init(&lat_value, &long_value, &PT3_value, &PT4_value, &MAV_value, &SV_value, &Flightmode_value) ;
    
    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for ATT event
    att_server_register_packet_handler(packet_handler);

    // turn on bluetooth!
    hci_power_control(HCI_POWER_ON);
    
    // convert_int32_to_bytes(lat_value, lat_bytes);
    // convert_int32_to_bytes(long_value, long_bytes);
    // convert_PT_to_bytes(PT3_value, PT3_bytes);
    // convert_PT_to_bytes(PT4_value, PT4_bytes);
    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
        printf("Should have intialized");
    }

}
