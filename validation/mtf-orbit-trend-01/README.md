# Orbit higher-timeframe trend

PineForge independently authored this clean-room PineScript v6 strategy to
exercise `request.security`, `input.timeframe`, gaps-off, and lookahead-off
with a single higher-timeframe EMA. Market execution and an ATR guard keep the
higher-timeframe materialization as the distinct semantic purpose.

Capture is pending on `BINANCE:ETHUSDT.P`, 15-minute bars, `Asia/Taipei`, Deep
Backtesting from 2025-04-01 through 2026-05-04, with source defaults. Native
validation must use the pinned 1-minute feed and its deterministic 15-minute
and 60-minute aggregations. The script has not been published. No performance
claim or investment advice is intended.
