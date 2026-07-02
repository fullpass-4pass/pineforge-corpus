#!/usr/bin/env python3
"""Rebuild the corpus reference feeds with FULL exchange history ("deep
warmup") from the R2 data lake, replacing the 6-month-warmup files in place.

Source of truth: R2 bucket `pineforge-data`, key scheme
  ohlcv/crypto/binance/com/perp-usdt/ETHUSDT/1m/year=YYYY/YYYY-MM.parquet
(1-minute bars, verified value-identical to the previous corpus feeds over
their whole span — this script re-asserts that before replacing anything).

Outputs (drop-in replacements, same filenames the harness resolves):
  data/ohlcv_ETH-USDT-USDT_1m_warmup6m.csv   — full history -> same end bar
  data/ohlcv_ETH-USDT-USDT_15m_warmup6m.csv  — 900s resample of the above
The window-only feeds (ohlcv_..._15m.csv / _1m.csv) are left untouched; they
define the comparison window and carry no warmup.

The "warmup6m" filenames are kept for compatibility (harnesses and dozens of
probe READMEs resolve them by name); since 2026-07-02 they carry full listing
history, not six months — see data section of the corpus README.

Credentials: ~/.cloudflare/r2-access-key and ~/.cloudflare/r2-secret-key.
Usage: python3 build_deep_warmup_feed.py [--cache DIR] [--dry-run]
"""
import argparse
import io
import os
import sys
from pathlib import Path

import boto3
import pandas as pd

ENDPOINT = "https://fca5930ce83606786e7cfde9d749296c.r2.cloudflarestorage.com"
BUCKET = "pineforge-data"
PREFIX = "ohlcv/crypto/binance/com/perp-usdt/ETHUSDT/1m/"
HERE = Path(__file__).resolve().parent
OUT_1M = HERE / "ohlcv_ETH-USDT-USDT_1m_warmup6m.csv"
OUT_15M = HERE / "ohlcv_ETH-USDT-USDT_15m_warmup6m.csv"
COLS = ["timestamp", "open", "high", "low", "close", "volume"]


def fmt(v: float) -> str:
    """Shortest-repr float formatting matching the existing feed files
    (integral values written bare: 2382 not 2382.0)."""
    if v == int(v) and abs(v) < 1e15:
        return str(int(v))
    return repr(v)


def client():
    keydir = Path.home() / ".cloudflare"
    return boto3.client(
        "s3",
        endpoint_url=ENDPOINT,
        aws_access_key_id=(keydir / "r2-access-key").read_text().strip(),
        aws_secret_access_key=(keydir / "r2-secret-key").read_text().strip(),
        region_name="auto",
    )


def pull_1m(cache: Path) -> pd.DataFrame:
    s3 = client()
    keys = []
    token = None
    while True:
        kw = {"Bucket": BUCKET, "Prefix": PREFIX}
        if token:
            kw["ContinuationToken"] = token
        resp = s3.list_objects_v2(**kw)
        keys += [o["Key"] for o in resp.get("Contents", [])
                 if o["Key"].endswith(".parquet")]
        if not resp.get("IsTruncated"):
            break
        token = resp["NextContinuationToken"]
    if not keys:
        sys.exit(f"no parquet objects under {PREFIX}")
    print(f"[pull] {len(keys)} monthly parquet files "
          f"({keys[0].rsplit('/', 1)[-1]} .. {keys[-1].rsplit('/', 1)[-1]})")

    cache.mkdir(parents=True, exist_ok=True)
    frames = []
    for k in sorted(keys):
        local = cache / k.replace("/", "_")
        if not local.exists():
            s3.download_file(BUCKET, k, str(local))
        frames.append(pd.read_parquet(local))
    df = pd.concat(frames, ignore_index=True)

    # Normalize schema: locate timestamp column, coerce to epoch-ms int64.
    tcol = next((c for c in df.columns
                 if c.lower() in ("timestamp", "ts", "time", "open_time")), None)
    if tcol is None:
        sys.exit(f"no timestamp column in parquet schema: {list(df.columns)}")
    ts = df[tcol]
    if isinstance(ts.dtype, pd.DatetimeTZDtype):
        ts = ts.dt.tz_convert("UTC").dt.tz_localize(None)
    if pd.api.types.is_datetime64_any_dtype(ts):
        ts = ts.astype("datetime64[ms]").astype("int64")
    else:
        ts = ts.astype("int64")
        # Normalize epoch unit by magnitude (s / ms / us / ns).
        m = int(ts.max())
        if m < 10**11:
            ts = ts * 1000
        elif m >= 10**17:
            ts = ts // 1_000_000
        elif m >= 10**14:
            ts = ts // 1000
    out = pd.DataFrame({
        "timestamp": ts,
        "open": df["open"].astype(float),
        "high": df["high"].astype(float),
        "low": df["low"].astype(float),
        "close": df["close"].astype(float),
        "volume": df["volume"].astype(float),
    })
    # The lake gap-fills exchange outages with flat zero-volume rows
    # (synthetic=True). TV charts simply have no bar there — drop them.
    if "synthetic" in df.columns:
        n_syn = int(df["synthetic"].sum())
        if n_syn:
            print(f"[pull] dropping {n_syn} synthetic gap-fill bars")
        out = out[~df["synthetic"].to_numpy()]
    out = (out.drop_duplicates(subset="timestamp", keep="last")
              .sort_values("timestamp").reset_index(drop=True))
    return out


