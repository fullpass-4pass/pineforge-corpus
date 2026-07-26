# Rivet fill recalculation

PineForge independently authored this clean-room PineScript v6 strategy to
exercise `process_orders_on_close=true`, `calc_on_order_fills=true`, and
post-fill bracket attachment without adding a second order interaction.

Capture is pending on `BINANCE:ETHUSDT.P`, 15-minute bars, `Asia/Taipei`, Deep
Backtesting from 2025-04-01 through 2026-05-04, with source defaults. Same-bar
fill recalculation remains unqualified until the exact owned tape passes the
native Corpus verifier. The script has not been published. No performance
claim or investment advice is intended.
