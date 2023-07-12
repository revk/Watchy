// Accelerometer
// https://www.mouser.com/datasheet/2/783/BST-BMA423-DS000-1509600.pdf

static const char __attribute__((unused)) TAG[] = "Accelerometer";

#include "revk.h"
#include "watchy.h"
#include <driver/i2c.h>

uint8_t features[64] = { 0 };

static void
i2c_write (uint8_t reg, uint8_t val)
{
   esp_err_t e;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, reg, true);
   i2c_master_write_byte (txn, val, true);
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Write fail %02X %02X %d", reg, val, e);
   i2c_cmd_link_delete (txn);
}

static void
i2c_write_features (void)
{
   esp_err_t e;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x5E, true);
   i2c_master_write (txn, features, sizeof (features), true);
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Write fail %d", e);
   i2c_cmd_link_delete (txn);
}

static uint8_t
i2c_read (uint8_t reg)
{
   uint8_t val = 0;
   esp_err_t e;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, reg, true);
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte (txn, &val, I2C_MASTER_LAST_NACK);
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Read fail %d", e);
   i2c_cmd_link_delete (txn);
   return val;
}

static void
i2c_read_features (void)
{
   esp_err_t e;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x5E, true);
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (ACCADDRESS << 1) | I2C_MASTER_READ, true);
   i2c_master_read (txn, features, sizeof (features), I2C_MASTER_LAST_NACK);
   i2c_master_stop (txn);
   if ((e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS)))
      ESP_LOGE (TAG, "Read fail %d", e);
   i2c_cmd_link_delete (txn);
}

void
acc_init (void)
{
   ESP_LOGE (TAG, "Initialise");
   uint8_t chip = i2c_read (0);
   if (chip != 0x13)
   {
      ESP_LOGE (TAG, "Chip ID %02X", chip);
      return;
   }
   uint8_t status = i2c_read (0x2A);
   if (status == 1)
      return;                   // OK...
#if 1                           // Soft reset and reconfigure
   ESP_LOGE (TAG, "Soft reset err=%02X status=%02X", i2c_read (0x02), i2c_read (0x03));
   i2c_write (0x7E, 0xB6);
   sleep (1);
   status = i2c_read (0x2A);
   ESP_LOGE (TAG, "Status %02X", status);
   i2c_read_features ();
   ESP_LOG_BUFFER_HEX_LEVEL (TAG, features, sizeof (features), ESP_LOG_ERROR);
   features[0x36] = 0x30;
   i2c_write_features ();
   i2c_read_features ();
   ESP_LOG_BUFFER_HEX_LEVEL (TAG, features, sizeof (features), ESP_LOG_ERROR);
#endif
   i2c_write (0x7C, 0x00);      // PWR_CONF (aps_off)
   usleep (500000);             // Allow it to wake up
   i2c_write (0x59, 0x00);      // INIT_CTRL


   i2c_write (0x59, 0x01);      // INIT_CTRL
   status = i2c_read (0x2A);
   i2c_write (0x7D, 0x04);      // PWR_CTRL (acc_on)
   i2c_write (0x7C, 0x01);      // PWR_CONF (aps_on)
   if (status != 1)
      ESP_LOGE (TAG, "Status %02X", status);
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
