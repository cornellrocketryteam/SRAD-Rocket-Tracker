/**
 * @file pins.hpp
 * @author rt526
 *
 * @brief GPIO pin definitions for the Pico
 */
#ifndef PINS_HPP
#define PINS_HPP

// Radio Pins
#define UART_PORT uart1
#define RFM_TX 4
#define RFM_RX 5
#define RFM_BAUDRATE 115200

#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define PACKET_SIZE 107        // Size of each packet
#define DMA_CHANNEL 0          // Use DMA channel 0

#endif // PINS_HPP