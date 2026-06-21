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
    ta::Crossover _ta_crossover_3;
    ta::Crossunder _ta_crossunder_4;
    bool _use_precalc = false;
    double last_ph_y;
    int last_ph_x;
    double prev_ph_y;
    int prev_ph_x;
    double last_pl_y;
    int last_pl_x;
    double prev_pl_y;
    int prev_pl_x;
    int i_pivot = 0;
    double ph = 0.0;
    double pl = 0.0;
    double cur_x = 0.0;
    double ref_lo = 0.0;
    double slope = 0.0;
    double ref_hi = 0.0;
    bool long_signal = false;
    bool short_signal = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_pivothigh_1(5, 5), _ta_pivotlow_2(5, 5), last_ph_y(na<double>()), last_ph_x(na<double>()), prev_ph_y(na<double>()), prev_ph_x(na<double>()), last_pl_y(na<double>()), last_pl_x(na<double>()), prev_pl_y(na<double>()), prev_pl_x(na<double>()) {
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
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_pivothigh_1 = ta::PivotHigh(get_input_int("Pivot strength", 5), get_input_int("Pivot strength", 5));
            _ta_pivotlow_2 = ta::PivotLow(get_input_int("Pivot strength", 5), get_input_int("Pivot strength", 5));
            _ta_initialized_ = true;
        }
        ph = (is_first_tick_ ? _ta_pivothigh_1.compute(current_bar_.high) : _ta_pivothigh_1.recompute(current_bar_.high));
        pl = (is_first_tick_ ? _ta_pivotlow_2.compute(current_bar_.low) : _ta_pivotlow_2.recompute(current_bar_.low));
        if (!(is_na(ph))) {
            cur_x = (bar_index_ - i_pivot);
            if ((!(is_na(last_ph_y)) && !(is_na(last_ph_x)))) {
            }
            prev_ph_y = last_ph_y;
            prev_ph_x = last_ph_x;
            last_ph_y = ph;
            last_ph_x = cur_x;
        }
        if (!(is_na(pl))) {
            cur_x = (bar_index_ - i_pivot);
            if ((!(is_na(last_pl_y)) && !(is_na(last_pl_x)))) {
            }
            prev_pl_y = last_pl_y;
            prev_pl_x = last_pl_x;
            last_pl_y = pl;
            last_pl_x = cur_x;
        }
        ref_lo = na<double>();
        if (((((!(is_na(prev_pl_y)) && !(is_na(prev_pl_x))) && !(is_na(last_pl_y))) && !(is_na(last_pl_x))) && (last_pl_x != prev_pl_x))) {
            slope = ((double)((last_pl_y - prev_pl_y)) / (double)((last_pl_x - prev_pl_x)));
            ref_lo = (last_pl_y + (slope * (bar_index_ - last_pl_x)));
        }
        ref_hi = na<double>();
        if (((((!(is_na(prev_ph_y)) && !(is_na(prev_ph_x))) && !(is_na(last_ph_y))) && !(is_na(last_ph_x))) && (last_ph_x != prev_ph_x))) {
            slope = ((double)((last_ph_y - prev_ph_y)) / (double)((last_ph_x - prev_ph_x)));
            ref_hi = (last_ph_y + (slope * (bar_index_ - last_ph_x)));
        }
        long_signal = (!(is_na(ref_lo)) && (is_first_tick_ ? _ta_crossover_3.compute(current_bar_.close, ref_lo) : _ta_crossover_3.recompute(current_bar_.close, ref_lo)));
        short_signal = (!(is_na(ref_hi)) && (is_first_tick_ ? _ta_crossunder_4.compute(current_bar_.close, ref_hi) : _ta_crossunder_4.recompute(current_bar_.close, ref_hi)));
        if ((long_signal && (signed_position_size() <= 0))) {
            strategy_entry(std::string("L"), true, na<double>(), na<double>(), 1, std::string("proj long"), "", 0, -1);
        }
        if ((short_signal && (signed_position_size() >= 0))) {
            strategy_entry(std::string("S"), false, na<double>(), na<double>(), 1, std::string("proj short"), "", 0, -1);
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_pivothigh_1.resize(n);
        _precalc__ta_pivotlow_2.resize(n);

        _ta_pivothigh_1 = ta::PivotHigh(5, 5);
        _ta_pivotlow_2 = ta::PivotLow(5, 5);


        for (int i = 0; i < n; ++i) {
            _precalc__ta_pivothigh_1[i] = _ta_pivothigh_1.compute(bars[i].high);
            _precalc__ta_pivotlow_2[i] = _ta_pivotlow_2.compute(bars[i].low);
        }

        _ta_pivothigh_1 = ta::PivotHigh(5, 5);
        _ta_pivotlow_2 = ta::PivotLow(5, 5);

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
