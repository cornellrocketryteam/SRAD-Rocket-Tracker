//
// btstack_config.h 
// https://bluekitchen-gmbh.com/btstack/#how_to/
// https://vanhunteradams.com/Pico/BLE/GATT_Server.html#Configuring-BTstack
//

#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

#ifndef ENABLE_BLE
#error Please link to pico_btstack_ble
#endif

// Enable BTstack features
#define ENABLE_LE_PERIPHERAL // Enable BLE Peripheral role
#define ENABLE_LE_DATA_LENGTH_EXTENSION // (Optional) Improve data transfer efficiency
#define ENABLE_GATT_OVER_BLE // Enable GATT functionality over BLE
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR
#define ENABLE_PRINTF_HEXDUMP

// BTstack configuration. buffers, sizes, etc
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4
#define HCI_ACL_PAYLOAD_SIZE (255 + 4)
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4
#define MAX_NR_HCI_CONNECTIONS 1 // number of devices that can be connected at once 
#define MAX_NR_SM_LOOKUP_ENTRIES 3
#define MAX_NR_WHITELIST_ENTRIES 16
#define MAX_NR_LE_DEVICE_DB_ENTRIES 16
#define ENABLE_ATT_DELAYED_RESPONSE //  change - not sure what it does but migth fix everything

// Enable and configure HCI Controller to Host Flow Control to avoid cyw43 shared bus overrun
#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL // Enables HCI flow control between the controller and host. This is useful when hardware flow control (CTS/RTS) is unavailable, ensuring the controller doesn’t flood the host with too much data
#define HCI_HOST_ACL_PACKET_LEN (255+4) // Sets the maximum size of an ACL packet from the controller to the host. 255 is the max payload size of an ACL data packet. 4 accounts for the HCI ACL header.
#define HCI_HOST_ACL_PACKET_NUM 3 // Specifies the number of ACL packets the host can buffer. Higher values improve performance but require more RAM.
#define HCI_HOST_SCO_PACKET_LEN 120 // SCO Packets are used for audio
#define HCI_HOST_SCO_PACKET_NUM 3

// Limit number of ACL/SCO Buffer to use by stack to avoid cyw43 shared bus overrun
#define MAX_NR_CONTROLLER_ACL_BUFFERS 3
#define MAX_NR_CONTROLLER_SCO_PACKETS 3

// Link Key DB and LE Device DB using TLV on top of Flash Sector interface
#define NVM_NUM_DEVICE_DB_ENTRIES 8 // number of devices the system can remeber
#define NVM_NUM_LINK_KEYS 8 // Link key is used to authenticate and encrypt communications with paired devices.

// We don't give btstack a malloc, so use a fixed-size ATT DB.
#define MAX_ATT_DB_SIZE 512 // size of the GATT DB right now is 346 bytes, so it gives space to grow. Keeping MAX_ATT_DB_SIZE below 2048 bytes is recommended unless you have measured your memory usage.

// BTstack HAL configuration
#define HAVE_EMBEDDED_TIME_MS
// map btstack_assert onto Pico SDK assert()
#define HAVE_ASSERT

// Some USB dongles take longer to respond to HCI reset (e.g. BCM20702A).
#define HCI_RESET_RESEND_TIMEOUT_MS 1000 // timesout if a response if not given after 1 second not really needed but just in case
#define ENABLE_SOFTWARE_AES128 // used for encryption, delete if takes a lot of time
#define ENABLE_MICRO_ECC_FOR_LE_SECURE_CONNECTIONS // secure cryptographic operations on devices with minimal processing power. disable if not caring for security
#endif