// RTC
// https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf

static const char __attribute__((unused)) TAG[] = "RTC";


#include "revk.h"
#include <driver/i2c.h>
#define port_mask(x)    ((x)&0x7F)

extern uint8_t sda,
  scl,
  i2cport,
  rtcaddress;

esp_err_t
ertc_init (void)
{
   if (!sda || !scl)
      return ESP_FAIL;
   i2c_config_t config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = port_mask (sda),
      .scl_io_num = port_mask (scl),
      .sda_pullup_en = true,
      .scl_pullup_en = true,
      .master.clk_speed = 100000,
   };
   esp_err_t e = i2c_driver_install (i2cport, I2C_MODE_MASTER, 0, 0, 0);
   if (!e)
      e = i2c_param_config (i2cport, &config);
   if (!e)
      e = i2c_set_timeout (i2cport, 80000 * 5);
   return e;
}

esp_err_t
ertc_read (struct tm *t)
{
   if (!t)
      return ESP_FAIL;
   memset (t, 0, sizeof (*t));
   uint8_t S,
     M,
     H,
     d,
     w,
     m,
     y;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (rtcaddress << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x02, true);
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (rtcaddress << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte (txn, &S, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &M, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &H, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &d, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &w, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &m, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &y, I2C_MASTER_LAST_NACK);
   i2c_master_stop (txn);
   esp_err_t e = i2c_master_cmd_begin (i2cport, txn, 10 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete (txn);
   if (e)
      return e;
   ESP_LOGI (TAG, "Rx %02X %02X %02X %02X %02X %02X %02X", S, M, H, d, w, m, y);
   t->tm_sec = ((S & 0x70) >> 4) * 10 + (S & 0xF);
   t->tm_min = ((M & 0x70) >> 4) * 10 + (M & 0xF);
   t->tm_hour = ((H & 0x30) >> 4) * 10 + (H & 0xF);
   t->tm_mday = ((d & 0x70) >> 4) * 10 + (d & 0xF);
   t->tm_mon = ((m & 0x10) >> 4) * 10 + (m & 0xF);
   t->tm_year = (m >> 7) * 100 + ((y & 0xF0) >> 4) * 10 + (y & 0xF);
   t->tm_isdst = 0;             // UTC in RTC
   mktime (t);
   if (S & 0x80)
      return ESP_FAIL;          // time reported but not set in RTC chip
   return ESP_OK;
}

esp_err_t
ertc_write (struct tm *t)
{
   if (!t)
      return ESP_FAIL;
   time_t now = mktime (t);
   gmtime_r (&now, t);          // Store UTC
   ESP_LOGI (TAG, "DST %d %lld", t->tm_isdst, now);
   uint8_t S,
     M,
     H,
     d,
     w,
     m,
     y;
   S = ((t->tm_sec / 10) << 4) + (t->tm_sec % 10);
   M = ((t->tm_min / 10) << 4) + (t->tm_min % 10);
   H = ((t->tm_hour / 10) << 4) + (t->tm_hour % 10);
   d = ((t->tm_mday / 10) << 4) + (t->tm_mday % 10);
   w = t->tm_wday;
   m = (((t->tm_mon + 1) / 10) << 4) + ((t->tm_mon + 1) % 10) + ((t->tm_year / 100) << 7);
   y = (((t->tm_year / 10) % 10) << 4) + (t->tm_year % 10);
   ESP_LOGI (TAG, "Tx %02X %02X %02X %02X %02X %02X %02X", S, M, H, d, w, m, y);
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (rtcaddress << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x00, true);     // Address
   i2c_master_write_byte (txn, 0x00, true);     // Control 1
   i2c_master_write_byte (txn, 0x00, true);     // Control 2
   i2c_master_write_byte (txn, S, true);
   i2c_master_write_byte (txn, M, true);
   i2c_master_write_byte (txn, H, true);
   i2c_master_write_byte (txn, d, true);
   i2c_master_write_byte (txn, w, true);
   i2c_master_write_byte (txn, m, true);
   i2c_master_write_byte (txn, y, true);
   i2c_master_stop (txn);
   esp_err_t e = i2c_master_cmd_begin (i2cport, txn, 10 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete (txn);
   if (e)
      return e;
   return ESP_OK;
}
