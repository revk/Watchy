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
#define	BATHIGH		2400
#define	BATLOW		2000

extern uint8_t battery;
extern uint8_t menu1;
extern uint8_t menu2;
extern uint8_t menu3;
extern uint8_t face;
extern uint8_t flip;

typedef struct bits
{                               // Struct to just save a bit of RAM
   uint8_t charging:1;          // We are charging
   uint8_t holdoff:1;           // We want to stay on
   uint8_t wifi:1;              // We want wifi
   uint8_t revkstarted:1;       // Main revk library started so all settings loaded
   uint8_t wifistarted:1;       // WiFi started
   uint8_t newmin:1;            // This is start of new minute
   uint8_t newhour:1;           // This is start of new hour
} bits_t;
extern bits_t bits;


const char *gfx_qr (const char *value, uint8_t scale);
void face_init (void);          // Cold start up watch face
void face_show (time_t, char);  // Show current time
