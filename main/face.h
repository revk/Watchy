// Watch faces

const char * gfx_qr (const char *value, gfx_pos_t posx, gfx_pos_t posy, uint8_t scale); // QR

void face_init(void);	// Cold start up watch face
void face_time(struct tm*);	// Show current time


