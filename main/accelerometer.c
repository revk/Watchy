// Accelerometer
// https://www.mouser.com/datasheet/2/783/BST-BMA423-DS000-1509600.pdf

static const char __attribute__((unused)) TAG[] = "Accelerometer";

#include "revk.h"
#include "watchy.h"
#include <driver/i2c.h>

void
acc_init (void)
{
   ESP_LOGE (TAG, "Initialise");
   uint8_t chip = 0;
   esp_err_t e;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x00, true);     // STEP_CNT_0
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte (txn, &chip, I2C_MASTER_LAST_NACK);
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Read fail %d", e);
   else
      ESP_LOGE (TAG, "Chip ID %02X", chip);
   // TODO we should check if running OK before doing a reset really...
   i2c_cmd_link_delete (txn);
   txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x7E, true);     // CMD
   i2c_master_write_byte (txn, 0xB6, true);     // Soft reset 
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Write fail (CMD) %d", e);
   sleep (1);
   i2c_cmd_link_delete (txn);
   txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x5E, true);     // Features
   i2c_master_write_byte (txn, 0x36, true);     // Enabled
   i2c_master_write_byte (txn, 0x00, true);     // 
   i2c_master_write_byte (txn, 0x30, true);     // Should be en_activity and en_counter
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Write fail (PWR_CTRL) %d", e);
   i2c_cmd_link_delete (txn);
}

uint32_t
steps_read (void)
{
   esp_err_t e;
   uint8_t A,
     B,
     C,
     D;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x1E, true);     // STEP_COUNTER_0
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte (txn, &A, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &B, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &C, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &D, I2C_MASTER_LAST_NACK);
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Read fail %d", e);
   i2c_cmd_link_delete (txn);
   if (e)
      return 0;
   return ((uint32_t) D << 24) + ((uint32_t) C << 16) + ((uint32_t) B << 8) + A;
}

uint32_t
activity_read (void)
{
   esp_err_t e;
   uint8_t A;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x27, true);     // ACTIVITY_TYPE
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte (txn, &A, I2C_MASTER_LAST_NACK);
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Read fail %d", e);
   i2c_cmd_link_delete (txn);
   if (e)
      return 0;
   return A;
}
