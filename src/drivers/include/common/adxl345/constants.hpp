#pragma once

/*************************** REGISTER MAP ***************************/
#define ADXL345_DEVID            0x00        // Device ID
#define ADXL345_RESERVED1        0x01        // Reserved. Do Not Access.
#define ADXL345_THRESH_TAP        0x1D        // Tap Threshold.
#define ADXL345_OFSX            0x1E        // X-Axis Offset.
#define ADXL345_OFSY            0x1F        // Y-Axis Offset.
#define ADXL345_OFSZ            0x20        // Z- Axis Offset.
#define ADXL345_DUR                0x21        // Tap Duration.
#define ADXL345_LATENT            0x22        // Tap Latency.
#define ADXL345_WINDOW            0x23        // Tap Window.
#define ADXL345_THRESH_ACT        0x24        // Activity Threshold
#define ADXL345_THRESH_INACT    0x25        // Inactivity Threshold
#define ADXL345_TIME_INACT        0x26        // Inactivity Time
#define ADXL345_ACT_INACT_CTL    0x27        // Axis Enable Control for Activity and Inactivity Detection
#define ADXL345_THRESH_FF        0x28        // Free-Fall Threshold.
#define ADXL345_TIME_FF            0x29        // Free-Fall Time.
#define ADXL345_TAP_AXES        0x2A        // Axis Control for Tap/Double Tap.
#define ADXL345_ACT_TAP_STATUS    0x2B        // Source of Tap/Double Tap
#define ADXL345_BW_RATE            0x2C        // Data Rate and Power mode Control
#define ADXL345_POWER_CTL        0x2D        // Power-Saving Features Control
#define ADXL345_INT_ENABLE        0x2E        // Interrupt Enable Control
#define ADXL345_INT_MAP            0x2F        // Interrupt Mapping Control
#define ADXL345_INT_SOURCE        0x30        // Source of Interrupts
#define ADXL345_DATA_FORMAT        0x31        // Data Format Control
#define ADXL345_DATAX0            0x32        // X-Axis Data 0
#define ADXL345_DATAX1            0x33        // X-Axis Data 1
#define ADXL345_DATAY0            0x34        // Y-Axis Data 0
#define ADXL345_DATAY1            0x35        // Y-Axis Data 1
#define ADXL345_DATAZ0            0x36        // Z-Axis Data 0
#define ADXL345_DATAZ1            0x37        // Z-Axis Data 1
#define ADXL345_FIFO_CTL        0x38        // FIFO Control
#define ADXL345_FIFO_STATUS        0x39        // FIFO Status

#define ADXL345_BW_1600            0xF            // 1111		IDD = 40uA
#define ADXL345_BW_800            0xE            // 1110		IDD = 90uA
#define ADXL345_BW_400            0xD            // 1101		IDD = 140uA
#define ADXL345_BW_200            0xC            // 1100		IDD = 140uA
#define ADXL345_BW_100            0xB            // 1011		IDD = 140uA
#define ADXL345_BW_50            0xA            // 1010		IDD = 140uA
#define ADXL345_BW_25            0x9            // 1001		IDD = 90uA
#define ADXL345_BW_12_5            0x8            // 1000		IDD = 60uA
#define ADXL345_BW_6_25            0x7            // 0111		IDD = 50uA
#define ADXL345_BW_3_13            0x6            // 0110		IDD = 45uA
#define ADXL345_BW_1_56            0x5            // 0101		IDD = 40uA
#define ADXL345_BW_0_78            0x4            // 0100		IDD = 34uA
#define ADXL345_BW_0_39            0x3            // 0011		IDD = 23uA
#define ADXL345_BW_0_20            0x2            // 0010		IDD = 23uA
#define ADXL345_BW_0_10            0x1            // 0001		IDD = 23uA
#define ADXL345_BW_0_05            0x0            // 0000		IDD = 23uA

#define ADXL345_DEVICE (0x53)    // Device Address for ADXL345
#define ADXL345_TO_READ (6) // Number of Bytes Read - Two Bytes Per Axis

