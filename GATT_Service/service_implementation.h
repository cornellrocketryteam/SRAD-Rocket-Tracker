#include "btstack_defines.h"
#include "ble/att_db.h"
#include "ble/att_server.h"
#include "btstack_util.h"
#include "bluetooth_gatt.h"
#include "btstack_debug.h"

#include "SRAD_RT_db.h"
#include <pt_cornell_rp2040_v1_3.h>

// Creating a struct for managning the GPS 
typedef struct {

    // Connection handle for service
    hci_con_handle_t con_handle ;

    // GPS Information
    int32_t *latitude_value ;
    char * latitude_character_description ;
    int32_t * longitude_value ;
    char * longitude_character_description ;
    
    // PT3 & PT4 Information
    float * PT3_value ;
    char * PT3_character_description ;
    float * PT4_value ;
    char * PT4_character_description ;

    // MAV & SV Information
    uint8_t * MAV_value ;
    char * MAV_character_description ;
    uint8_t * SV_value ;
    char * SV_character_description ;

    // Flightmode
    uint8_t * Flightmode_value ;
    char * Flightmode_character_description ;

    // GPS Handles
    uint16_t latitude_handle ;
    uint16_t latitude_CD_handle ;
    uint16_t longitude_handle ;
    uint16_t longitude_CD_handle ;

    // PT3 & PT4 Handles
    uint16_t PT3_handle ;
    uint16_t PT3_CD_handle ;
    uint16_t PT4_handle ;
    uint16_t PT4_CD_handle ;

    // MAV & SV Handles
    uint16_t MAV_handle ;
    uint16_t SV_handle ;
    uint16_t MAV_CD_handle ;
    uint16_t SV_CD_handle ;

    // Flightmode Handles
    uint16_t Flightmode_handle ;
    uint16_t Flightmode_CD_handle ;

    // Callback functions, needed in order for asynchronous notifications
    btstack_context_callback_registration_t callback_lat ;
    btstack_context_callback_registration_t callback_long ;
    btstack_context_callback_registration_t callback_PT3 ;
    btstack_context_callback_registration_t callback_PT4 ;
    btstack_context_callback_registration_t callback_MAV ;
    btstack_context_callback_registration_t callback_SV ;
    btstack_context_callback_registration_t callback_FM ;

} GATT_DB ;

// Create a callback registration object, and an att service handler object
static att_service_handler_t service_handler ;
static GATT_DB service_object ;

// Characteristic user descriptions (appear in LightBlue app)
char characteristic_lat[] = "Latitiude micro-degrees" ;
char characteristic_long[] = "Longitude micro-degrees" ;
char characteristic_PT3[] = "PT3 - PSI" ;
char characteristic_PT4[] = "PT4 - PSI" ;
char characteristic_MAV[] = "MAV State" ;
char characteristic_SV[] = "SV State" ;
char characteristic_FM[] = "Flightmode" ;

// Protothreads semaphore
struct pt_sem BLUETOOTH_READY ;

// Callback functions for ATT notifications on characteristics
static void latitude_callback(void * context){
	// Associate the void pointer input with our custom service object
	GATT_DB * instance = (GATT_DB *) context ;
    // Dereference the pointer before converting to bytes
    int32_t latitude_value = *(instance->latitude_value);
    // Convert the int32_t latitude_value into a byte array
    uint8_t latitude_bytes[4];
    latitude_bytes[0] = (uint8_t) (latitude_value & 0xFF);
    latitude_bytes[1] = (uint8_t) ((latitude_value >> 8) & 0xFF);
    latitude_bytes[2] = (uint8_t) ((latitude_value >> 16) & 0xFF);
    latitude_bytes[3] = (uint8_t) ((latitude_value >> 24) & 0xFF);
    // Send a notification
    att_server_notify(instance->con_handle, instance->latitude_handle, latitude_bytes, sizeof(latitude_bytes));
}
static void longitude_callback(void * context) {
    GATT_DB * instance = (GATT_DB *) context ;
    int32_t longitude_value= *(instance->longitude_value);
    uint8_t longitude_bytes[4];
    longitude_bytes[0] = (uint8_t) (longitude_value & 0xFF);
    longitude_bytes[1] = (uint8_t) ((longitude_value >> 8) & 0xFF);
    longitude_bytes[2] = (uint8_t) ((longitude_value >> 16) & 0xFF);
    longitude_bytes[3] = (uint8_t) ((longitude_value >> 24) & 0xFF);
    // Send a notification
	att_server_notify(instance->con_handle, instance->longitude_handle, longitude_bytes, sizeof(longitude_bytes));
}
static void PT3_callback(void * context){
    GATT_DB * instance = (GATT_DB *) context ;
    // Convert float PT3 to 4-byte array using memcpy
    uint8_t PT3_bytes[4];
    memcpy(PT3_bytes, &instance->PT3_value, sizeof(instance->PT3_value));

    // Send a notification with PT3 value
    att_server_notify(instance->con_handle, instance->PT3_handle, PT3_bytes, sizeof(PT3_bytes));
}
static void PT4_callback(void * context){
    GATT_DB * instance = (GATT_DB *) context ;
    // Convert float PT3 to 4-byte array using memcpy
    uint8_t PT4_bytes[4];
    memcpy(PT4_bytes, &instance->PT4_value, sizeof(instance->PT4_value));

    // Send a notification with PT3 value
    att_server_notify(instance->con_handle, instance->PT4_handle, PT4_bytes, sizeof(PT4_bytes));
}
static void MAV_callback(void * context){
	// Associate the void pointer input with our custom service object
	GATT_DB * instance = (GATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->MAV_handle, instance->MAV_value, sizeof(instance->MAV_value)) ;
}
static void SV_callback(void * context){
	GATT_DB * instance = (GATT_DB *) context ;
	att_server_notify(instance->con_handle, instance->SV_handle, instance->SV_value, sizeof(instance->SV_value)) ;
}
static void FM_callback(void * context){
	GATT_DB * instance = (GATT_DB *) context ;
	att_server_notify(instance->con_handle, instance->Flightmode_handle, instance->Flightmode_value, sizeof(instance->Flightmode_value)) ;
}

