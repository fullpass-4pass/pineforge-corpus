# PineForge validation corpus

The corpus is PineForge's reproducibility kit for the parity claim in the
project README. Every probe is a hand-written, clean-room PineScript v6
strategy paired with TradingView's exported trade list and PineForge's own
trade list, so a third party can diff the two CSVs and confirm engine
behaviour matches TradingView on the same bar feed.

## Headline parity

- **252** verified strategies, all under `corpus/validation/`.
- **~389,000 trades** total across the suite — summing the per-row TV /
  engine counts in [`validation_report.md`](validation_report.md):
  TV 389,590; engine 389,688 (`+98` ≈ 0.03 % over TV).
- **251** excellent (bit-for-bit or within strict thresholds on every
  parity dimension).
- **1** documented anomaly — `anomaly-equity-mirror-strategy-equity-01` —
  where TradingView's broker emulator exhibits non-deterministic
  accept/reject behaviour at the exact 1× equity margin boundary. Engine
  is deterministic and correct; full write-up lives in
  `pineforge-utils/parity-anomalies/tv-margin-boundary.md`.
- **0** strong / moderate / weak.

The canonical, regenerated-each-sweep disposition table is
[`validation_report.md`](validation_report.md) (rendered as
`validation_report.html` and `validation_report.pdf`).

## Artifact tuple

Each probe directory ships four files in git:

| File                | Source                       | Role                                                       |
| ------------------- | ---------------------------- | ---------------------------------------------------------- |
| `strategy.pine`     | hand-written                 | PineScript v6 source                                       |
| `generated.cpp`     | pineforge-codegen transpiler | C++ output of the transpiler over `strategy.pine`          |
| `tv_trades.csv`     | TradingView export           | TV broker emulator's trade list for `strategy.pine`        |
| `engine_trades.csv` | PineForge                    | Engine's trade list for the same script (TV-format CSV)    |

`generated.cpp` is the transpiler output of our own clean-room
PineScript and ships under the same Apache-2.0 license as
`strategy.pine`. It is included in-tree so public users can rebuild
without needing access to the separate, source-available `pineforge-codegen`
transpiler — `cmake --build build --target corpus_strategies` compiles
each `generated.cpp` into a per-strategy shared library. The compiled
`strategy.dylib` / `.so` / `.dll` are platform-specific build artefacts
and remain ignored.

## Reference OHLCV

The corpus ships exactly **one** feed (stored via Git LFS):

- `data/ohlcv_ETH-USDT-USDT_1m.csv` — Binance ETH-USDT-USDT perp
  1-minute bars, full exchange history from the instrument's 2020
  listing through the end of the comparison window. The deep history
  matches the depth TradingView's own chart computes warmup over, so
  TA, MTF, pivot, and equity-feedback state starts where TV's does.

Every other feed the harnesses consume is derived deterministically
from it into `data/derived/` (gitignored) by the engine repo's
`scripts/derive_corpus_feeds.py` (invoked automatically by
`scripts/run_corpus.sh` and `scripts/run_strategy.py`):

- `data/derived/ohlcv_ETH-USDT-USDT_15m.csv` — 900s resample
  (open=first, high=max, low=min, close=last, volume=sum), the default
  15m chart feed.
- `data/derived/ohlcv_ETH-USDT-USDT_15m_window.csv` — comparison-window
  slice of the above, used by cold-start probes and as the harness's
  window-bounds reference.

`ltf-*` probes consume the committed 1m feed directly (engine-side
aggregation to the 15m chart); `magnifier-*` probes synthesize intrabar
ticks from chart bars and need no extra feed.

## Layout

