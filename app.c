/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"


//1s is 32768 ticks - The Definition below sets up timer for 10ms
#define PERIODIC_TIMEOUT 328

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;


typedef union{
        struct{
            uint8_t     byte_0;
            uint8_t     byte_1;
            uint8_t     byte_2;
            uint8_t     byte_3;
            }Bytes;
        uint32_t value;
    } _32_8bit;




volatile _32_8bit tick_count;
// Handle for sleeptimer
sl_sleeptimer_timer_handle_t my_sleeptimer_handle;

volatile uint8_t data_send[] = {0,0,0,0,255,255,0,0,0,0,255,255,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
volatile uint8_t data_send2[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xbb};

volatile uint16_t TX_counter = 0;
volatile uint16_t RX_counter = 0;
volatile uint16_t CRCERR_counter = 0;
volatile uint16_t FAIL_counter = 0;

uint16_t payload_sent_len;

#define CHAR_HANDLE       21
#define PAYLOAD_LENGTH    64

//static uint8_t payload[PAYLOAD_LENGTH] = {2};

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}


/**************************************************************************//**
 * @brief
 *   Sleeptimer callback function. Each time sleeptimer reaches timeout value,
 *   this callback is executed.
 *****************************************************************************/
void sleeptimer_cb(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  sl_status_t sc;

  tick_count.value+=1;
  app_log("Tickcount = %d \n\r", tick_count.value);

  data_send[3] = tick_count.Bytes.byte_0;
  data_send[2] = tick_count.Bytes.byte_1;
  data_send[1] = tick_count.Bytes.byte_2;
  data_send[0] = tick_count.Bytes.byte_3;

  sc = sl_bt_system_get_counters(0, (uint16_t *)&TX_counter,  (uint16_t *)&RX_counter, (uint16_t *)&CRCERR_counter, (uint16_t *)&FAIL_counter );
  app_assert_status(sc);

  data_send[4] = 0xff;
  data_send[5] = 0xff;
  data_send[6] = TX_counter;
  data_send[7] = RX_counter;
  data_send[8] = CRCERR_counter;
  data_send[9] = FAIL_counter;


  sc = sl_bt_gatt_server_write_attribute_value(gattdb_Data_TX,
                                                                 0,
                                                                 sizeof(data_send),
                                                                 (unsigned char *)&data_send);
  app_assert_status(sc);

}


/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint8_t Dev_name[11] = "ConnDev0000";


  app_log("event: %x \n\r", SL_BT_MSG_ID(evt->header));

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);


      //Add last 2 bytes of the MAC ADDRESS to the Name
      Dev_name[7] = (0x0f & address.addr[4]);
      if(Dev_name[7]<=0x09) Dev_name[7]+=0x30;
      else Dev_name[7]+=0x37;
      Dev_name[8] = (address.addr[4]>>4);
      if(Dev_name[8]<=0x09) Dev_name[8]+=0x30;
      else Dev_name[8]+=0x37;
      Dev_name[9] = (0x0f & address.addr[5]);
      if(Dev_name[9]<=0x09) Dev_name[9]+=0x30;
      else Dev_name[9]+=0x37;
      Dev_name[10] = (address.addr[5]>>4);
      if(Dev_name[10]<=0x09) Dev_name[10]+=0x30;
      else Dev_name[10]+=0x37;

      data_send[10] = 0xFF;
      data_send[11] = 0xFF;
      data_send[12] = address.addr[5];
      data_send[13] = address.addr[4];
      data_send[14] = address.addr[3];
      data_send[15] = address.addr[2];
      data_send[16] = address.addr[1];
      data_send[17] = address.addr[0];
      data_send[18] = 0xFF;
      data_send[19] = 0xFF;



      sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,0,sizeof(Dev_name),Dev_name);
      app_assert_status(sc);

      //Set the Max TX power to 0 dBm
      sc = sl_bt_system_set_tx_power(0, 0, 0, 0);
      app_assert_status(sc);

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);

      //Writes Data regarding Data_TX to the internal GATT Database
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_Data_TX,0,sizeof(data_send),(unsigned char *)&data_send);
      app_assert_status(sc);
      //Writes Data regarding Data_RX to the internal GATT Database
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_Data_RX,0,sizeof(data_send2),(unsigned char *)&data_send2);
      app_assert_status(sc);

      //Resets Tick Counter
      tick_count.value = 0;
      //Initiates Tick Counter timer
      sc = sl_sleeptimer_start_periodic_timer(&my_sleeptimer_handle, PERIODIC_TIMEOUT, sleeptimer_cb, (void *)NULL,0,0);
      app_assert_status(sc);

      //Reset System Counters
      sc = sl_bt_system_get_counters(1, (uint16_t *)&TX_counter,  (uint16_t *)&RX_counter, (uint16_t *)&CRCERR_counter, (uint16_t *)&FAIL_counter );
      app_assert_status(sc);

      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      tick_count.value = 0;

      break;

    case sl_bt_evt_gatt_procedure_completed_id:

         app_log("Gatt Event Completed %x \n\r",evt->data.evt_gatt_procedure_completed.result );


         break;

    case sl_bt_evt_gatt_server_attribute_value_id:
      {

      app_log("Packet received, sending data back\r\n");
               // Received the 6 byte image, send 64 bytes back
//               sc = sl_bt_gatt_write_characteristic_value(evt->data.evt_gatt_server_attribute_value.connection,
//                                                     CHAR_HANDLE,
//                                                     64,
//                                                     data_send);
      sc =  sl_bt_gatt_write_characteristic_value_without_response(evt->data.evt_gatt_server_attribute_value.connection,
                                                       CHAR_HANDLE,
                                                       64,
                                                       data_send, &payload_sent_len);

               app_assert_status(sc);
      }
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:

      sl_sleeptimer_stop_timer(&my_sleeptimer_handle);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}
