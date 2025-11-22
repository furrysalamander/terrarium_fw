#pragma once

#include <Arduino.h>

String loadIndexTemplate();
String loadPortalTemplate();

void sendMainCss();
void sendAppJs();
void sendPortalCss();
void sendPortalJs();