```
corpus/
├── validation/                252 probes — surface-driven probe family
│   ├── ta-*                    61 probes — TA built-in math (rsi, macd, sma, ...)
│   ├── composite-*             52 probes — multi-surface integration (community-style)
│   ├── order-*                 40 probes — entry/exit/cancel placement
│   ├── udt-*                   22 probes — user-defined types + methods
│   ├── mtf-*                   16 probes — request.security regular HTF
│   ├── bracket-*               13 probes — TP/SL via strategy.exit / strategy.order
│   ├── matrix-*                 6 probes — matrix<T> typed/generic
│   ├── analyzer-*               6 probes — engine analyzer / parity isolation
│   ├── drawing-*                6 probes — drawing objects as data
│   ├── pyramid-*                4 probes — pyramiding=N
│   ├── oca-*                    3 probes — OCA group cancel/reduce/none
│   ├── magnifier-*              3 probes — bar_magnifier sub-bar walks
│   ├── ltf-*                    2 probes — request.security_lower_tf arrays
│   ├── session-*                2 probes — session() / TZ / DST
│   ├── recompute-*              2 probes — calc_on_every_tick / TA recompute
│   ├── na-*                     2 probes — na propagation
│   ├── input-*                  2 probes — input.source runtime override / subscript
│   ├── cap-*                    2 probes — intraday cap (max_intraday_filled_orders)
│   ├── barstate-*               2 probes — barstate.* checks
│   ├── vwap-*                   2 probes — VWAP band pricing / fills
│   ├── risk-*                   1 probe  — risk gates / limits
│   ├── stats-*                  1 probe  — performance stats / reporting
│   ├── timeframe-*              1 probe  — script_tf/input_tf timeframe handling
│   ├── anomaly-*                1 probe  — documented TV non-determinism
│   └── symbol-specified/       (excluded from sweep) 5 stock probes pending pineforge-data
├── draft-probes/               staged probes pending TV capture (excluded from sweep; own README)
├── special-validation/         separate-instrument probes (feeds git-ignored; own README + build_specials.sh)
├── data/                       reference OHLCV (committed 1m feed; 15m derived locally into data/derived/)
├── LICENSE                     Apache-2.0
├── NOTICE                      attribution
├── LEGAL.md                    provenance / trademarks
├── README.md                   this file
├── CMakeLists.txt              per-strategy .so build glob
├── .gitignore                  ignores compiled strategy libs, data/derived/, .omc/
├── validation_report.md        canonical parity disposition, regenerated each sweep
└── validation_report.{html,pdf}   rendered from .md
```

Total: **252** probes.

## Naming convention

Every probe directory follows:

```
<category>-<descriptive-slug>-NN[a-z]?
```

- **`<category>`** — one of the 24 surface categories below. The category
  is the engine surface or PineScript feature the probe is built to
  exercise.
- **`<descriptive-slug>`** — kebab-case description of the specific
  behaviour under test (e.g. `atr-trail-series-int-points`,
  `kalman-filter-1d`, `bb-kc-squeeze-release`).
- **`NN`** — two-digit sequence number, used to disambiguate when more
  than one probe lands on the same `(category, slug)` pair.
- **`[a-z]?`** — optional letter suffix, used **only** for documented
  A/B variant pairs that share the same numeric slot (e.g.
  `barstate-isconfirmed-magnifier-on-01a` vs
  `…-magnifier-off-01b`).

The 24 categories (with probe counts):

| Category    | Count | Surface exercised                                          |
| ----------- | ----: | ---------------------------------------------------------- |
| `ta`        |    61 | TA built-in math (rsi, macd, sma, hma, …)                  |
| `composite` |    52 | Multi-surface integration probes (community-style scripts) |
| `order`     |    40 | Entry/exit/cancel order placement                          |
| `udt`       |    22 | User-defined types + methods                               |
| `mtf`       |    16 | `request.security` regular HTF                             |
| `bracket`   |    13 | TP/SL via `strategy.exit` / `strategy.order`               |
| `matrix`    |     6 | `matrix<T>` typed and generic                              |
| `analyzer`  |     6 | Engine analyzer / parity isolation                         |
| `drawing`   |     6 | Drawing objects as data (`line`, `box`, `chart.point`)      |
| `pyramid`   |     4 | `pyramiding=N`                                             |
| `oca`       |     3 | OCA group cancel / reduce / none                           |
| `magnifier` |     3 | `bar_magnifier` sub-bar walks                              |
| `session`   |     2 | `session()` / TZ / DST                                     |
| `recompute` |     2 | `calc_on_every_tick` / TA recompute                        |
| `na`        |     2 | `na` propagation                                           |
| `ltf`       |     2 | `request.security_lower_tf` arrays                         |
| `input`     |     2 | `input.source` runtime override / subscript                |
| `cap`       |     2 | Intraday cap (`max_intraday_filled_orders`)                |
| `barstate`  |     2 | `barstate.*` checks                                        |
| `vwap`      |     2 | VWAP band pricing / fills                                  |
| `risk`      |     1 | risk gates / limits                                        |
| `stats`     |     1 | performance stats / reporting                              |
| `timeframe` |     1 | script_tf/input_tf timeframe handling                      |
| `anomaly`   |     1 | Documented TV non-determinism                              |

