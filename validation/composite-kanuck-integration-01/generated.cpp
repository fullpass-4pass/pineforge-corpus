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
    math::Sum _ta_sum_1;
    std::vector<double> _precalc__ta_sum_1;
    ta::SMA _ta_sma_2;
    std::vector<double> _precalc__ta_sma_2;
    ta::RSI _ta_rsi_3;
    std::vector<double> _precalc__ta_rsi_3;
    ta::Crossover _ta_crossover_4;
    ta::Crossunder _ta_crossunder_5;
    bool _use_precalc = false;
    Series<double> _s_close{500};
    Series<double> kama{500};
    int tick_counter;
    int i_kama_len = 0;
    int i_kama_fast = 0;
    int i_kama_slow = 0;
    int i_step = 0;
    int i_sma_len = 0;
    int i_rsi_offset = 0;
    int i_deep_lag = 0;
    double change_n = 0.0;
    double vol_sum = 0.0;
    double er = 0.0;
    double fast_sc = 0.0;
    double slow_sc = 0.0;
    double sc = 0.0;
    bool gate = false;
    double long_sma = 0.0;
    double deep_rsi = 0.0;
    double deep_close = 0.0;
    bool deep_ready = false;
    bool kama_up = false;
    bool kama_down = false;
    bool go_long = false;
    bool go_short = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_sum_1(14), _ta_sma_2(400), _ta_rsi_3(14), tick_counter(0) {
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
        if (is_first_tick_) _s_close.push(current_bar_.close);
        else _s_close.update(current_bar_.close);
        if (!_var_initialized) {
            kama.push(na<double>());
            _var_initialized = true;
        } else {
            if (is_first_tick_) kama.push(kama[0]);
        }
        if (!_inputs_initialized_) {
            i_kama_len = get_input_int("KAMA length", 14);
            i_kama_fast = get_input_int("KAMA fast end", 2);
            i_kama_slow = get_input_int("KAMA slow end", 30);
            i_step = get_input_int("Tick-counter modulo", 2);
            i_sma_len = get_input_int("Long SMA length", 400);
            i_rsi_offset = get_input_int("RSI source offset", 200);
            i_deep_lag = get_input_int("Close-reference lag", 450);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_sum_1 = math::Sum(get_input_int("KAMA length", 14));
            _ta_sma_2 = ta::SMA(get_input_int("Long SMA length", 400));
            _ta_initialized_ = true;
        }
        change_n = std::abs((current_bar_.close - _s_close[i_kama_len]));
        vol_sum = (is_first_tick_ ? _ta_sum_1.compute(std::abs((current_bar_.close - _s_close[1]))) : _ta_sum_1.recompute(std::abs((current_bar_.close - _s_close[1]))));
        er = (((vol_sum > 0)) ? (((double)(change_n) / (double)(vol_sum))) : (0.0));
        fast_sc = ((double)(2.0) / (double)((i_kama_fast + 1)));
        slow_sc = ((double)(2.0) / (double)((i_kama_slow + 1)));
        sc = std::pow(((er * (fast_sc - slow_sc)) + slow_sc), 2);
        kama.update(((is_na(kama[1])) ? (current_bar_.close) : ((kama[1] + (sc * (current_bar_.close - kama[1]))))));
        tick_counter += 1;
        gate = (std::fmod((double)(tick_counter), (double)(i_step)) == 0);
        long_sma = (is_first_tick_ ? _ta_sma_2.compute(current_bar_.close) : _ta_sma_2.recompute(current_bar_.close));
        deep_rsi = (is_first_tick_ ? _ta_rsi_3.compute(_s_close[i_rsi_offset]) : _ta_rsi_3.recompute(_s_close[i_rsi_offset]));
        deep_close = _s_close[i_deep_lag];
        deep_ready = ((!(is_na(long_sma)) && !(is_na(deep_rsi))) && !(is_na(deep_close)));
        kama_up = (is_first_tick_ ? _ta_crossover_4.compute(current_bar_.close, kama[0]) : _ta_crossover_4.recompute(current_bar_.close, kama[0]));
        kama_down = (is_first_tick_ ? _ta_crossunder_5.compute(current_bar_.close, kama[0]) : _ta_crossunder_5.recompute(current_bar_.close, kama[0]));
        go_long = ((deep_ready && kama_up) && gate);
        go_short = ((deep_ready && kama_down) && gate);
        if ((go_long && (signed_position_size() <= 0))) {
            strategy_entry(std::string("L"), true, na<double>(), na<double>(), 1, std::string("integ long"), "", 0, -1);
        }
        if ((go_short && (signed_position_size() >= 0))) {
            strategy_entry(std::string("S"), false, na<double>(), na<double>(), 1, std::string("integ short"), "", 0, -1);
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_sum_1.resize(n);
        _precalc__ta_sma_2.resize(n);
        _precalc__ta_rsi_3.resize(n);

        _ta_sum_1 = math::Sum(14);
        _ta_sma_2 = ta::SMA(400);
        _ta_rsi_3 = ta::RSI(14);

        _s_close.clear();

        for (int i = 0; i < n; ++i) {
            _s_close.push(bars[i].close);
            _precalc__ta_sum_1[i] = _ta_sum_1.compute(std::abs((bars[i].close - _s_close[1])));
            _precalc__ta_sma_2[i] = _ta_sma_2.compute(bars[i].close);
            _precalc__ta_rsi_3[i] = _ta_rsi_3.compute(_s_close[i_rsi_offset]);
        }

        _ta_sum_1 = math::Sum(14);
        _ta_sma_2 = ta::SMA(400);
        _ta_rsi_3 = ta::RSI(14);
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
