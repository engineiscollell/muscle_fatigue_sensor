// ============================================================================
// adc_hw.cpp
// ============================================================================

#include "adc_hw.h"

#include <Arduino.h>
#include <SPI.h>

#include "config.h"

// ============================================================================
// GLOBALS
// ============================================================================

static volatile bool g_adcReading = false;

// ============================================================================
// REGISTRES AD7172-2
// ============================================================================

static constexpr uint8_t REG_STATUS      = 0x00;
static constexpr uint8_t REG_ADCMODE     = 0x01;
static constexpr uint8_t REG_IFMODE      = 0x02;
static constexpr uint8_t REG_DATA        = 0x04;
static constexpr uint8_t REG_ID          = 0x07;

static constexpr uint8_t REG_CH0         = 0x10;

static constexpr uint8_t REG_SETUPCON0   = 0x20;
static constexpr uint8_t REG_FILTCON0    = 0x28;

// ============================================================================
// COMMUNICATION REGISTER
// ============================================================================

static constexpr uint8_t COMM_WRITE = 0x00;
static constexpr uint8_t COMM_READ  = 0x40;

// ============================================================================
// SPI
// ============================================================================

static SPISettings spiSettings(
    1000000,
    MSBFIRST,
    SPI_MODE3
);

// ============================================================================
// INTERNAL DECLARATIONS
// ============================================================================

static void adc_write_register16(
    uint8_t reg,
    uint16_t value
);

static uint8_t adc_read_register8(
    uint8_t reg
);

static uint16_t adc_read_register16(
    uint8_t reg
);

static void start_single_conversion();

static uint32_t read_data_24bit();

// ============================================================================
// INIT ADC
// ============================================================================

void adcHwInit()
{
    // =========================================================================
    // GPIO
    // =========================================================================

    pinMode(PIN_ADC_CS, OUTPUT);

    digitalWrite(
        PIN_ADC_CS,
        HIGH
    );

    pinMode(
        PIN_ADC_DRDY,
        INPUT
    );

    // =========================================================================
    // RESET ADC
    // =========================================================================

    SPI.beginTransaction(
        spiSettings
    );

    digitalWrite(
        PIN_ADC_CS,
        LOW
    );

    for (int i = 0; i < 8; i++)
    {
        SPI.transfer(0xFF);
    }

    digitalWrite(
        PIN_ADC_CS,
        HIGH
    );

    SPI.endTransaction();

    delay(5);

    // =========================================================================
    // IFMODE
    // =========================================================================

    adc_write_register16(
        REG_IFMODE,
        0x0100
    );

    // =========================================================================
    // SETUP0
    // =========================================================================

    adc_write_register16(
        REG_SETUPCON0,
        0x80A0
    );

    // =========================================================================
    // FILTER0
    // sinc5 + sinc1
    // =========================================================================

    adc_write_register16(
        REG_FILTCON0,
        0x000B
    );

    // =========================================================================
    // CHANNEL0
    // =========================================================================

    adc_write_register16(
        REG_CH0,
        0x8001
    );

    // =========================================================================
    // ADCMODE
    // REF_EN + SINGLE CONVERSION
    // =========================================================================

    adc_write_register16(
        REG_ADCMODE,
        0x8020
    );

    // =========================================================================
    // READBACK DEBUG
    // =========================================================================

    uint8_t id =
        adc_read_register16(REG_ID);

    uint8_t status =
        adc_read_register8(REG_STATUS);

    uint16_t adcmode =
        adc_read_register16(REG_ADCMODE);

    uint16_t ifmode =
        adc_read_register16(REG_IFMODE);

    uint16_t setup =
        adc_read_register16(REG_SETUPCON0);

    uint16_t filt =
        adc_read_register16(REG_FILTCON0);

    uint16_t ch0 =
        adc_read_register16(REG_CH0);

    Serial.println();
    Serial.println("[ADC] READBACK");

    Serial.printf(
        "ID         : 0x%04X\n",
        id
    );

    Serial.printf(
        "STATUS     : 0x%02X\n",
        status
    );

    Serial.printf(
        "ADCMODE    : 0x%04X\n",
        adcmode
    );

    Serial.printf(
        "IFMODE     : 0x%04X\n",
        ifmode
    );

    Serial.printf(
        "SETUPCON0  : 0x%04X\n",
        setup
    );

    Serial.printf(
        "FILTCON0   : 0x%04X\n",
        filt
    );

    Serial.printf(
        "CH0        : 0x%04X\n",
        ch0
    );

    Serial.printf(
        "DRDY state : %d\n",
        digitalRead(PIN_ADC_DRDY)
    );

    Serial.println("[ADC] Init OK");
}

