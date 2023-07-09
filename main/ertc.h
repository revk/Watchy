// RTC

esp_err_t ertc_init(void);
esp_err_t ertc_read(struct tm*);
esp_err_t ertc_write(struct tm*);
