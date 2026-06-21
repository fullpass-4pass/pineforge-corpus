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

class GeneratedStrategy : public BacktestEngine {
public:
    ta::PivotHigh _ta_pivothigh_1;
    std::vector<double> _precalc__ta_pivothigh_1;
    ta::PivotLow _ta_pivotlow_2;
    std::vector<double> _precalc__ta_pivotlow_2;
    ta::EMA _ta_ema_3;
    std::vector<double> _precalc__ta_ema_3;
    ta::EMA _ta_ema_4;
    std::vector<double> _precalc__ta_ema_4;
    ta::EMA _ta_ema_5;
    std::vector<double> _precalc__ta_ema_5;
    ta::RSI _ta_rsi_6;
    std::vector<double> _precalc__ta_rsi_6;
    ta::Highest _ta_highest_7;
    std::vector<double> _precalc__ta_highest_7;
    ta::Lowest _ta_lowest_8;
    std::vector<double> _precalc__ta_lowest_8;
    bool _use_precalc = false;
    double last_ph;
    double last_pl;
    int last_ph_x;
    int last_pl_x;
    double prev_ph_y;
    int prev_ph_x;
    double prev_pl_y;
    int prev_pl_x;
    Series<double> hi_now;
    Series<double> lo_now;
    int i_pivot = 0;
    int i_ema_fast = 0;
    int i_ema_mid = 0;
    int i_ema_slow = 0;
    int i_rsi_len = 0;
    double i_rsi_lo = 0.0;
    double i_rsi_hi = 0.0;
    int i_break_len = 0;
    double i_rr = 0.0;
    double ph = 0.0;
    double pl = 0.0;
    double cur_x = 0.0;
    double ema_fast = 0.0;
    double ema_mid = 0.0;
    double ema_slow = 0.0;
    bool stack_bull = false;
    bool stack_bear = false;
    bool trend_bull = false;
    bool trend_bear = false;
    double r = 0.0;
    bool mom_bull = false;
    bool mom_bear = false;
    double break_hi = 0.0;
    double break_lo = 0.0;
    bool struct_bull = false;
    bool struct_bear = false;
    bool gate_long = false;
    bool gate_short = false;
    bool go_long = false;
    bool go_short = false;
    double entry_px = 0.0;
    double sl_px = 0.0;
    double tp_px = 0.0;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_pivothigh_1(5, 5), _ta_pivotlow_2(5, 5), _ta_ema_3(21), _ta_ema_4(55), _ta_ema_5(200), _ta_rsi_6(14), _ta_highest_7(20), _ta_lowest_8(20), last_ph(na<double>()), last_pl(na<double>()), last_ph_x(na<double>()), last_pl_x(na<double>()), prev_ph_y(na<double>()), prev_ph_x(na<double>()), prev_pl_y(na<double>()), prev_pl_x(na<double>()) {
        initial_capital_ = 1000000.0;
        default_qty_type_ = QtyType::FIXED;
        default_qty_value_ = 1.0;
        pyramiding_ = 1;
        commission_type_ = CommissionType::PERCENT;
        commission_value_ = 0.0;
        slippage_ = 0;
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
        if (!_var_initialized) {
            _var_initialized = true;
        } else {
        }
        if (!_inputs_initialized_) {
            i_pivot = get_input_int("Pivot strength", 5);
            i_ema_fast = get_input_int("EMA fast", 21);
            i_ema_mid = get_input_int("EMA mid", 55);
            i_ema_slow = get_input_int("EMA slow", 200);
            i_rsi_len = get_input_int("RSI length", 14);
            i_rsi_lo = get_input_double("RSI long threshold", 55);
            i_rsi_hi = get_input_double("RSI short threshold", 45);
            i_break_len = get_input_int("Breakout window", 20);
            i_rr = get_input_double("Reward:risk ratio", 2.0);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_pivothigh_1 = ta::PivotHigh(get_input_int("Pivot strength", 5), get_input_int("Pivot strength", 5));
            _ta_pivotlow_2 = ta::PivotLow(get_input_int("Pivot strength", 5), get_input_int("Pivot strength", 5));
            _ta_ema_3 = ta::EMA(get_input_int("EMA fast", 21));
            _ta_ema_4 = ta::EMA(get_input_int("EMA mid", 55));
            _ta_ema_5 = ta::EMA(get_input_int("EMA slow", 200));
            _ta_rsi_6 = ta::RSI(get_input_int("RSI length", 14));
            _ta_highest_7 = ta::Highest(get_input_int("Breakout window", 20));
            _ta_lowest_8 = ta::Lowest(get_input_int("Breakout window", 20));
            _ta_initialized_ = true;
        }
        ph = (is_first_tick_ ? _ta_pivothigh_1.compute(current_bar_.high) : _ta_pivothigh_1.recompute(current_bar_.high));
        pl = (is_first_tick_ ? _ta_pivotlow_2.compute(current_bar_.low) : _ta_pivotlow_2.recompute(current_bar_.low));
        if (!(is_na(ph))) {
            cur_x = (bar_index_ - i_pivot);
            if ((!(is_na(last_ph)) && !(is_na(last_ph_x)))) {
            }
            prev_ph_y = last_ph;
            prev_ph_x = last_ph_x;
            last_ph = ph;
            last_ph_x = cur_x;
        }
        if (!(is_na(pl))) {
            cur_x = (bar_index_ - i_pivot);
            if ((!(is_na(last_pl)) && !(is_na(last_pl_x)))) {
            }
            prev_pl_y = last_pl;
            prev_pl_x = last_pl_x;
            last_pl = pl;
            last_pl_x = cur_x;
        }
        ema_fast = (is_first_tick_ ? _ta_ema_3.compute(current_bar_.close) : _ta_ema_3.recompute(current_bar_.close));
        ema_mid = (is_first_tick_ ? _ta_ema_4.compute(current_bar_.close) : _ta_ema_4.recompute(current_bar_.close));
        ema_slow = (is_first_tick_ ? _ta_ema_5.compute(current_bar_.close) : _ta_ema_5.recompute(current_bar_.close));
        stack_bull = ((ema_fast > ema_mid) && (ema_mid > ema_slow));
        stack_bear = ((ema_fast < ema_mid) && (ema_mid < ema_slow));
        trend_bull = (ema_fast > ema_mid);
        trend_bear = (ema_fast < ema_mid);
        r = (is_first_tick_ ? _ta_rsi_6.compute(current_bar_.close) : _ta_rsi_6.recompute(current_bar_.close));
        mom_bull = (r > i_rsi_lo);
        mom_bear = (r < i_rsi_hi);
        hi_now.push((is_first_tick_ ? _ta_highest_7.compute(current_bar_.high) : _ta_highest_7.recompute(current_bar_.high)));
        lo_now.push((is_first_tick_ ? _ta_lowest_8.compute(current_bar_.low) : _ta_lowest_8.recompute(current_bar_.low)));
        break_hi = hi_now[1];
        break_lo = lo_now[1];
        struct_bull = (current_bar_.close > break_hi);
        struct_bear = (current_bar_.close < break_lo);
        gate_long = ((trend_bull && mom_bull) && struct_bull);
        gate_short = ((trend_bear && mom_bear) && struct_bear);
        go_long = ((stack_bull && gate_long) && !(is_na(last_pl)));
        go_short = ((stack_bear && gate_short) && !(is_na(last_ph)));
        if ((go_long && (signed_position_size() <= 0))) {
            strategy_entry(std::string("L"), true, na<double>(), na<double>(), 1, std::string("integ trend long"), "", 0, -1);
        }
        if ((go_short && (signed_position_size() >= 0))) {
            strategy_entry(std::string("S"), false, na<double>(), na<double>(), 1, std::string("integ trend short"), "", 0, -1);
        }
        entry_px = position_entry_price_;
        if (((signed_position_size() > 0) && !(is_na(last_pl)))) {
            sl_px = last_pl;
            tp_px = (entry_px + ((entry_px - last_pl) * i_rr));
            strategy_exit(std::string("Brk"), std::string("L"), tp_px, sl_px, na<double>(), na<double>(), na<double>(), 100.0, "", na<double>(), "");
        }
        if (((signed_position_size() < 0) && !(is_na(last_ph)))) {
            sl_px = last_ph;
            tp_px = (entry_px - ((last_ph - entry_px) * i_rr));
            strategy_exit(std::string("Brk"), std::string("S"), tp_px, sl_px, na<double>(), na<double>(), na<double>(), 100.0, "", na<double>(), "");
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_pivothigh_1.resize(n);
        _precalc__ta_pivotlow_2.resize(n);
        _precalc__ta_ema_3.resize(n);
        _precalc__ta_ema_4.resize(n);
        _precalc__ta_ema_5.resize(n);
        _precalc__ta_rsi_6.resize(n);
        _precalc__ta_highest_7.resize(n);
        _precalc__ta_lowest_8.resize(n);

        _ta_pivothigh_1 = ta::PivotHigh(5, 5);
        _ta_pivotlow_2 = ta::PivotLow(5, 5);
        _ta_ema_3 = ta::EMA(21);
        _ta_ema_4 = ta::EMA(55);
        _ta_ema_5 = ta::EMA(200);
        _ta_rsi_6 = ta::RSI(14);
        _ta_highest_7 = ta::Highest(20);
        _ta_lowest_8 = ta::Lowest(20);


        for (int i = 0; i < n; ++i) {
            _precalc__ta_pivothigh_1[i] = _ta_pivothigh_1.compute(bars[i].high);
            _precalc__ta_pivotlow_2[i] = _ta_pivotlow_2.compute(bars[i].low);
            _precalc__ta_ema_3[i] = _ta_ema_3.compute(bars[i].close);
            _precalc__ta_ema_4[i] = _ta_ema_4.compute(bars[i].close);
            _precalc__ta_ema_5[i] = _ta_ema_5.compute(bars[i].close);
            _precalc__ta_rsi_6[i] = _ta_rsi_6.compute(bars[i].close);
            _precalc__ta_highest_7[i] = _ta_highest_7.compute(bars[i].high);
            _precalc__ta_lowest_8[i] = _ta_lowest_8.compute(bars[i].low);
        }

        _ta_pivothigh_1 = ta::PivotHigh(5, 5);
        _ta_pivotlow_2 = ta::PivotLow(5, 5);
        _ta_ema_3 = ta::EMA(21);
        _ta_ema_4 = ta::EMA(55);
        _ta_ema_5 = ta::EMA(200);
        _ta_rsi_6 = ta::RSI(14);
        _ta_highest_7 = ta::Highest(20);
        _ta_lowest_8 = ta::Lowest(20);

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
