// RTC
// https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf

static const char __attribute__((unused)) TAG[] = "RTC";

#include "revk.h"
#include "watchy.h"
#include <driver/i2c.h>

esp_err_t
ertc_init (void)
{
   i2c_config_t config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = GPIOSDA,
      .scl_io_num = GPIOSCL,
      .sda_pullup_en = true,
      .scl_pullup_en = true,
      .master.clk_speed = 100000,
   };
   esp_err_t e = i2c_driver_install (I2CPORT, I2C_MODE_MASTER, 0, 0, 0);
   if (!e)
      e = i2c_param_config (I2CPORT, &config);
   if (!e)
      e = i2c_set_timeout (I2CPORT, 80000 * 5);
   return e;
}

time_t
ertc_read (void)
{
   uint8_t S=0,
     M=0,
     H=0,
     d=0,
     w=0,
     m=0,
     y=0;
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (RTCADDRESS << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte (txn, 0x02, true);
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (RTCADDRESS << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte (txn, &S, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &M, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &H, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &d, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &w, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &m, I2C_MASTER_ACK);
   i2c_master_read_byte (txn, &y, I2C_MASTER_LAST_NACK);
   i2c_master_stop (txn);
   esp_err_t e = i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete (txn);
   if (e)
      return 0;
   ESP_LOGI (TAG, "Rx %02X %02X %02X %02X %02X %02X %02X", S, M, H, d, w, m, y);
   struct tm t = { 0 };
   t.tm_sec = ((S & 0x70) >> 4) * 10 + (S & 0xF);
   t.tm_min = ((M & 0x70) >> 4) * 10 + (M & 0xF);
   t.tm_hour = ((H & 0x30) >> 4) * 10 + (H & 0xF);
   t.tm_mday = ((d & 0x70) >> 4) * 10 + (d & 0xF);
   t.tm_mon = ((m & 0x10) >> 4) * 10 + (m & 0xF) - 1;
   t.tm_year = (m >> 7) * 100 + ((y & 0xF0) >> 4) * 10 + (y & 0xF);
   //time_t now=timegm (&t);
   time_t now = mktime (&t);    // TODO needs to be timegm for non UK timezones
   if (S & 0x80)
      return 0;                 // time reported but not set in RTC chip
   return now;
}

void
ertc_write (time_t now)
{
   struct tm t = { 0 };
   gmtime_r (&now, &t);         // Store UTC
   uint8_t S = ((t.tm_sec / 10) << 4) + (t.tm_sec % 10);
   uint8_t M = ((t.tm_min / 10) << 4) + (t.tm_min % 10);
   uint8_t H = ((t.tm_hour / 10) << 4) + (t.tm_hour % 10);
   uint8_t d = ((t.tm_mday / 10) << 4) + (t.tm_mday % 10);
   uint8_t w = t.tm_wday;
   uint8_t m = (((t.tm_mon + 1) / 10) << 4) + ((t.tm_mon + 1) % 10) + ((t.tm_year / 100) << 7);
   uint8_t y = (((t.tm_year / 10) % 10) << 4) + (t.tm_year % 10);
   ESP_LOGI (TAG, "Tx %02X %02X %02X %02X %02X %02X %02X", S, M, H, d, w, m, y);
   i2c_cmd_handle_t txn = i2c_cmd_link_create ();
   i2c_master_start (txn);
   i2c_master_write_byte (txn, (RTCADDRESS << 1) | I2C_MASTER_WRITE, true);
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
   i2c_master_cmd_begin (I2CPORT, txn, 10 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete (txn);
   return;
}
