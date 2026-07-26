# Switchback all-in reversal

PineForge independently authored this clean-room PineScript v6 strategy to
exercise omitted-quantity, percent-of-equity reversal admission together with
an explicit prior-side close. The published source retains a conservative
10-percent default; the parity capture deliberately overrides order size to
100 percent and transaction costs to zero. The source's 50-percent margin
setting leaves enough broker-emulator headroom to isolate reversal admission
from forced-liquidation behavior.

## TradingView capture

- Symbol: `BINANCE:ETHUSDT.P`
- Chart timeframe: 15 minutes
- Chart timezone: `Asia/Taipei`
- Session: 24x7
- Deep Backtesting range: 2025-04-01 through 2026-05-04
- Validation overrides: default order size = 100 percent of equity,
  commission = 0 percent, slippage = 0 ticks
- Margin: source default, 50 percent for long and short
- Other inputs and strategy properties: source defaults
- Public publication: not performed
- Owned source SHA-256:
  `77027fec26840a4403914402ee01308f1b6b7d9471fdf65fe5be3d1fa9b24303`
- TradingView export SHA-256:
  `29e9785d1eeb3548b5b51dfa133650b5db314f2064060ac427f2855748559c74`

The native corpus verifier classifies this exact source/export/settings tuple
as `excellent`. The final open report row is treated as a report-only tail by
the native corpus comparison; 429 trades are closed in the export. No
performance claim or investment advice is intended.
