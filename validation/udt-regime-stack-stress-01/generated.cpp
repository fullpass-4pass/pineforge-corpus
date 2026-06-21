#include <pineforge/engine.hpp>
#include <pineforge/ta.hpp>
#include <pineforge/math.hpp>
#include <pineforge/series.hpp>
#include <pineforge/na.hpp>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_map>
#include <pineforge/color.hpp>
#include <pineforge/log.hpp>
#include <pineforge/str_utils.hpp>
#include <pineforge/session_time.hpp>

using namespace pineforge;

// --- syminfo derivation helpers (PineForge G2) ---
static inline std::string _pf_derive_prefix(const std::string& tickerid) {
    std::size_t colon = tickerid.find(':');
    return (colon == std::string::npos) ? tickerid : tickerid.substr(0, colon);
}

static inline std::string _pf_derive_main_tickerid(const std::string& tickerid) {
    // Strip trailing digits (optionally followed by '!') from the symbol part.
    // e.g. "CME_MINI:ES1!" -> "CME_MINI:ES", "NYMEX:CL2!" -> "NYMEX:CL"
    std::string result = tickerid;
    std::size_t colon = result.find(':');
    std::size_t start = (colon == std::string::npos) ? 0 : colon + 1;
    // Find end of base symbol (strip trailing digits + optional '!')
    std::size_t end = result.size();
    if (end > start && result[end - 1] == '!') {
        --end;
    }
    while (end > start && std::isdigit((unsigned char)result[end - 1])) {
        --end;
    }
    return result.substr(0, end);
}

static inline std::string _pf_derive_country(const std::string& tickerid) {
    // Lookup country by exchange prefix (text before ':').
    std::size_t colon = tickerid.find(':');
    std::string prefix = (colon == std::string::npos)
        ? tickerid : tickerid.substr(0, colon);
    static const std::unordered_map<std::string, std::string> _tbl = {
        {"AMEX", "US"},
        {"AQUIS", "GB"},
        {"ARCA", "US"},
        {"ASX", "AU"},
        {"B3", "BR"},
        {"BMF", "BR"},
        {"BMFBOVESPA", "BR"},
        {"BSE", "IN"},
        {"CBOE", "US"},
        {"CBOT", "US"},
        {"CME", "US"},
        {"CME_MINI", "US"},
        {"COINBASE", "US"},
        {"COMEX", "US"},
        {"HKEX", "HK"},
        {"JSE", "ZA"},
        {"KOSPI", "KR"},
        {"KRX", "KR"},
        {"LSE", "GB"},
        {"MOEX", "RU"},
        {"NASDAQ", "US"},
        {"NSE", "IN"},
        {"NYMEX", "US"},
        {"NYSE", "US"},
        {"OSE", "JP"},
        {"OTC", "US"},
        {"SGX", "SG"},
        {"SIX", "CH"},
        {"SSE", "CN"},
        {"SZSE", "CN"},
        {"TSE", "JP"},
        {"TSX", "CA"},
        {"UPBIT", "KR"},
        {"VENTURE", "CA"},
        {"XETRA", "DE"}
    };
    auto it = _tbl.find(prefix);
    return (it != _tbl.end()) ? it->second : na<std::string>();
}
// --- end syminfo derivation helpers ---

struct LayerInputs {
    int rsi_len = 14;
    int ma_len = 20;
    int bb_len = 20;
    double bb_mult = 2.0;
    static LayerInputs create() { return LayerInputs{}; }
};

struct RsiLayer {
    double value = na<double>();
    int contrib = 0;
    static RsiLayer create() { return RsiLayer{}; }
};

struct TrendLayer {
    double ema = na<double>();
    int contrib = 0;
    static TrendLayer create() { return TrendLayer{}; }
};

struct BbLayer {
    double mid = na<double>();
    double upper = na<double>();
    double lower = na<double>();
    int contrib = 0;
    static BbLayer create() { return BbLayer{}; }
};

