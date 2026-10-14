#ifndef SIMULATOR_HTML_H
#define SIMULATOR_HTML_H

#include <WString.h>

/**
 * @brief Renders the /simulator diagnostic page: one checkbox per cyclic CAN signal
 * (RenaultTwingoGen1Battery::sim_signals), grouped by send interval, with an I/P/A legend
 * and a note on signals whose real source is the BMS itself (not the EVC).
 *
 * @param[in] var
 * @return String
 */
#ifdef TWINGO_SIM_PAGE
String simulator_processor(const String& var);
#endif

/**
 * @brief The control block of the bus (format, mode, way of the sleep, state, cell source, stage times, vehicle age,
 * manual age, time in 0x436, kilometres of 0x426). Since Bau 15 it is part of the Twingo section of the Advanced page.
 */
String simulator_controls_html();

#endif