// ============================================================================
// READ SAMPLE
// ============================================================================

uint32_t adcHwReadSample()
{
    // =========================================================================
    // START CONVERSION
    // =========================================================================

    start_single_conversion();

    // =========================================================================
    // WAIT ADC
    // =========================================================================

    delay(5);

    // =========================================================================
    // READ DATA
    // =========================================================================

    return read_data_24bit();
}

// ============================================================================
// INTERNALS
// ============================================================================

// ----------------------------------------------------------------------------
// WRITE REGISTER 16 BIT
// ----------------------------------------------------------------------------

static void adc_write_register16(
    uint8_t reg,
    uint16_t value
)
{
    SPI.beginTransaction(
        spiSettings
    );

    digitalWrite(
        PIN_ADC_CS,
        LOW
    );

    SPI.transfer(
        COMM_WRITE | reg
    );

    SPI.transfer(
        (value >> 8) & 0xFF
    );

    SPI.transfer(
        value & 0xFF
    );

    digitalWrite(
        PIN_ADC_CS,
        HIGH
    );

    SPI.endTransaction();
}

// ----------------------------------------------------------------------------
// READ REGISTER 8 BIT
// ----------------------------------------------------------------------------

static uint8_t adc_read_register8(
    uint8_t reg
)
{
    uint8_t value = 0;

    SPI.beginTransaction(
        spiSettings
    );

    digitalWrite(
        PIN_ADC_CS,
        LOW
    );

    SPI.transfer(
        COMM_READ | reg
    );

    value =
        SPI.transfer(0x00);

    digitalWrite(
        PIN_ADC_CS,
        HIGH
    );

    SPI.endTransaction();

    return value;
}

// ----------------------------------------------------------------------------
// READ REGISTER 16 BIT
// ----------------------------------------------------------------------------

static uint16_t adc_read_register16(
    uint8_t reg
)
{
    uint16_t value = 0;

    SPI.beginTransaction(
        spiSettings
    );

    digitalWrite(
        PIN_ADC_CS,
        LOW
    );

    SPI.transfer(
        COMM_READ | reg
    );

    value |=
        ((uint16_t)SPI.transfer(0x00) << 8);

    value |=
        ((uint16_t)SPI.transfer(0x00));

    digitalWrite(
        PIN_ADC_CS,
        HIGH
    );

    SPI.endTransaction();

    return value;
}

// ----------------------------------------------------------------------------
// START SINGLE CONVERSION
// ----------------------------------------------------------------------------

static void start_single_conversion()
{
    SPI.beginTransaction(
        spiSettings
    );

    digitalWrite(
        PIN_ADC_CS,
        LOW
    );

    SPI.transfer(
        COMM_WRITE | REG_ADCMODE
    );

    // REF_EN + SINGLE CONVERSION MODE

    SPI.transfer(0x80);
    SPI.transfer(0x10);

    digitalWrite(
        PIN_ADC_CS,
        HIGH
    );

    SPI.endTransaction();
}

// ----------------------------------------------------------------------------
// READ DATA REGISTER
// ----------------------------------------------------------------------------

static uint32_t read_data_24bit()
{
    uint32_t data = 0;

    g_adcReading = true;

    SPI.beginTransaction(
        spiSettings
    );

    digitalWrite(
        PIN_ADC_CS,
        LOW
    );

    SPI.transfer(
        COMM_READ | REG_DATA
    );

    uint8_t b1 =
        SPI.transfer(0x00);

    uint8_t b2 =
        SPI.transfer(0x00);

    uint8_t b3 =
        SPI.transfer(0x00);

    digitalWrite(
        PIN_ADC_CS,
        HIGH
    );

    SPI.endTransaction();

    g_adcReading = false;

    data =
        ((uint32_t)b1 << 16) |
        ((uint32_t)b2 << 8)  |
        ((uint32_t)b3);

    return data;
}