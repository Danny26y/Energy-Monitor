#include <LiquidCrystal.h>
#include <ctype.h>
#include <string.h>
#include "MonitorCore.h"

// Simulated energy credit only. Restart restores configured starting balance.
#ifndef START_CREDIT_MWH
#define START_CREDIT_MWH 2000000UL
#endif
const uint16_t MAX_POWER_W = 2000;
const bool REVERSE_SENSOR = false;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
MonitorCore monitor;
uint32_t lastReadMs, lastHistoryMs, lastPageMs, lastDisplayMs, lastReportMs;
uint16_t powerW = 0, rawAdc = 0;
uint8_t page = 0;
bool streamEnabled = false;
char inputLine[48];
uint8_t inputLength = 0;
bool inputOverflow = false;

uint16_t readPower() {
  rawAdc = analogRead(A0);
  uint16_t raw = REVERSE_SENSOR ? 1023 - rawAdc : rawAdc;
  return ((uint32_t)raw * MAX_POWER_W + 511) / 1023;
}
void printCredit(uint32_t value) {
  Serial.print(value / 1000000UL); Serial.print('.');
  uint32_t fraction = value % 1000000UL;
  for (uint32_t place = 100000; place > 0; place /= 10)
    Serial.print((uint8_t)((fraction / place) % 10));
}
void printStatus() {
  Serial.print(F("ADC=")); Serial.print(rawAdc);
  Serial.print(F(" W=")); Serial.print(powerW);
  Serial.print(F(" AvgW=")); Serial.println(monitor.averageW(), 1);
  Serial.print(F("Credit=")); printCredit(monitor.remainingMWh);
  Serial.print(F("kWh "));
  if (!monitor.remainingMWh) Serial.println(F("EMPTY"));
  else if (monitor.low()) Serial.println(F("LOW"));
  else Serial.println(F("OK"));
  Serial.print(F("Hours="));
  if (!monitor.remainingMWh) Serial.print('0');
  else if (monitor.averageW() < 1) Serial.print(F("IDLE"));
  else Serial.print(monitor.remainingMWh / (1000.0f * monitor.averageW()), 3);
  Serial.print(F(" ms=")); Serial.println(millis());
}
void printChecks() {
  uint8_t count;
  uint8_t failed = runCoreChecks(count);
  Serial.print(F("SELFTEST ")); Serial.print(failed ? F("FAIL ") : F("PASS "));
  Serial.print(count); Serial.print(F(" checks; failures=")); Serial.println(failed);
}
void drawDisplay() {
  lcd.setCursor(0, 0); lcd.print(F("                "));
  lcd.setCursor(0, 0); lcd.print(F("Power:")); lcd.print(powerW); lcd.print('W');
  if (monitor.low()) { lcd.setCursor(13, 0); lcd.print(F("LOW")); }
  lcd.setCursor(0, 1); lcd.print(F("                ")); lcd.setCursor(0, 1);
  if (!monitor.remainingMWh) { lcd.print(F("CREDIT EMPTY")); return; }
  if (page == 2 && monitor.low()) { lcd.print(F("LOW CREDIT!")); return; }
  if (page == 1) {
    lcd.print(F("Time:"));
    float average = monitor.averageW();
    if (average < 1) lcd.print(F("-- idle"));
    else {
      float hours = monitor.remainingMWh / (1000.0f * average);
      if (hours > 9999) lcd.print(F(">9999h"));
      else if (hours < 1.0f / 60.0f) {
        lcd.print((uint32_t)(hours * 3600.0f + 0.999f)); lcd.print(F(" s"));
      }
      else if (hours < 1.0f) { lcd.print(hours * 60.0f, 1); lcd.print(F(" min")); }
      else { lcd.print(hours, 2); lcd.print(F(" h")); }
    }
  } else {
    lcd.print(F("Left:")); lcd.print(monitor.remainingMWh / 1000000.0f, 3);
    lcd.print(F("kWh"));
  }
}
void handleCommand() {
  inputLine[inputLength] = 0;
  char *command = inputLine;
  while (*command == ' ') ++command;
  char *end = command + strlen(command);
  while (end > command && end[-1] == ' ') *--end = 0;
  for (char *p = command; *p; ++p) *p = toupper((unsigned char)*p);
  if (!*command) return;
  if (!strcmp(command, "HELP")) {
    Serial.println(F("ADD 0.500 (kWh), STATUS, TEST, HELP"));
    Serial.println(F("STREAM ON / STREAM OFF"));
    Serial.println(F("Restart restores starting credit."));
  } else if (!strcmp(command, "STATUS")) printStatus();
  else if (!strcmp(command, "TEST")) printChecks();
  else if (!strcmp(command, "STREAM ON")) {
    streamEnabled = true; Serial.println(F("OK stream ON (1 s)"));
  } else if (!strcmp(command, "STREAM OFF")) {
    streamEnabled = false; Serial.println(F("OK stream OFF"));
  } else if (!strncmp(command, "ADD ", 4)) {
    char *amountText = command + 4;
    while (*amountText == ' ') ++amountText;
    uint32_t amount;
    if (!parseCredit(amountText, amount))
      Serial.println(F("ERR amount: positive decimal kWh"));
    else if (!monitor.addCredit(amount)) Serial.println(F("ERR balance would exceed 1000 kWh"));
    else {
      Serial.print(F("OK added_kWh=")); printCredit(amount); Serial.println();
      printStatus(); page = 0; drawDisplay();
    }
  } else Serial.println(F("ERR unknown command; type HELP"));
}
void pollSerial() {
  // Bounded input work prevents continuous input starving energy sampling.
  for (uint8_t count = 0; count < 16 && Serial.available(); ++count) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (inputOverflow) Serial.println(F("ERR line too long; command ignored"));
      else if (inputLength) handleCommand();
      inputLength = 0; inputOverflow = false;
    } else if (c == '\b' || c == 127) {
      if (!inputOverflow && inputLength) --inputLength;
    } else if (c >= 32 && c <= 126 && !inputOverflow) {
      if (inputLength < sizeof(inputLine) - 1) inputLine[inputLength++] = c;
      else inputOverflow = true;
    }
  }
}
void setup() {
  pinMode(A0, INPUT); pinMode(LED_BUILTIN, OUTPUT);
  monitor.reset(START_CREDIT_MWH);
  lcd.begin(16, 2); Serial.begin(9600);
  powerW = readPower(); monitor.sample(powerW);
  lastReadMs = lastHistoryMs = lastPageMs = lastDisplayMs = lastReportMs = millis();
  Serial.println(F("ENERGY MONITOR SIM v2 | 9600 baud 8N1 | HELP for commands"));
  printChecks(); printStatus(); drawDisplay();
}
void loop() {
  uint32_t now = millis();
  uint32_t elapsed = now - lastReadMs;
  if (elapsed >= 100) {
    lastReadMs = now; monitor.integrate(powerW, elapsed); powerW = readPower();
  }
  if ((uint32_t)(now - lastHistoryMs) >= 1000) {
    lastHistoryMs = now; monitor.sample(powerW);
  }
  pollSerial();
  if ((uint32_t)(now - lastPageMs) >= 2000) {
    lastPageMs = now; page = (page + 1) % (monitor.low() ? 3 : 2);
  }
  if ((uint32_t)(now - lastDisplayMs) >= 250) {
    lastDisplayMs = now; drawDisplay();
  }
  digitalWrite(LED_BUILTIN, !monitor.remainingMWh || (monitor.low() && (now / 500) % 2));
  if (streamEnabled && (uint32_t)(now - lastReportMs) >= 1000) {
    lastReportMs = now; printStatus();
  }
}
