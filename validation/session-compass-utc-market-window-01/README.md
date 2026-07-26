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
  `8c9e46e6cefa0da3590d5a64e6c2ddd28ee9f325aa14271321f8ac89c175983e`
- Generated C++ SHA-256:
  `7098736e8add0e5ffdb51b24db20022b1aa710f7eec406d7dee6eaf207a3e5a5`
- TradingView export SHA-256:
  `1ce1e20e8914c58489b673f383ff34ca6b6653b729714e127830e82e6ca8f6f1`
- Native PineForge Corpus result: `excellent`

This is a validation example, not a performance claim or investment advice.