// Read callback (no client configuration handles on characteristics without Notify)
static uint16_t custom_service_read_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
	UNUSED(con_handle);
    // Latitude Characteristic
    if (attribute_handle == service_object.latitude_handle){
        // Return latitude value as a blob (int32_t converted to bytes)
        return att_read_callback_handle_blob((uint8_t *)&service_object.latitude_value, sizeof(service_object.latitude_value), offset, buffer, buffer_size);
    }

    if (attribute_handle == service_object.longitude_handle){
        // Return longitude value as a blob (int32_t converted to bytes)
        return att_read_callback_handle_blob((uint8_t *)&service_object.longitude_value, sizeof(service_object.longitude_value), offset, buffer, buffer_size);
    }

    if (attribute_handle == service_object.PT3_handle){
        // Convert PT3 float value to bytes (using type casting to uint8_t* for byte array)
        uint8_t pt3_bytes[sizeof(service_object.PT3_value)];

        // Copy the float value into the byte array (may need to handle byte-ordering if needed)
        memcpy(pt3_bytes, &service_object.PT3_value, sizeof(service_object.PT3_value));

        // Return the PT3 value as a blob (float converted to bytes)
        return att_read_callback_handle_blob(pt3_bytes, sizeof(pt3_bytes), offset, buffer, buffer_size);
    }

    if (attribute_handle == service_object.PT4_handle){
        // Convert PT3 float value to bytes (using type casting to uint8_t* for byte array)
        uint8_t pt4_bytes[sizeof(service_object.PT4_value)];

        // Copy the float value into the byte array (may need to handle byte-ordering if needed)
        memcpy(pt4_bytes, &service_object.PT4_value, sizeof(service_object.PT4_value));

        // Return the PT3 value as a blob (float converted to bytes)
        return att_read_callback_handle_blob(pt4_bytes, sizeof(pt4_bytes), offset, buffer, buffer_size);
    }

    // MAV State
    if (attribute_handle == service_object.MAV_handle){
        return att_read_callback_handle_blob(service_object.MAV_value, sizeof(service_object.MAV_value), offset, buffer, buffer_size);
    }

    // SV State
    if (attribute_handle == service_object.SV_handle){
        return att_read_callback_handle_blob(service_object.SV_value, sizeof(service_object.SV_value), offset, buffer, buffer_size);
    }

    // Flightmode State
    if (attribute_handle == service_object.Flightmode_handle){
        return att_read_callback_handle_blob(service_object.Flightmode_value, sizeof(service_object.Flightmode_value), offset, buffer, buffer_size);
    }

    return 0;
}

// Add write callback functions here if needed

/////////////////////////////////////////////////////////////////////////////
////////////////////////////// USER API /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Our application code to be able to initialize our GATT server, and we would 
// like for our application to make its own updates to some of the characteristics 
// in that server, and optionally notify the client when those updates occur. 

// Initialize our custom service handler
void custom_service_server_init(int32_t * lat_ptr, int32_t * long_ptr, float * PT3_ptr, float * PT4_ptr, uint8_t * MAV_ptr, uint8_t * SV_ptr, uint8_t * FM_ptr){
    // Initialize the semaphore
    PT_SEM_SAFE_INIT(&BLUETOOTH_READY,0);
    // Pointer to our service object
    GATT_DB * instance = &service_object ;

    // Assign characteristic value
    instance->latitude_value = lat_ptr;
    instance->longitude_value = long_ptr;
    instance->PT3_value = PT3_ptr;
    instance->PT4_value = PT4_ptr;
    instance->MAV_value = MAV_ptr;
    instance->SV_value = SV_ptr;
    instance->Flightmode_value = FM_ptr;

    // Assign characteristic descriptions
    instance->latitude_character_description=characteristic_lat;
    instance->longitude_character_description=characteristic_long;
    instance->PT3_character_description=characteristic_PT3;
    instance->PT4_character_description=characteristic_PT4;
    instance->MAV_character_description=characteristic_MAV;
    instance->SV_character_description=characteristic_SV;
    instance->Flightmode_character_description=characteristic_FM;

    // Assigning Characteristic Handles
    instance->latitude_handle=ATT_CHARACTERISTIC_00000002_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE ;
    instance->latitude_CD_handle=ATT_CHARACTERISTIC_00000002_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE ;
    instance->longitude_handle=ATT_CHARACTERISTIC_00000003_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->longitude_CD_handle=ATT_CHARACTERISTIC_00000003_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->PT3_handle=ATT_CHARACTERISTIC_00000004_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->PT3_CD_handle=ATT_CHARACTERISTIC_00000004_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->PT4_handle=ATT_CHARACTERISTIC_00000005_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->PT4_CD_handle=ATT_CHARACTERISTIC_00000005_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->MAV_handle=ATT_CHARACTERISTIC_00000006_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->MAV_CD_handle=ATT_CHARACTERISTIC_00000006_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->SV_handle=ATT_CHARACTERISTIC_00000007_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->SV_CD_handle=ATT_CHARACTERISTIC_00000007_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->Flightmode_handle=ATT_CHARACTERISTIC_00000008_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->Flightmode_CD_handle=ATT_CHARACTERISTIC_00000008_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    }