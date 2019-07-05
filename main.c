#include <avr/interrupt.h>
#include <util/twi.h>
#include <util/delay.h>

#include <stdint.h>
#include <stdbool.h>

// The analog input. Note that A4 and A5 are also SDA/SCL for I2C!
#define ADC_INPUT_BANK DDRC
#define ADC_INPUT_PIN PC0
#define ADC_CHANNEL 0
// This can be increased to average several subsequent readings.
#define ADC_SAMPLES 1

// Read a new analog value every N milliseconds.
#define READ_INTERVAL_MS 100

// A pin toggled in each loop iteration for debugging purposes.
#define DEBUG_BANK DDRB
#define DEBUG_PORT PORTB
#define DEBUG_PIN PB0

// The address of the device on the I2C bus.
#define I2C_SLAVE_ADDRESS 0x42

// The last value read from the ADC
static uint16_t last_reading;
// A buffer to prevent data races between I2C and ADC
static volatile uint8_t sndbuf[3];
// The last byte received from I2C
static volatile uint8_t i2c_cmd;
// State of the debug pin (for toggling it)
static volatile bool debug_pin_on;

static void toggle_debug_pin(void)
{
    debug_pin_on = debug_pin_on ? false : true;
    if (debug_pin_on)
        DEBUG_PORT |= (1 << DEBUG_PIN);
    else
        DEBUG_PORT &= ~(1 << DEBUG_PIN);
}

static void on_i2c_request(void)
{
    toggle_debug_pin();

    switch (i2c_cmd) {
        case 0:
            sndbuf[0] = (last_reading >> 8) & 0xff;
            sndbuf[1] = last_reading & 0xff;
            sndbuf[2] = sndbuf[0] ^ sndbuf[1];
            TWDR = 0xff;
            break;
        case 1:
        case 2:
        case 3:
            TWDR = sndbuf[i2c_cmd - 1];
            break;
        default:
            TWDR = 0xaa;
            break;
    }
}

static void on_i2c_receive(const int received_byte)
{
    // The master device requests a specific byte, sent by on_i2c_request
    i2c_cmd = received_byte;
}

static uint16_t analog_read(const uint8_t channel)
{
    // Select ADC channel
    ADMUX = (ADMUX & ~(0x1F)) | (channel & 0x1F);
    // Do a single conversion and wait for it to conclude
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADCW;
}

static uint16_t analog_read_mean(const uint8_t channel, const uint8_t n)
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < n; i++)
        sum += analog_read(channel);

    return (uint16_t)(sum / n);
}

int main(void)
{
    // Set debug pin as output
    DEBUG_BANK |= (1 << DEBUG_PIN);

    // Init I2C
    cli();
    // load address into TWI address register
    TWAR = I2C_SLAVE_ADDRESS << 1;
    // enable address matching, enable TWI, clear TWINT, enable TW interrupt
    TWCR = (1 << TWIE) | (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
    sei();

    // ADC pin is an input pin
    ADC_INPUT_BANK &= ~(1 << ADC_INPUT_PIN);
    // use Vcc as reference
    ADMUX = (1 << REFS0);

    // ADC prescaler: 8 MHz / 64 = 125 KHz
    ADCSRA = (1 << ADPS2) | (1 << ADPS1);
    // enable ADC
    ADCSRA |= (1 << ADEN);

    // "Warm up" the ADC
    (void)analog_read(ADC_CHANNEL);

    // Main loop
    for (;;) {
        last_reading = (uint16_t)analog_read_mean(ADC_CHANNEL, ADC_SAMPLES);
        toggle_debug_pin();
        _delay_ms(READ_INTERVAL_MS);
    }
}

ISR(TWI_vect)
{
    switch(TW_STATUS)
    {
    case TW_SR_DATA_ACK:
        on_i2c_receive(TWDR);
        break;
    case TW_ST_SLA_ACK:
        on_i2c_request();
        break;
    case TW_ST_DATA_ACK:
        on_i2c_request();
        break;
    case TW_BUS_ERROR:
        // clear register first on error
        TWCR = 0;
        break;
    default:
        break;
    }
    TWCR = (1 << TWIE) | (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
}
