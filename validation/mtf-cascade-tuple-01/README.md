# Cascade higher-timeframe tuple

PineForge independently authored this clean-room PineScript v6 strategy to
exercise tuple-return `request.security` evaluation for one higher-timeframe
EMA and ATR request with explicit gaps-off and lookahead-off policy.

Capture is pending on `BINANCE:ETHUSDT.P`, 15-minute bars, `Asia/Taipei`, Deep
Backtesting from 2025-04-01 through 2026-05-04, with source defaults. Native
validation must use the pinned 1-minute feed and its deterministic 15-minute
and 240-minute aggregations. The script has not been published. No performance
claim or investment advice is intended.
