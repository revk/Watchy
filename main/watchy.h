// Common values

// GPIOs
#define	GPIOTX		1
#define	GPIORX		3
#define	GPIOBTN1	26
#define	GPIOBTN2	25
#define	GPIOBTN3	35
#define	GPIOBTN4	4
#define	GPIOSS		5
#define	GPIODC		10
#define	GPIORES		9
#define	GPIOSCK		18
#define	GPIOSDA		21
#define	GPIOSCL		22
#define	GPIOMOSI	23
#define	GPIORTCINT	27
#define	GPIOADC		34
#define	GPIOVIB		13
#define	GPIOACCINT1	14
#define	GPIOACCINT2	12
#define	GPIOBUSY	19

// Other fixed values
#define	I2CPORT	0
#define	RTCADDRESS	0x51
#define	ADCCHANNEL	ADC_CHANNEL_6
#define	ACCADDRESS	0x18
#define	BATHIGH		2400    // Based on what I see on full charge
#define	BATLOW		1700    // Guess

extern time_t moon_next;
extern uint32_t steps;
extern uint32_t last_steps;
extern uint8_t battery;
extern uint8_t menu1;
extern uint8_t menu2;
extern uint8_t menu3;
extern uint8_t face;
extern uint8_t flip;
extern uint8_t moon_phase;
extern char rtctz[];

typedef struct bits
{                               // Struct to just save a bit of RAM
   uint8_t charging:1;          // We are charging
   uint8_t startup:1;           // We want startup
   uint8_t wifi:1;              // We want wifi connected
   uint8_t holdoff:1;           // We want to stay on line (e.g. access internet, etc)
   uint8_t busy:1;              // Really keep up
   uint8_t revkstarted:1;       // Main revk library started so all settings loaded
   uint8_t wifistarted:1;       // WiFi started
   uint8_t newmin:1;            // This is start of new minute
   uint8_t newhour:1;           // This is start of new hour
   uint8_t newday:1;            // This is start of new day (localtime)
   uint8_t timeunsync:1;        // Time sync wanted
} bits_t;
extern bits_t bits;
extern const uint8_t gfx_cos[256];

const char *gfx_qr (const char *value, uint8_t scale);
void gfx_gap (int8_t);
void gfx_square_icon (const uint8_t * icon, uint16_t bytes, uint8_t visible);
uint16_t gfx_square_icon_size (uint16_t);
void gfx_status (void);
void gfx_battery (void);        // Icon
void gfx_percent (void);        // Icon
void gfx_charging (void);       // Icon
void gfx_wifi (void);           // Icon
void gfx_mqtt (void);           // Icon
void gfx_phase (uint8_t cx, uint8_t cy, uint8_t r);
void gfx_analogue (uint8_t cx, uint8_t cy, uint8_t r, struct tm *t);

#define	gfx_icon(i) gfx_square_icon(icon_##i,icon_##i##_size,1)
#define	gfx_icon_size(i) gfx_square_icon_size(icon_##i##_size)
#define	gfx_iconq(i,v) gfx_square_icon(icon_##i,icon_##i##_size,v)

const char *st (uint8_t n);

void face_init (void);          // Cold start up watch face
void face_show (time_t, char);  // Show current time
