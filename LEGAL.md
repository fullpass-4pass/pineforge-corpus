# Legal — PineForge Validation Corpus

This corpus is licensed under the Apache License, Version 2.0
(see `LICENSE`). Same license as the PineForge engine runtime.

## What This Tree Contains

- **PineScript v6 sources** (`strategy.pine`) — clean-room PineForge
  originals. Each carries an Apache-2.0 SPDX header, a clean-room
  marker, and a rationale block describing the engine surface under
  test. None are derived from third-party PineScript on TradingView's
  public library or community scripts; the prior `basic/` (TradingView
  built-in templates) and `community/` (mixed MPL-2.0 + unlicensed
  third-party) trees were deleted and replaced by these clean-room
  rewrites.

- **TradingView "List of Trades" CSV exports** (`tv_trades.csv`) —
  emitted by TradingView's broker emulator running our own clean-room
  `strategy.pine` files on the reference OHLCV feed, exported manually
  (no scraping, no API, no automated extraction). Included as
  **factual parity reference records**. Our position: these are
  factual outcomes of our **own** strategies (facts are not
  copyrightable). We do **not** Apache-relicense TradingView's export
  format/columns, so they sit in-tree as factual reference, not as an
  Apache-2.0 work of ours. The one open item is **contractual** —
  public redistribution under TradingView's Terms of Service — being
  confirmed by US + Taiwan counsel (opinion targeted Q3 2026). Parity
  does not depend on shipping them — see **Reproducibility** below.

- **Engine trade lists** (`engine_trades.csv`) — produced by PineForge
  against the same OHLCV in TradingView's row-and-column format for
  direct line-by-line parity comparison.

- **OHLCV market data** (`data/ohlcv_ETH-USDT-USDT_1m.csv`) —
  Binance USDT-M Perpetual ETH/USDT-USDT. Public market price/volume
  series; redistribution is factual-data territory.

- **Transpiler output** (`generated.cpp`) — the C++ output of the
  [`pineforge-codegen`](https://github.com/pineforge-4pass/pineforge-codegen-oss)
  transpiler over each `strategy.pine`. The transpiler is
  **source-available** under the **PolyForm Noncommercial License
  1.0.0** (a separate repository); its output over our clean-room
  `strategy.pine` originals is shipped here under the same Apache-2.0
  license as those `.pine` files. Including `generated.cpp` in-tree
  means the engine repo plus this corpus plus a C++17 compiler is
  sufficient to reproduce the headline parity figure end-to-end — no
  transpiler access required.

The compiled per-probe artefacts (`strategy.dylib` / `strategy.so` /
`strategy.dll`) remain platform-specific build outputs and are not
shipped; reproducers compile them locally via `cmake --build build
--target corpus_strategies`. See `corpus/.gitignore`.

## Trademarks

TradingView and PineScript are trademarks of their respective owners.
This corpus is not affiliated with or endorsed by TradingView.

## License Field

SPDX `Apache-2.0` covers the **PineForge-authored** artefacts in this
tree — `strategy.pine`, `generated.cpp`, `engine_trades.csv`, scripts,
and build config. Public market `data/` (Binance OHLCV) is factual
price/volume data. `tv_trades.csv` are TradingView exports included as
factual reference only and are **not** a PineForge-licensed work (see
above).

## Reproducibility

The headline parity figure documented in `validation_report.md` is
reproducible from the PineForge-authored artefacts in this tree plus
the public engine runtime — **without** the `tv_trades.csv` reference
files: `strategy.pine` + `generated.cpp` + public OHLCV regenerate
`engine_trades.csv`, which you can diff against a TradingView export you
produce yourself. See the project root `README.md` for the full
reproducer instructions.
