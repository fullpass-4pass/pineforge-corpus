# ta-isolate-05-sma-152-close-cross

## Purpose
Isolate `ta.sma(close, 152)` over a long window (~38h on 15m TF). Tests engine sliding-window sum precision. Naive running-sum has ULP drift `~1.5e-13` per update; over 30k bars that compounds to ~1e-9 — measurable at threshold crossings.

Targets long-window SMA paths used by community-style market-structure-shift logic (this corpus does not redistribute that script) and any other SMA(152)-consuming surface in this tree.

## Setup + export
Standard corpus workflow (score with the engine repo's `scripts/verify_corpus.py`). Quick reference:
- Symbol/TF: ETH-USDT-USDT 15m, full warmup6m window
- Apply `strategy.pine` → Strategy Tester → List of Trades → CSV → `tv_trades.csv` next to this README

## Diagnostic interpretation
| Result | Conclusion |
|---|---|
| `excellent` | Engine SMA sliding-sum bit-exact at length 152. MarketShift SMA path fine — combined with probe 04, isolates the MarketShift gap to its non-TA logic OR OHLCV input. |
| `strong/moderate` 1-3 trades | Sliding-sum running-error at threshold boundary. Consider engine periodic-recompute every N bars to bound drift. |
| `moderate/weak` significant drift | Algorithmic difference: TV may use full N-bar resum each bar (slow but bit-exact); engine may use running-sum (fast but drifty). Switch engine to per-bar resum at long lengths. Per-bar diff: publish a helper indicator exposing `float t_sma152 = ta.sma(close, 152)` on the same chart, then diff against engine `// @pf-trace`. |
