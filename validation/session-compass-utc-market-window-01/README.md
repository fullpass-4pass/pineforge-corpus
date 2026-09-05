# Compass UTC market window

PineForge independently authored this clean-room PineScript v6 strategy to
exercise explicit session and timezone evaluation. A UTC trading window gates
EMA entries, the session boundary closes risk, and an ATR stop remains active
inside the window.

## TradingView capture

- Symbol: `BINANCE:ETHUSDT.P`
- Chart timeframe: 15 minutes
- Chart timezone: `Asia/Taipei`
- Strategy session/timezone: `0800-1600`, `Etc/UTC`
- Deep Backtesting range: 2025-04-01 through 2026-05-04
- Other inputs and strategy properties: source defaults
- Public publication: not performed
- Owned source SHA-256:
  `39b293e6b1b064008b65150638c8f3f4b8a6441e983325c03160e8e3e486e7f6`
- Generated C++ SHA-256:
  `7098736e8add0e5ffdb51b24db20022b1aa710f7eec406d7dee6eaf207a3e5a5`
- TradingView export SHA-256:
  `6fa95a6c62eb7c0d53969dcb089d72debd0679b3fdf564e77a303f5365f9159b`
- Native PineForge Corpus result: `excellent`

This is a validation example, not a performance claim or investment advice.
