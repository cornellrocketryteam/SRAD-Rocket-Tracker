#include "btstack_defines.h"
#include "ble/att_db.h"
#include "ble/att_server.h"
#include "btstack_util.h"
#include "bluetooth_gatt.h"
#include "hardware/sync.h"

// Create a struct for managing this service
typedef struct {

	// Connection handle for service
	hci_con_handle_t con_handle ;

	// Latitiude information
	char * 		latitude_value ;
	char * 		latitude_user_description ;
    
    // Longitude information
	char * 		longitude_value ;
	char * 		longitude_user_description ;

    // PT3 Information
    char * 		PT3_value ;
	char * 		PT3_user_description ;

    // PT4 Information
    char * 		PT4_value ;
	char * 		PT4_user_description ;

    // MAV Information
    char * 		MAV_value ;
	char * 		MAV_user_description ;

    // SV Information
    char * 		SV_value ;
	char * 		SV_user_description ;

    // Flightmode Information
    char * 		FM_value ;
	char * 		FM_user_description ;

	// Characteristic latitude handles
	uint16_t  	latitude_handle ;
	uint16_t 	latitude_user_description_handle ;
	uint16_t    latitiude_client_configuration ;
	uint16_t    latitiude_client_configuration_handle ;

    // Characteristic longitude handles
	uint16_t  	longitude_handle ;
	uint16_t 	longitude_user_description_handle ;
	uint16_t    longitude_client_configuration ;
	uint16_t    longitude_client_configuration_handle ;

    // Characteristic PT3 handles
	uint16_t  	PT3_handle ;
	uint16_t 	PT3_user_description_handle ;
	uint16_t    PT3_client_configuration ;
	uint16_t    PT3_client_configuration_handle ;

    // Characteristic PT4 handles
	uint16_t  	PT4_handle ;
	uint16_t 	PT4_user_description_handle ;
	uint16_t    PT4_client_configuration ;
	uint16_t    PT4_client_configuration_handle ;

    // Characteristic MAV handles
	uint16_t  	MAV_handle ;
	uint16_t 	MAV_user_description_handle ;
	uint16_t    MAV_client_configuration ;
	uint16_t    MAV_client_configuration_handle ;

    // Characteristic SV handles
	uint16_t  	SV_handle ;
	uint16_t 	SV_user_description_handle ;
	uint16_t    SV_client_configuration ;
	uint16_t    SV_client_configuration_handle ;

    // Characteristic FM handles
	uint16_t  	FM_handle ;
	uint16_t 	FM_user_description_handle ;
	uint16_t    FM_client_configuration ;
	uint16_t    FM_client_configuration_handle ;

	// Callback functions
	btstack_context_callback_registration_t callback_lat ;
    btstack_context_callback_registration_t callback_long ;
    btstack_context_callback_registration_t callback_PT3 ;
    btstack_context_callback_registration_t callback_PT4 ;
    btstack_context_callback_registration_t callback_MAV ;
    btstack_context_callback_registration_t callback_SV ;
    btstack_context_callback_registration_t callback_FM ;

} GYATT_DB ;

// Create a callback registration object, and an att service handler object
static att_service_handler_t service_handler ;
static GYATT_DB service_object ;

// Characteristic user descriptions (appear in LightBlue app)
char char_lat[] = "Latitiude micro-degrees" ;
char char_long[] = "Longitude micro-degrees" ;
char char_PT3[] = "PT3 - PSI" ;
char char_PT4[] = "PT4 - PSI" ;
char char_MAV[] = "MAV State" ;
char char_SV[] = "SV State" ;
char char_FM[] = "Flightmode" ;

// Callback functions for ATT notifications on characteristics
static void characteristic_latitude_callback(void * context){
	// Associate the void pointer input with our custom service object
	GYATT_DB * instance = (GYATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->latitude_handle, (uint8_t*)instance->latitude_value, strlen(instance->latitude_value)) ;
}

static void characteristic_longitude_callback(void * context){
	// Associate the void pointer input with our custom service object
	GYATT_DB * instance = (GYATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->longitude_handle, (uint8_t*)instance->longitude_value, strlen(instance->longitude_value)) ;
}

static void characteristic_PT3_callback(void * context){
	// Associate the void pointer input with our custom service object
	GYATT_DB * instance = (GYATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->PT3_handle, (uint8_t*)instance->PT3_value, strlen(instance->PT3_value)) ;
}

static void characteristic_PT4_callback(void * context){
	// Associate the void pointer input with our custom service object
	GYATT_DB * instance = (GYATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->PT4_handle, (uint8_t*)instance->PT4_value, strlen(instance->PT4_value)) ;
}