struct ScoreSnapshot {
    int rsi_c = 0;
    int trend_c = 0;
    int bb_c = 0;
    static ScoreSnapshot create() { return ScoreSnapshot{}; }
};

struct VolContext {
    double atr = na<double>();
    double atr_ma = na<double>();
    double ratio = 1.0;
    static VolContext create() { return VolContext{}; }
};

struct OhlcBar {
    double o = na<double>();
    double h = na<double>();
    double l = na<double>();
    double c = na<double>();
    static OhlcBar create() { return OhlcBar{}; }
};

struct SessionScratch {
    bool in_session = true;
    int bar_streak = 0;
    static SessionScratch create() { return SessionScratch{}; }
};

struct GateState {
    bool vol_ok = true;
    bool map_ok = true;
    static GateState create() { return GateState{}; }
};

const int Aggression_conservative = 0;
const int Aggression_balanced = 1;
const int Aggression_aggressive = 2;
static const std::string Aggression_str_values[] = {std::string("Conservative"), std::string("Balanced"), std::string("Aggressive")};

class GeneratedStrategy : public BacktestEngine {
public:
    ta::RSI _ta_rsi_1;
    std::vector<double> _precalc__ta_rsi_1;
    ta::EMA _ta_ema_2;
    std::vector<double> _precalc__ta_ema_2;
    ta::BB _ta_bb_3;
    std::vector<ta::BBResult> _precalc__ta_bb_3;
    ta::ATR _ta_atr_4;
    std::vector<double> _precalc__ta_atr_4;
    ta::SMA _ta_sma_5;
    ta::Crossover _ta_crossover_6;
    bool _use_precalc = false;
    Series<double> _s_close;
    LayerInputs cfg;
    double thr_long;
    double thr_short;
    std::unordered_map<std::string, double> gateMap;
    RsiLayer rsiL;
    TrendLayer trL;
    BbLayer bbL;
    ScoreSnapshot snap;
    VolContext vol;
    OhlcBar curBar;
    SessionScratch sess;
    GateState gates;
    int prevScore;
    int rsiLen = 0;
    int maLen = 0;
    int bbLen = 0;
    double bbMult = 0.0;
    double profile = 0.0;
    double rsiVal = 0.0;
    double emaVal = 0.0;
    double atrVal = 0.0;
    double atrMa = 0.0;
    double atrRatio = 0.0;
    double score = 0.0;
    bool longCond = false;
    bool shortCond = false;
    double exitL = 0.0;
    double exitS = 0.0;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_rsi_1(14), _ta_ema_2(20), _ta_bb_3(20, 2.0), _ta_atr_4(14), _ta_sma_5(20), thr_long(2), thr_short(-2), prevScore(na<double>()) {
        initial_capital_ = 1000000.0;
        default_qty_type_ = QtyType::FIXED;
        default_qty_value_ = 1.0;
        pyramiding_ = 1;
        commission_type_ = CommissionType::PERCENT;
        commission_value_ = 0.0;
        slippage_ = 0;
        script_has_strategy_close_ = true;
    }

    void set_strategy_override(const std::string& key, const std::string& value) {
        if (key == "initial_capital") { initial_capital_ = std::stod(value); return; }
        if (key == "commission_value") { commission_value_ = std::stod(value); return; }
        if (key == "default_qty_value") { default_qty_value_ = std::stod(value); return; }
        if (key == "pyramiding") { pyramiding_ = std::stoi(value); return; }
        if (key == "slippage") { slippage_ = std::stoi(value); return; }
        if (key == "process_orders_on_close") { process_orders_on_close_ = (value == "true" || value == "1"); return; }
        if (key == "close_entries_rule") { close_entries_rule_any_ = (value == "ANY" || value == "any" || value == "1"); return; }
        if (key == "default_qty_type") {
            if (value == "fixed" || value == "strategy.fixed" || value == "0") default_qty_type_ = QtyType::FIXED;
            else if (value == "percent_of_equity" || value == "strategy.percent_of_equity" || value == "1") default_qty_type_ = QtyType::PERCENT_OF_EQUITY;
            else if (value == "cash" || value == "strategy.cash" || value == "2") default_qty_type_ = QtyType::CASH;
            return;
        }
        if (key == "commission_type") {
            if (value == "percent" || value == "strategy.commission.percent" || value == "0") commission_type_ = CommissionType::PERCENT;
            else if (value == "cash_per_order" || value == "strategy.commission.cash_per_order" || value == "1") commission_type_ = CommissionType::CASH_PER_ORDER;
            else if (value == "cash_per_contract" || value == "strategy.commission.cash_per_contract" || value == "2") commission_type_ = CommissionType::CASH_PER_CONTRACT;
            return;
        }
    }

