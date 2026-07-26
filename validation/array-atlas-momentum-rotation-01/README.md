# Atlas array momentum rotation

PineForge independently authored this clean-room PineScript v6 strategy to
exercise bounded array mutation, array averaging, explicit history handling,
and percent-rank momentum. Execution uses conventional market entry/close and
an ATR guard so collection semantics remain the distinct purpose.

A private TradingView capture was executed on `BINANCE:ETHUSDT.P`, 15-minute
bars, `Asia/Taipei`, Deep Backtesting from 2025-04-01 through 2026-05-04, with
source defaults. The native Corpus verifier returned `excellent`; admission
remains blocked until the 30-strategy integration commit exists. The script
has not been published. No performance claim or investment advice is intended.
