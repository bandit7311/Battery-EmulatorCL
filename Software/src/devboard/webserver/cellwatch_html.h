#ifndef CELLWATCH_H
#define CELLWATCH_H

#include <WString.h>
#include "../../battery/RENAULT-TWINGO-GEN1-BATTERY.h"  // the switch TWINGO_CELLWATCH (Bau 20)

/**
 * @brief Replaces placeholder with content section in web page. Renders the Cellwatch diagnostic
 * page: polls /cellwatchStatus repeatedly and appends a new row only when a fresh sample has
 * actually arrived (change detection on the sample counter), no fixed reload timer.
 *
 * @param[in] var
 *
 * @return String
 */
#ifdef TWINGO_CELLWATCH
String cellwatch_processor(const String& var);
#endif

#endif