    void on_bar(const Bar& bar) override {
        if (is_first_tick_) _s_close.push(current_bar_.close);
        else _s_close.update(current_bar_.close);
        if (!_var_initialized) {
            cfg = LayerInputs{.rsi_len = rsiLen, .ma_len = maLen, .bb_len = bbLen, .bb_mult = bbMult};
            gateMap = std::unordered_map<std::string, double>();
            rsiL = RsiLayer{.value = na<double>(), .contrib = 0};
            trL = TrendLayer{.ema = na<double>(), .contrib = 0};
            bbL = BbLayer{.mid = na<double>(), .upper = na<double>(), .lower = na<double>(), .contrib = 0};
            snap = ScoreSnapshot{.rsi_c = 0, .trend_c = 0, .bb_c = 0};
            vol = VolContext{.atr = na<double>(), .atr_ma = na<double>(), .ratio = 1.0};
            curBar = OhlcBar{.o = na<double>(), .h = na<double>(), .l = na<double>(), .c = na<double>()};
            sess = SessionScratch{.in_session = true, .bar_streak = 0};
            gates = GateState{.vol_ok = true, .map_ok = true};
            _var_initialized = true;
        } else {
        }
        if (!_inputs_initialized_) {
            rsiLen = get_input_int("RSI Length", 14);
            maLen = get_input_int("MA Length", 20);
            bbLen = get_input_int("BB Length", 20);
            bbMult = get_input_double("BB Mult", 2.0);
            profile = get_input_int("Stress profile", Aggression_balanced);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_rsi_1 = ta::RSI(get_input_int("RSI Length", 14));
            _ta_ema_2 = ta::EMA(get_input_int("MA Length", 20));
            _ta_bb_3 = ta::BB(get_input_int("BB Length", 20), get_input_double("BB Mult", 2.0));
            _ta_initialized_ = true;
        }
        if ((bar_index_ == 0)) {
            (gateMap[std::string("long")] = 2.0);
            (gateMap[std::string("short")] = (-2.0));
            (gateMap[std::string("exit_long")] = 0.0);
            (gateMap[std::string("exit_short")] = 0.0);
        }
        auto __switch_val_0 = profile;
        if (__switch_val_0 == Aggression_conservative) {
            thr_long = (is_na((gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0)) ? 2.0 : (gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0));
            thr_short = (is_na((gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0)) ? (-2.0) : (gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0));
        }
        else if (__switch_val_0 == Aggression_balanced) {
            thr_long = (is_na((gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0)) ? 2.0 : (gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0));
            thr_short = (is_na((gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0)) ? (-2.0) : (gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0));
        }
        else if (__switch_val_0 == Aggression_aggressive) {
            thr_long = (is_na((gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0)) ? 2.0 : (gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0));
            thr_short = (is_na((gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0)) ? (-2.0) : (gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0));
        }
        else {
            thr_long = (is_na((gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0)) ? 2.0 : (gateMap.count(std::string("long")) ? gateMap[std::string("long")] : 0.0));
            thr_short = (is_na((gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0)) ? (-2.0) : (gateMap.count(std::string("short")) ? gateMap[std::string("short")] : 0.0));
        }
        rsiVal = (is_first_tick_ ? _ta_rsi_1.compute(current_bar_.close) : _ta_rsi_1.recompute(current_bar_.close));
        emaVal = (is_first_tick_ ? _ta_ema_2.compute(current_bar_.close) : _ta_ema_2.recompute(current_bar_.close));
        auto _result__ta_bb_3 = (is_first_tick_ ? _ta_bb_3.compute(current_bar_.close) : _ta_bb_3.recompute(current_bar_.close));
        double bbMid = _result__ta_bb_3.middle;
        double bbUpper = _result__ta_bb_3.upper;
        double bbLower = _result__ta_bb_3.lower;
        atrVal = (is_first_tick_ ? _ta_atr_4.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_4.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        atrMa = (is_first_tick_ ? _ta_sma_5.compute(atrVal) : _ta_sma_5.recompute(atrVal));
        atrRatio = (((atrMa > 0)) ? (((double)(atrVal) / (double)(atrMa))) : (1.0));
        cfg.rsi_len = rsiLen;
        cfg.ma_len = maLen;
        cfg.bb_len = bbLen;
        cfg.bb_mult = bbMult;
        rsiL.value = rsiVal;
        rsiL.contrib = (((rsiVal > 50)) ? (1) : ((((rsiVal < 50)) ? ((-1)) : (0))));
        trL.ema = emaVal;
        trL.contrib = (((current_bar_.close > emaVal)) ? (1) : ((-1)));
        bbL.mid = bbMid;
        bbL.upper = bbUpper;
        bbL.lower = bbLower;
        bbL.contrib = (((current_bar_.close > bbMid)) ? (1) : ((-1)));
        vol.atr = atrVal;
        vol.atr_ma = atrMa;
        vol.ratio = atrRatio;
        curBar.o = current_bar_.open;
        curBar.h = current_bar_.high;
        curBar.l = current_bar_.low;
        curBar.c = current_bar_.close;
        gates.vol_ok = (vol.ratio < 25.0);
        gates.map_ok = ((gateMap.count(std::string("long")) > 0) && (gateMap.count(std::string("short")) > 0));
        score = ((rsiL.contrib + trL.contrib) + bbL.contrib);
        snap.rsi_c = rsiL.contrib;
        snap.trend_c = trL.contrib;
        snap.bb_c = bbL.contrib;
        sess.in_session = true;
        sess.bar_streak = ((((bar_index_ > 0) && (current_bar_.close == _s_close[1]))) ? ((sess.bar_streak + 1)) : (0));
        longCond = ((((gates.vol_ok && gates.map_ok) && (score >= thr_long)) && !(is_na(prevScore))) && (prevScore < thr_long));
        shortCond = ((((gates.vol_ok && gates.map_ok) && (score <= thr_short)) && !(is_na(prevScore))) && (prevScore > thr_short));
        if (longCond) {
            strategy_entry(std::string("Long"), true, na<double>(), na<double>(), na<double>(), "");
        }
        if (shortCond) {
            strategy_entry(std::string("Short"), false, na<double>(), na<double>(), na<double>(), "");
        }
        exitL = (is_na((gateMap.count(std::string("exit_long")) ? gateMap[std::string("exit_long")] : 0.0)) ? 0.0 : (gateMap.count(std::string("exit_long")) ? gateMap[std::string("exit_long")] : 0.0));
        exitS = (is_na((gateMap.count(std::string("exit_short")) ? gateMap[std::string("exit_short")] : 0.0)) ? 0.0 : (gateMap.count(std::string("exit_short")) ? gateMap[std::string("exit_short")] : 0.0));
        if (((signed_position_size() > 0) && (score <= exitL))) {
            strategy_close(std::string("Long"), "", na<double>(), na<double>(), false);
        }
        if (((signed_position_size() < 0) && (score >= exitS))) {
            strategy_close(std::string("Short"), "", na<double>(), na<double>(), false);
        }
        prevScore = score;
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_rsi_1.resize(n);
        _precalc__ta_ema_2.resize(n);
        _precalc__ta_bb_3.resize(n);
        _precalc__ta_atr_4.resize(n);

        _ta_rsi_1 = ta::RSI(14);
        _ta_ema_2 = ta::EMA(20);
        _ta_bb_3 = ta::BB(20, 2.0);
        _ta_atr_4 = ta::ATR(14);

        _s_close.clear();

        for (int i = 0; i < n; ++i) {
            _s_close.push(bars[i].close);
            _precalc__ta_rsi_1[i] = _ta_rsi_1.compute(bars[i].close);
            _precalc__ta_ema_2[i] = _ta_ema_2.compute(bars[i].close);
            _precalc__ta_bb_3[i] = _ta_bb_3.compute(bars[i].close);
            _precalc__ta_atr_4[i] = _ta_atr_4.compute(bars[i].high, bars[i].low, bars[i].close);
        }

        _ta_rsi_1 = ta::RSI(14);
        _ta_ema_2 = ta::EMA(20);
        _ta_bb_3 = ta::BB(20, 2.0);
        _ta_atr_4 = ta::ATR(14);
        _s_close.clear();

        _use_precalc = true;
    }

    void run(const Bar* bars, int n) {
        precalculate(bars, n);
        BacktestEngine::run(bars, n);
    }

    void run(const Bar* input_bars, int n_input,
             const std::string& input_tf,
             const std::string& script_tf,
             bool bar_magnifier = false,
             int magnifier_samples = 4,
             MagnifierDistribution magnifier_dist = MagnifierDistribution::ENDPOINTS) {
        bool needs_dynamic = bar_magnifier || !input_tf.empty() || !script_tf.empty();
        if (needs_dynamic) {
            _use_precalc = false;
        } else {
            precalculate(input_bars, n_input);
        }
        BacktestEngine::run(input_bars, n_input, input_tf, script_tf, bar_magnifier, magnifier_samples, magnifier_dist);
    }

};

extern "C" {
    void* strategy_create(const char* params_json) {
        return new GeneratedStrategy();
    }
    void run_backtest(void* s, Bar* bars, int n, ReportC* out) {
        auto* strat = static_cast<GeneratedStrategy*>(s);
        strat->run(bars, n);
        strat->fill_report(out);
    }
    void run_backtest_full(void* s, Bar* bars, int n,
                           const char* input_tf, const char* script_tf,
                           int bar_magnifier, int magnifier_samples,
                           int magnifier_dist,
                           ReportC* out) {
        auto* strat = static_cast<GeneratedStrategy*>(s);
        std::string itf = input_tf ? input_tf : "";
        std::string stf = script_tf ? script_tf : "";
        bool needs_full_run = (bar_magnifier != 0)
            || !itf.empty() || !stf.empty();
        if (!needs_full_run) {
            strat->run(bars, n);
        } else {
            strat->run(bars, n, itf, stf, bar_magnifier != 0, magnifier_samples,
                       static_cast<MagnifierDistribution>(magnifier_dist));
        }
        strat->fill_report(out);
    }
    void strategy_free(void* s) {
        delete static_cast<GeneratedStrategy*>(s);
    }
    void report_free(ReportC* report) {
        BacktestEngine::free_report(report);
    }
    void strategy_set_input(void* s, const char* key, const char* value) {
        if (!s || !key || !value) return;
        static_cast<GeneratedStrategy*>(s)->set_input(key, value);
    }
    void strategy_set_override(void* s, const char* key, const char* value) {
        if (!s || !key || !value) return;
        static_cast<GeneratedStrategy*>(s)->set_strategy_override(key, value);
    }
    void strategy_set_magnifier_volume_weighted(void* s, int on) {
        if (!s) return;
        static_cast<GeneratedStrategy*>(s)->set_magnifier_volume_weighted(on != 0);
    }
}