static void characteristic_MAV_callback(void * context){
	// Associate the void pointer input with our custom service object
	GYATT_DB * instance = (GYATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->MAV_handle, (uint8_t*)instance->MAV_value, strlen(instance->MAV_value)) ;
}

static void characteristic_SV_callback(void * context){
	// Associate the void pointer input with our custom service object
	GYATT_DB * instance = (GYATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->SV_handle, (uint8_t*)instance->SV_value, strlen(instance->SV_value)) ;
}

static void characteristic_FM_callback(void * context){
	// Associate the void pointer input with our custom service object
	GYATT_DB * instance = (GYATT_DB *) context ;
	// Send a notification
	att_server_notify(instance->con_handle, instance->FM_handle, (uint8_t*)instance->FM_value, strlen(instance->FM_value)) ;
}

// Read callback (no client configuration handles on characteristics without Notify)
static uint16_t custom_service_read_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
	UNUSED(con_handle);

	// Characteristic Latitude
	if (attribute_handle == service_object.latitude_handle){
		return att_read_callback_handle_blob((uint8_t*)service_object.latitude_value, strlen(service_object.latitude_value), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.latitude_user_description_handle) {
		return att_read_callback_handle_blob((uint8_t*)service_object.latitude_user_description, strlen(service_object.latitude_user_description), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.latitiude_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(service_object.latitiude_client_configuration, offset, buffer, buffer_size);
    }

    // Characteristic Longitude
	if (attribute_handle == service_object.longitude_handle){
		return att_read_callback_handle_blob((uint8_t*)service_object.longitude_value, strlen(service_object.longitude_value), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.longitude_user_description_handle) {
		return att_read_callback_handle_blob((uint8_t*)service_object.longitude_user_description, strlen(service_object.longitude_user_description), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.longitude_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(service_object.longitude_client_configuration, offset, buffer, buffer_size);
    }

    // Characteristic PT3
	if (attribute_handle == service_object.PT3_handle){
		return att_read_callback_handle_blob((uint8_t*)service_object.PT3_value, strlen(service_object.PT3_value), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.PT3_user_description_handle) {
		return att_read_callback_handle_blob((uint8_t*)service_object.PT3_user_description, strlen(service_object.PT3_user_description), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.PT3_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(service_object.PT3_client_configuration, offset, buffer, buffer_size);
    }

    // Characteristic PT4
	if (attribute_handle == service_object.PT4_handle){
		return att_read_callback_handle_blob((uint8_t*)service_object.PT4_value, strlen(service_object.PT4_value), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.PT4_user_description_handle) {
		return att_read_callback_handle_blob((uint8_t*)service_object.PT4_user_description, strlen(service_object.PT4_user_description), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.PT4_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(service_object.PT4_client_configuration, offset, buffer, buffer_size);
    }

    // Characteristic MAV
	if (attribute_handle == service_object.MAV_handle){
		return att_read_callback_handle_blob((uint8_t*)service_object.MAV_value, strlen(service_object.MAV_value), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.MAV_user_description_handle) {
		return att_read_callback_handle_blob((uint8_t*)service_object.MAV_user_description, strlen(service_object.MAV_user_description), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.MAV_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(service_object.MAV_client_configuration, offset, buffer, buffer_size);
    }

    // Characteristic SV
	if (attribute_handle == service_object.SV_handle){
		return att_read_callback_handle_blob((uint8_t*)service_object.SV_value, strlen(service_object.SV_value), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.SV_user_description_handle) {
		return att_read_callback_handle_blob((uint8_t*)service_object.SV_user_description, strlen(service_object.SV_user_description), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.SV_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(service_object.SV_client_configuration, offset, buffer, buffer_size);
    }

    // Characteristic FM
	if (attribute_handle == service_object.FM_handle){
		return att_read_callback_handle_blob((uint8_t*)service_object.FM_value, strlen(service_object.FM_value), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.FM_user_description_handle) {
		return att_read_callback_handle_blob((uint8_t*)service_object.FM_user_description, strlen(service_object.FM_user_description), offset, buffer, buffer_size);
	}
	if (attribute_handle == service_object.FM_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(service_object.FM_client_configuration, offset, buffer, buffer_size);
    }
    return 0;
}

// Write callback
static int custom_service_write_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);

	// Enable/disable notifications - Lat
    if (attribute_handle == service_object.latitiude_client_configuration_handle){
        service_object.latitiude_client_configuration = little_endian_read_16(buffer, 0);
        service_object.con_handle = con_handle;
    }
    // Enable/disable notifications - Long
    if (attribute_handle == service_object.longitude_client_configuration_handle){
        service_object.longitude_client_configuration = little_endian_read_16(buffer, 0);
        service_object.con_handle = con_handle;
    }
	// Enable/disable notifications - PT3
    if (attribute_handle == service_object.PT3_client_configuration_handle){
        service_object.PT3_client_configuration = little_endian_read_16(buffer, 0);
        service_object.con_handle = con_handle;
    }
    // Enable/disable notifications - PT4
    if (attribute_handle == service_object.PT4_client_configuration_handle){
        service_object.PT4_client_configuration = little_endian_read_16(buffer, 0);
        service_object.con_handle = con_handle;
    }
	// Enable/disable notifications - MAV
    if (attribute_handle == service_object.MAV_client_configuration_handle){
        service_object.MAV_client_configuration = little_endian_read_16(buffer, 0);
        service_object.con_handle = con_handle;
    }
    // Enable/disable notifications - SV
    if (attribute_handle == service_object.SV_client_configuration_handle){
        service_object.SV_client_configuration = little_endian_read_16(buffer, 0);
        service_object.con_handle = con_handle;
    }
	// Enable/disable notifications - FM
    if (attribute_handle == service_object.FM_client_configuration_handle){
        service_object.FM_client_configuration = little_endian_read_16(buffer, 0);
        service_object.con_handle = con_handle;
    }
	return 0;

}
/////////////////////////////////////////////////////////////////////////////
////////////////////////////// USER API /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void custom_service_server_init(char * lat_ptr, char * long_ptr, char * PT3_ptr, char * PT4_ptr, char * MAV_ptr, char * SV_ptr, char * FM_ptr)
{

    // Pointer to our service object
	GYATT_DB * instance = &service_object ;

    // Assign characteristic value
	instance->latitude_value = lat_ptr ;
	instance->longitude_value = long_ptr ;
	instance->PT3_value = PT3_ptr ;
	instance->PT4_value = PT4_ptr ;
	instance->MAV_value = MAV_ptr ;
	instance->SV_value = SV_ptr ;
    instance->FM_value = FM_ptr ;

    // Assign characteristic user description
	instance->latitude_user_description = char_lat;
	instance->longitude_user_description = char_long ;
	instance->PT3_user_description = char_PT3 ;
	instance->PT4_user_description = char_PT4 ;
	instance->MAV_user_description = char_MAV ;
	instance->SV_user_description = char_SV ;
    instance->FM_user_description = char_FM ;

    // Assigning Characteristic Handles
    instance->latitude_handle=ATT_CHARACTERISTIC_00000002_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE ;
    instance->latitude_user_description_handle=ATT_CHARACTERISTIC_00000002_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE ;
    instance->longitude_handle=ATT_CHARACTERISTIC_00000003_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->longitude_user_description_handle=ATT_CHARACTERISTIC_00000003_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->PT3_handle=ATT_CHARACTERISTIC_00000004_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->PT3_user_description_handle=ATT_CHARACTERISTIC_00000004_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->PT4_handle=ATT_CHARACTERISTIC_00000005_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->PT4_user_description_handle=ATT_CHARACTERISTIC_00000005_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->MAV_handle=ATT_CHARACTERISTIC_00000006_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->MAV_user_description_handle=ATT_CHARACTERISTIC_00000006_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->SV_handle=ATT_CHARACTERISTIC_00000007_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->SV_user_description_handle=ATT_CHARACTERISTIC_00000007_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;
    instance->FM_handle=ATT_CHARACTERISTIC_00000008_0000_0715_2006_853A52A41A44_01_VALUE_HANDLE;
    instance->FM_user_description_handle=ATT_CHARACTERISTIC_00000008_0000_0715_2006_853A52A41A44_01_USER_DESCRIPTION_HANDLE;

	instance->latitiude_client_configuration_handle=ATT_CHARACTERISTIC_00000002_0000_0715_2006_853A52A41A44_01_CLIENT_CONFIGURATION_HANDLE;
    instance->longitude_client_configuration_handle=ATT_CHARACTERISTIC_00000003_0000_0715_2006_853A52A41A44_01_CLIENT_CONFIGURATION_HANDLE;
    instance->PT3_client_configuration_handle=ATT_CHARACTERISTIC_00000004_0000_0715_2006_853A52A41A44_01_CLIENT_CONFIGURATION_HANDLE;
    instance->PT4_client_configuration_handle=ATT_CHARACTERISTIC_00000005_0000_0715_2006_853A52A41A44_01_CLIENT_CONFIGURATION_HANDLE;
    instance->MAV_client_configuration_handle=ATT_CHARACTERISTIC_00000006_0000_0715_2006_853A52A41A44_01_CLIENT_CONFIGURATION_HANDLE;
    instance->SV_client_configuration_handle=ATT_CHARACTERISTIC_00000007_0000_0715_2006_853A52A41A44_01_CLIENT_CONFIGURATION_HANDLE;
    instance->FM_client_configuration_handle=ATT_CHARACTERISTIC_00000008_0000_0715_2006_853A52A41A44_01_CLIENT_CONFIGURATION_HANDLE;
    

    // Service Start and End Handles
    service_handler.start_handle = ATT_SERVICE_00000001_0000_0715_2006_853A52A41A44_START_HANDLE;
    service_handler.end_handle = ATT_SERVICE_00000001_0000_0715_2006_853A52A41A44_END_HANDLE;
    service_handler.read_callback = &custom_service_read_callback;
	service_handler.write_callback = &custom_service_write_callback ;

    // Register the service handler
	att_server_register_service_handler(&service_handler);
}

// Update Latitude value
void set_latitude_value(int32_t * value){
	// Pointer to our service object
	GYATT_DB * instance = &service_object ;

	// Update field value
	sprintf(instance->latitude_value, "%d", *value);

	if (instance->latitiude_client_configuration) {
		// Register a callback
		instance->callback_lat.callback = characteristic_latitude_callback;
		instance->callback_lat.context  = (void*) instance;
		att_server_register_can_send_now_callback(&instance->callback_lat, instance->con_handle);
	}
}

// Update Longitude value
void set_longitude_value(int32_t * value){
	// Pointer to our service object
	GYATT_DB * instance = &service_object ;

	// Update field value
	sprintf(instance->longitude_value, "%d", *value) ;

	if (instance->longitude_client_configuration) {
		// Register a callback
		instance->callback_long.callback = characteristic_longitude_callback;
		instance->callback_long.context  = (void*) instance;
		att_server_register_can_send_now_callback(&instance->callback_long, instance->con_handle);
	}
}

// Update PT3 value
void set_PT3_value(float * value){

	// Pointer to our service object
	GYATT_DB * instance = &service_object ;

	// Update field value
	sprintf(instance->PT3_value, "%.3f", *value) ;

	if (instance->PT3_client_configuration) {
		// Register a callback
		instance->callback_PT3.callback = characteristic_PT3_callback;
		instance->callback_PT3.context  = (void*) instance;
		att_server_register_can_send_now_callback(&instance->callback_PT3, instance->con_handle);
	}
}

// Update PT4 value
void set_PT4_value(float * value){

	// Pointer to our service object
	GYATT_DB * instance = &service_object ;

	// Update field value
	sprintf(instance->PT4_value, "%.3f", *value) ;

	if (instance->PT4_client_configuration) {
		// Register a callback
		instance->callback_PT4.callback = characteristic_PT4_callback;
		instance->callback_PT4.context  = (void*) instance;
		att_server_register_can_send_now_callback(&instance->callback_PT4, instance->con_handle);
	}
}

// Update MAV value
void set_MAV_value(bool * value){

	// Pointer to our service object
	GYATT_DB * instance = &service_object ;

	// Update field value
	sprintf(instance->MAV_value, "%d", *value) ;

	if (instance->MAV_client_configuration) {
		// Register a callback
		instance->callback_MAV.callback = characteristic_MAV_callback;
		instance->callback_MAV.context  = (void*) instance;
		att_server_register_can_send_now_callback(&instance->callback_MAV, instance->con_handle);
	}
}

// Update SV value
void set_SV_value(bool * value){

	// Pointer to our service object
	GYATT_DB * instance = &service_object ;

	// Update field value
	sprintf(instance->SV_value, "%d", *value) ;

	if (instance->SV_client_configuration) {
		// Register a callback
		instance->callback_SV.callback = characteristic_SV_callback;
		instance->callback_SV.context  = (void*) instance;
		att_server_register_can_send_now_callback(&instance->callback_SV, instance->con_handle);
	}
}

// Update FM value
void set_FM_value(int * value){

	// Pointer to our service object
	GYATT_DB * instance = &service_object ;

	// Update field value
	sprintf(instance->FM_value, "%d", *value) ;

	if (instance->FM_client_configuration) {
		// Register a callback
		instance->callback_FM.callback = characteristic_FM_callback;
		instance->callback_FM.context  = (void*) instance;
		att_server_register_can_send_now_callback(&instance->callback_FM, instance->con_handle);
	}
}

void set_All(int32_t * latitude, int32_t * longitude, float * PT3, float * PT4, bool * MAV, bool * SV, int * FM) {
	set_latitude_value(latitude);
	set_longitude_value(longitude);
	set_PT3_value(PT3);
	set_PT4_value(PT4);
	set_MAV_value(MAV);
	set_SV_value(SV);
	set_FM_value(FM);
}