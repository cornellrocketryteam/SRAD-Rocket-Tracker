#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "stdint.h"
#include "cmath"

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

// BTstack objects
static btstack_timer_source_t heartbeat;
static btstack_packet_callback_registration_t hci_event_callback_registration;

// Buffers for sending the values as byte arrays
char lat_bytes[100]; 
char long_bytes[100];
char PT3_bytes[100]; 
char PT4_bytes[100]; 
char MAV_bytes[100]; 
char SV_bytes[100];
char FM_bytes[100];


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
    
    // define test values
    int lat_val = 33132860; // in micro degrees 10^-6
    int long_val = -96883190;
    float PT3 = 128.12345f; // PT3 and 4 are to the 5th decimal point
    float PT4 = 33.67890f;
    bool MAV = 0;
    bool SV = true;
    int FM = 3;

    printf("Should have intialized\n");

    while (true) {
        set_latitude_value(lat_val);
        set_longitude_value(long_val);
        set_PT3_value(PT3);
        set_PT4_value(PT4);
        set_MAV_value(MAV);
        set_SV_value(SV);
        set_FM_value(FM);
        sleep_ms(10000);
        printf("Latitude: %d Longitude: %d PT3: %.3f PT4: %.3f MAV: %d SV: %d FM: %d\n" , lat_val, long_val, PT3, PT4, MAV, SV, FM);
    }

}
