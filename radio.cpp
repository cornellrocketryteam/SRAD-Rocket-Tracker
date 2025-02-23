/**
 * @file radio.cpp
 * @author sj728
 *
 * @brief Radio module implementation for the RFD900x
 */

#include "radio.hpp"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "telemetry.hpp"
#include "time.h"
#include "pico/time.h"
#include "pins.hpp"
#include <cstdio>
#include <sstream>
#include <cstring>
#include <inttypes.h> // For PRId64
#include "hardware/dma.h" // For DMA

uint8_t rx_buffer[PACKET_SIZE];  // Buffer to store received data
volatile bool packet_received = false;  // Flag to indicate packet completion
int dma_chan;
dma_channel_config dma_config;

void dma_handler() {
    // Clear the interrupt flag
    dma_hw->ints0 = 1u << dma_chan;

    // Set flag indicating a full packet was received
    packet_received = true;

    // Restart the DMA for the next packet
    dma_channel_set_read_addr(dma_chan, &uart_get_hw(UART_PORT)->dr, true);
}

void setup_uart_dma() {
    // Initialize UART1
    uart_init(UART_PORT, RFM_BAUDRATE);
    gpio_set_function(4, GPIO_FUNC_UART);  // TX Pin (if used)
    gpio_set_function(5, GPIO_FUNC_UART);  // RX Pin

    uart_set_format(UART_PORT, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_PORT, false); // Disable FIFO to use DMA properly

    // Configure DMA Channel for UART RX
    dma_chan = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_chan);
    
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8); // 8-bit transfers
    channel_config_set_read_increment(&dma_config, false); // Read from fixed address (UART RX FIFO)
    channel_config_set_write_increment(&dma_config, true); // Write to buffer sequentially
    channel_config_set_dreq(&dma_config, uart_get_dreq(UART_PORT, false)); // UART RX DREQ

    dma_channel_configure(
        dma_chan,
        &dma_config,
        rx_buffer,         // Write address (our buffer)
        &uart_get_hw(UART_PORT)->dr, // Read address (UART1 RX FIFO)
        PACKET_SIZE,       // Number of bytes per transfer
        false              // Don't start yet
    );

    // Enable DMA IRQ when transfer is complete
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Start DMA Transfer
    dma_channel_start(dma_chan);
}

#ifdef RATS_VERBOSE
#define debug_log(...)       \
    do                       \
    {                        \
        printf(__VA_ARGS__); \
    } while (0)
#else
#define debug_log(...) \
    do                 \
    {                  \
    } while (0)
#endif

bool Radio::start()
{
    // By default, the radio module has baudrate value rfm_baudrate, so we
    // set that value on the local UART port
    uart_init(UART_PORT, RFM_BAUDRATE);

    gpio_set_function(RFM_TX, GPIO_FUNC_UART);
    gpio_set_function(RFM_RX, GPIO_FUNC_UART);

    // Set UART format to 8 data bits, 1 stop bit, no parity
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);

    // Set flow control to false for both
    uart_set_hw_flow(UART_PORT, false, false);

    uart_set_fifo_enabled(UART_PORT, false); // Disable FIFO to use DMA properly

    // DMA Setup
    dma_chan = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_dreq(&dma_config, uart_get_dreq(UART_PORT, false));

    dma_channel_configure(
        dma_chan,
        &dma_config,
        rx_buffer,         // Buffer to store packet
        &uart_get_hw(UART_PORT)->dr, // Read from UART RX FIFO
        PACKET_SIZE,      // Packet size (107 bytes)
        false              // Don't start yet
    );

    // Enable DMA IRQ
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Start DMA transfer
    dma_channel_start(dma_chan);

    return true;
}

bool Radio::read(Telemetry *result)
{
    // Wait until the DMA transfer completes
    if (!packet_received) {
        return false;  // No new packet received yet
    }

    packet_received = false;  // Reset flag

    // Copy the received data into the result structure
    std::memcpy(result, rx_buffer, sizeof(Telemetry));

    // Restart DMA transfer for the next packet
    dma_channel_set_read_addr(dma_chan, &uart_get_hw(UART_PORT)->dr, true);

    return true;
} 