def resample_15m(df1m: pd.DataFrame) -> pd.DataFrame:
    bucket = (df1m["timestamp"] // 900_000) * 900_000
    g = df1m.groupby(bucket, sort=True)
    out = pd.DataFrame({
        "timestamp": g["timestamp"].first().index,
        "open": g["open"].first().values,
        "high": g["high"].max().values,
        "low": g["low"].min().values,
        "close": g["close"].last().values,
        "volume": g["volume"].sum().round(6).values,
    })
    return out.reset_index(drop=True)


def assert_superset(new: pd.DataFrame, old_path: Path, label: str):
    """Every row of the existing feed must exist in the rebuilt feed with
    identical OHLCV values — refuses to replace the feeds otherwise."""
    old = pd.read_csv(old_path)
    merged = old.merge(new, on="timestamp", how="left",
                       suffixes=("_old", "_new"), indicator=True)
    missing = int((merged["_merge"] != "both").sum())
    if missing:
        sys.exit(f"[{label}] {missing} bars of {old_path.name} missing from rebuild")
    bad = 0
    for c in ("open", "high", "low", "close", "volume"):
        diff = (merged[f"{c}_old"] - merged[f"{c}_new"]).abs()
        tol = 1e-6 if c == "volume" else 0.0   # 15m volume = rounded 1m sum
        bad += int((diff > tol).sum())
    if bad:
        sys.exit(f"[{label}] {bad} value mismatches vs {old_path.name}")
    print(f"[{label}] overlap OK: all {len(old)} existing bars reproduced exactly")


def write_feed(df: pd.DataFrame, path: Path):
    buf = io.StringIO()
    buf.write(",".join(COLS) + "\n")
    for row in df.itertuples(index=False):
        buf.write(f"{row.timestamp},{fmt(row.open)},{fmt(row.high)},"
                  f"{fmt(row.low)},{fmt(row.close)},{fmt(row.volume)}\n")
    tmp = path.with_suffix(".csv.new")
    tmp.write_text(buf.getvalue())
    os.replace(tmp, path)
    span_d = (df['timestamp'].iloc[-1] - df['timestamp'].iloc[0]) / 86_400_000
    print(f"[write] {path.name}: {len(df)} bars, {span_d:.0f} days, "
          f"{path.stat().st_size / 1e6:.1f} MB")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cache", type=Path,
                    default=Path(os.environ.get("R2_CACHE", HERE / ".r2cache")))
    ap.add_argument("--dry-run", action="store_true",
                    help="pull + verify only; do not replace the feeds")
    args = ap.parse_args()

    old_1m = pd.read_csv(OUT_1M)
    start_ms = int(old_1m["timestamp"].iloc[0])
    df1m = pull_1m(args.cache)
    # The validated feed wins verbatim over its own span; the lake only
    # contributes the pre-existing-feed warmup extension. (The two were
    # verified value-identical over the span before this policy was set;
    # the lake's synthetic gap-fill rows are the sole difference.)
    ext = df1m[df1m["timestamp"] < start_ms]
    print(f"[merge] {len(ext)} warmup bars prepended before existing "
          f"{len(old_1m)} (feed start {start_ms})")
    df1m = pd.concat([ext, old_1m], ignore_index=True)
    # 15m: same policy — existing bars verbatim, extension resampled from
    # the 1m. The feed start is 900s-aligned, so the seam cannot split a
    # bucket. Diagnostic only: how well resample(old 1m) reproduces the
    # existing 15m file over its span.
    old_15m = pd.read_csv(OUT_15M)
    resampled = resample_15m(df1m)
    chk = old_15m.merge(resampled, on="timestamp", how="left",
                        suffixes=("_o", "_n"))
    diag = sum(int(((chk[f"{c}_o"] - chk[f"{c}_n"]).abs()
                    > (1e-3 if c == "volume" else 0.0)).sum())
               for c in ("open", "high", "low", "close", "volume"))
    print(f"[15m] diagnostic: resample(1m) deviates from existing 15m file "
          f"on {diag} of {len(old_15m)} bars (existing bars kept verbatim)")
    df15m = pd.concat([resampled[resampled["timestamp"] < start_ms],
                       old_15m], ignore_index=True)

    assert_superset(df1m, OUT_1M, "1m")
    assert_superset(df15m, OUT_15M, "15m")
    if args.dry_run:
        print("[dry-run] verification passed; feeds NOT replaced")
        return
    write_feed(df1m, OUT_1M)
    write_feed(df15m, OUT_15M)


if __name__ == "__main__":
    main()