(The `symbol-specified/` subtree — 5 stock probes needing per-symbol OHLCV
and SymInfo overrides — is excluded from the default sweep pending
pineforge-data integration; it is not counted in the 252.)

## Where the numbers come from

The headline figure is produced by the verifier sweep that emits
[`validation_report.md`](validation_report.md). That report is the
authoritative disposition for every probe — tier, profile, per-dimension
deltas, anomaly verdicts.

The full pipeline (build + run + verify across the whole tree) is one
command:

```bash
JOBS=8 scripts/run_corpus.sh
```

That script:

1. Configures CMake with `-DPINEFORGE_BUILD_CORPUS_STRATEGIES=ON`.
2. Builds `libpineforge.a` plus one `strategy.so` per probe via
   `cmake --build build --target corpus_strategies`.
3. Loads each `strategy.so` through `scripts/run_strategy.py`, runs it
   against the 15m chart feed derived from
   `corpus/data/ohlcv_ETH-USDT-USDT_1m.csv`, and writes
   `engine_trades.csv` next to the probe.
4. Runs `scripts/verify_corpus.py --all` to produce the report.

## Reproducing parity locally

No transpiler access required — `generated.cpp` ships in-tree.

```bash
# 1. Clone the engine and pull this corpus submodule
git clone https://github.com/pineforge-4pass/pineforge-engine.git
cd pineforge-engine
git submodule update --init corpus

# 2. Build all per-strategy .so files, run them, and verify
JOBS=8 scripts/run_corpus.sh
```

You need the engine repo, this corpus, and a C++17 compiler. The engine
is deterministic given a fixed bar feed, the shipped `generated.cpp`,
and a fixed runtime build. If a probe's rebuilt `engine_trades.csv`
disagrees with the committed copy, that is a bug — please open an issue.

## CSV format

Both `tv_trades.csv` and `engine_trades.csv` use TradingView's row layout:

- **Two rows per trade**, sharing the same `Trade #`. The exit row is
  emitted before the entry row (TV convention; PineForge mirrors it for
  direct diff).
- **Reverse-chronological by trade number** (newest first).
- **Time format**: `YYYY-MM-DD HH:MM`. Engine CSVs are UTC. TradingView
  exports use the chart's wall-clock timezone; this corpus defaults to
  UTC+8 unless a probe `inputs.json` overrides `tv_trades_csv_tz`.

