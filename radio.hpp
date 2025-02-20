/**
 * @file radio.hpp
 * @author sj728
 *
 * @brief Interface for the radio module (RFD900x)
 */

#ifndef RADIO_HPP
#define RADIO_HPP
#include "telemetry.hpp"

/* Container for radio functionality */
class Radio
{
public:
    /**
     * Initialize radio module
     * @returns true if successful, false otherwise
     */
    bool start();

    /**
     * Read newest data from radio module.
     * @returns true if new data was read, false otherwise
     */
    bool read(Telemetry *data);
};

#endif // RADIO_HPP