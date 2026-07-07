# CLAUDE.md — pineforge-corpus

> Guardrails for AI agents. This repository is PUBLIC.

- **Public repo.** Never reference private repositories, scraped community
  strategy names or author handles, account-operational workflow, or private
  tooling in any doc, probe, or commit message. Every probe here is
  clean-room authored (see `LEGAL.md`).
- **Ground truth is immutable.** `tv_trades.csv` files are captured TV
  exports — never regenerate, hand-edit, or relocate them.
- `engine_trades.csv` and `validation_report.md` are regenerated ONLY via the
  engine repo's `scripts/run_corpus.sh`; regenerate the report in the same
  change as any `engine_trades.csv` refresh. Never cite the committed report
  without re-running the gate — it can lag the artifacts.
- `data/` ships exactly one committed 1m feed; chart feeds derive locally
  into `data/derived/` (git-ignored). Never add, edit, or resample OHLCV.
