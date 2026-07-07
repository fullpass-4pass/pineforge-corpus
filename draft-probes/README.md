# Draft probes — pending TradingView capture

Probes staged here are NOT yet in `validation/` because they have no
`tv_trades.csv` reference yet (capture pending). They are excluded from
`verify_corpus.py --all` (which scans basic/community/validation) so they do
not affect the headline parity count. Each carries `strategy.pine`,
`generated.cpp`, and an `engine_trades.csv` (current engine output on
the 15m chart feed derived from `data/ohlcv_ETH-USDT-USDT_1m.csv`). Once a TV export is captured into a
probe's `tv_trades.csv`, move the dir into `validation/` to activate it.

## order-poc-exit-while-flat-{true-01,false-02}

Designed 2026-06-28 to test the hypothesis that `process_orders_on_close=true`
drops a `from_entry` `strategy.exit` bracket issued while flat (alongside a
pending limit entry that fills on a later bar) — the shape of community
initial-balance / breakout strategies, and a suspected cause of exit
divergences observed in engine parity testing.

RESULT: hypothesis REJECTED. On the reference feed the engine output is
IDENTICAL for `process_orders_on_close` true vs false (179 trades each; 12
bracket fills [11 stop + 1 TP], 167 opposite-cross). The from_entry exit
survives the placement→fill gap and fires at its level under both modes — no
on_close-specific drop. (An earlier 5-bar synthetic suggested a drop; that was
a contrived-feed artifact, disproven by these real-feed probes.)

Kept as regression coverage for the `process_orders_on_close` × limit-entry ×
exit-issued-while-flat pattern, pending TV capture to confirm engine↔TV parity.
