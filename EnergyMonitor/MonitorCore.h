#pragma once
#include <stdint.h>
const uint32_t MAX_BALANCE_MWH = 1000000000UL;
const uint32_t LOW_CREDIT_MWH = 200000UL;
const uint8_t HISTORY_SIZE = 12;
struct MonitorCore {
  uint32_t remainingMWh, residualWattMs, historySum;
  uint16_t history[HISTORY_SIZE];
  uint8_t historyIndex, historyCount;
  void reset(uint32_t credit) {
    remainingMWh = credit > MAX_BALANCE_MWH ? MAX_BALANCE_MWH : credit;
    residualWattMs = historySum = 0; historyIndex = historyCount = 0;
    for (uint8_t i = 0; i < HISTORY_SIZE; ++i) history[i] = 0;
  }
  void integrate(uint16_t watts, uint32_t elapsedMs) {
    if (!remainingMWh) { residualWattMs = 0; return; }
    // 1 mWh = 3600 W*ms. Carry the fractional remainder without float loss.
    uint64_t total = (uint64_t)watts * elapsedMs + residualWattMs;
    uint64_t used = total / 3600; residualWattMs = total % 3600;
    if (used >= remainingMWh) { remainingMWh = 0; residualWattMs = 0; }
    else remainingMWh -= (uint32_t)used;
  }
  bool addCredit(uint32_t amount) {
    if (!amount || amount > MAX_BALANCE_MWH - remainingMWh) return false;
    remainingMWh += amount; return true;
  }
  void sample(uint16_t watts) {
    if (historyCount == HISTORY_SIZE) historySum -= history[historyIndex];
    else ++historyCount;
    history[historyIndex] = watts; historySum += watts;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
  }
  float averageW() const { return historyCount ? (float)historySum / historyCount : 0; }
  bool low() const { return remainingMWh > 0 && remainingMWh <= LOW_CREDIT_MWH; }
};
// Strict decimal kWh, up to six fractional digits; integer arithmetic throughout.
inline bool parseCredit(const char *text, uint32_t &amount) {
  uint32_t whole = 0, fraction = 0, scale = 100000;
  uint8_t digits = 0, decimals = 0; bool dot = false;
  for (; *text; ++text) {
    if (*text == '.' && !dot && digits) { dot = true; continue; }
    if (*text < '0' || *text > '9') return false;
    uint8_t digit = *text - '0';
    if (!dot) {
      if (++digits > 4) return false;
      whole = whole * 10 + digit; if (whole > 1000) return false;
    } else {
      if (++decimals > 6) return false;
      fraction += digit * scale; scale /= 10;
    }
  }
  if (!digits || (dot && !decimals)) return false;
  amount = whole * 1000000UL + fraction;
  return amount > 0 && amount <= MAX_BALANCE_MWH;
}
// Diagnostic checks use a separate instance; no change to live balance.
inline uint8_t runCoreChecks(uint8_t &count) {
  uint8_t failures = 0; count = 0;
  MonitorCore test; uint32_t parsed = 0;
#define CHECK_CORE(c) do { ++count; if (!(c)) ++failures; } while (0)
  test.reset(2000000UL); test.integrate(0, 60000);
  CHECK_CORE(test.remainingMWh == 2000000UL);
  test.integrate(2000, 60000);
  CHECK_CORE(test.remainingMWh == 1966667UL && test.residualWattMs == 1200);
  test.reset(2000000UL);
  for (uint16_t i = 0; i < 600; ++i) test.integrate(2000, 100);
  CHECK_CORE(test.remainingMWh == 1966667UL && test.residualWattMs == 1200);
  test.reset(1); test.integrate(2000, 100);
  CHECK_CORE(test.remainingMWh == 0 && test.residualWattMs == 0);
  CHECK_CORE(test.addCredit(500000) && test.remainingMWh == 500000);
  test.reset(MAX_BALANCE_MWH);
  CHECK_CORE(!test.addCredit(1) && test.remainingMWh == MAX_BALANCE_MWH);
  CHECK_CORE(!test.addCredit(0));
  test.reset(200000); CHECK_CORE(test.low());
  test.reset(200001); CHECK_CORE(!test.low());
  test.reset(0); CHECK_CORE(!test.low()); CHECK_CORE(test.averageW() == 0);
  test.sample(2000); CHECK_CORE(test.averageW() == 2000);
  for (uint8_t i = 0; i < 12; ++i) test.sample(0);
  CHECK_CORE(test.averageW() == 0 && test.historyCount == 12);
  for (uint8_t i = 0; i < 6; ++i) test.sample(2000);
  CHECK_CORE(test.averageW() == 1000);
  CHECK_CORE(parseCredit("0.5", parsed) && parsed == 500000);
  CHECK_CORE(parseCredit("0.000001", parsed) && parsed == 1);
  CHECK_CORE(parseCredit("1000", parsed) && parsed == MAX_BALANCE_MWH);
  CHECK_CORE(!parseCredit("1000.000001", parsed));
  CHECK_CORE(!parseCredit("-1", parsed)); CHECK_CORE(!parseCredit("nan", parsed));
  CHECK_CORE(!parseCredit("1e3", parsed)); CHECK_CORE(!parseCredit("0", parsed));
  CHECK_CORE(!parseCredit("1.1234567", parsed));
  CHECK_CORE(!parseCredit("1.2junk", parsed)); CHECK_CORE(!parseCredit("", parsed));
  CHECK_CORE(!parseCredit("1.", parsed));
  CHECK_CORE((uint32_t)(50UL - 0xfffffff0UL) == 66UL);
#undef CHECK_CORE
  return failures;
}