`tv_trades.csv` (TradingView's actual export):

```
Trade #,Type,Date and time,Signal,Price USDT,Position size (qty),...
14,Exit long,2026-04-27 20:30,TPSL,2291.52,1,2291.52,...
14,Entry long,2026-04-27 20:15,…,2289.7,1,2289.7,…
```

`engine_trades.csv` (PineForge's mirrored format, fewer columns —
PineForge does not currently emit TV's "Signal" tag or percent-of-position
excursions):

```
Trade #,Type,Date and time,Price,Qty,Net PnL,Net PnL %,Favorable excursion USD,Adverse excursion USD,Cumulative PnL
14,Exit long,2026-04-27 20:30,2291.520000,1,1.820000,0.0795,2.250000,-0.160000,-38.120000
14,Entry long,2026-04-27 20:15,2289.700000,1,1.820000,0.0795,2.250000,-0.160000,-38.120000
```

`Net PnL` and `Net PnL %` are per-trade. `Cumulative PnL` is the
engine-side running total. The excursion columns use TV's names and sign
convention: favorable excursion is a non-negative total-USD run-up,
adverse excursion is a **negative** total-USD drawdown
(`(price diff) × qty`, summed over pyramid entries). Note this is the
export convention only — Pine's `strategy.*trades.max_drawdown`
accessors stay positive per the Pine v6 spec.

## Parity thresholds

The verifier (`scripts/verify_corpus.py`) applies one of two threshold
profiles per probe and emits a tier label:

### Profiles

| Dimension                  | STRICT  | PRODUCTION |
| -------------------------- | ------: | ---------: |
| Absolute trade-count parity | Δ == 0 |     Δ == 0 |
| Coverage (matched / all closed TV trades) | ≥99% | ≥99% |
| Entry-price p90 delta      |   0.01% |      0.01% |
| Exit-price p90 delta       |   0.01% |      0.05% |
| Per-trade P&L p90 delta    |   1.0%  |       100% |

PRODUCTION relaxes the exit-price tolerance (5×) and the per-trade P&L
tolerance (~100× — its 100% gate catches only catastrophic divergence)
to absorb sub-bar broker-side fill drift on trailing exits. The verifier
auto-selects PRODUCTION for probes whose `strategy.pine` uses any
`trail_*` parameter on `strategy.exit` (plain stop/limit exits stay
STRICT); an `inputs.json` `parity_profile` override wins over
auto-detection.

Excursion metrics (MAE/MFE) are **report-only diagnostics**, not gates:
they pin TV's excursion conventions (sign, total-USD scaling, exit-fill
inclusion) and flag regressions loudly (a sign error reads ~200%), but
same-bar stop/limit round-trips carry TV-side excursions sourced from
intrabar data that chart-TF OHLC cannot reproduce (the engine correctly
emits 0), so excursions can never be a pass/fail dimension.

A trade is "matched" when engine and TV agree on direction and entry/
exit times fall within a 1-hour gating window (plus a $3 entry-price
gate to defend against same-bar duplicates). The PnL p90 calc applies a
near-zero filter (`|tv_pnl| > $0.01`) to avoid div-by-near-zero blow-up
on TV's magnifier zero-PnL trades.

### Tier labels

| Tier          | Meaning |
| ------------- | ------- |
| `excellent`   | All gated dimensions (absolute count parity, coverage, entry, exit, P&L) pass the resolved profile. Bit-for-bit or within strict-profile thresholds. |
| `strong`      | Dimensions pass a relaxed envelope (count <6%, entry 10×, exit 50×, P&L 100%, coverage ≥95%) — close but not excellent. Used as a pass-with-caveat tier. |
| `moderate`    | Some dimensions exceed the strong envelope but trades still align meaningfully. Investigate. |
| `weak`        | Significant divergence. Real bug or probe-design issue. |
| `minimal`     | Probe produces zero engine trades or zero TV trades — nothing to compare. |
| `anomaly`     | Engine output is correct per Pine spec; TV is non-deterministic on this probe. Documented per-probe via `inputs.json::expected_tier: "anomaly"` plus a `notes` field with the deep-analysis write-up. Excluded from headline excellent count. Currently 1 probe (`anomaly-equity-mirror-strategy-equity-01`). |
| `engine_only` | Engine produces correct trades that intentionally diverge from TV (e.g., engine fires a bar TV's broker emulator silently drops). Documented per-probe via `inputs.json::validation_overrides::expect_tv_match: false` plus an `expect_tv_match_reason` write-up. Currently 0 probes. |
| `missing`     | Required artefact (TV CSV or engine CSV) absent. Should never appear in committed state. |

The `anomaly` and `engine_only` overrides only fire when the computed
tier would be below `excellent` — a future engine fix that lifts a
documented divergence to bit-for-bit match still reports as `excellent`,
not silently masked.

## Publishing posture

The corpus is published under **Apache-2.0**, matching the engine. Every
`strategy.pine` is a clean-room PineForge original — no third-party
PineScript is redistributed. TradingView trade-list CSVs are factual
records of running each script on TV's broker emulator, included only for
parity verification. OHLCV is public market data from Binance USDT-M
futures. See [`LEGAL.md`](LEGAL.md) for the full provenance and trademark
notes.
