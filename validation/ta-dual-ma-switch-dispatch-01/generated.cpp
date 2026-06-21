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
    ta::EMA _ta_ema_1;
    ta::SMA _ta_sma_2;
    ta::WMA _ta_wma_3;
    ta::HMA _ta_hma_4;
    ta::SMA _ta_sma_5;
    ta::EMA _ta_ema_1_cs1;
    ta::SMA _ta_sma_2_cs1;
    ta::WMA _ta_wma_3_cs1;
    ta::HMA _ta_hma_4_cs1;
    ta::SMA _ta_sma_5_cs1;
    ta::Crossover _ta_crossover_6;
    ta::Crossunder _ta_crossunder_7;
    bool _use_precalc = false;
    std::string maType = std::string("");
    int fastLen = 0;
    int slowLen = 0;
    double src = 0.0;
    double fastMA = 0.0;
    double slowMA = 0.0;
    bool longCond = false;
    bool shortCond = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_ema_1(10), _ta_sma_2(10), _ta_wma_3(10), _ta_hma_4(10), _ta_sma_5(10), _ta_ema_1_cs1(30), _ta_sma_2_cs1(30), _ta_wma_3_cs1(30), _ta_hma_4_cs1(30), _ta_sma_5_cs1(30) {
        initial_capital_ = 1000000.0;
        default_qty_type_ = QtyType::FIXED;
        default_qty_value_ = 1.0;
        pyramiding_ = 1;
        commission_type_ = CommissionType::PERCENT;
        commission_value_ = 0.0;
        slippage_ = 0;
        _src_series_active_ = true;
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

    double getMA_cs0(double source, int length) {
        double _func_ret = 0.0;
        auto __switch_val_0 = maType;
        if (__switch_val_0 == std::string("EMA")) {
            _func_ret = (is_first_tick_ ? _ta_ema_1.compute(source) : _ta_ema_1.recompute(source));
        }
        else if (__switch_val_0 == std::string("SMA")) {
            _func_ret = (is_first_tick_ ? _ta_sma_2.compute(source) : _ta_sma_2.recompute(source));
        }
        else if (__switch_val_0 == std::string("WMA")) {
            _func_ret = (is_first_tick_ ? _ta_wma_3.compute(source) : _ta_wma_3.recompute(source));
        }
        else if (__switch_val_0 == std::string("HMA")) {
            _func_ret = (is_first_tick_ ? _ta_hma_4.compute(source) : _ta_hma_4.recompute(source));
        }
        else {
            _func_ret = (is_first_tick_ ? _ta_sma_5.compute(source) : _ta_sma_5.recompute(source));
        }
        return _func_ret;
    }

    double getMA_cs1(double source, int length) {
        double _func_ret = 0.0;
        auto __switch_val_1 = maType;
        if (__switch_val_1 == std::string("EMA")) {
            _func_ret = (is_first_tick_ ? _ta_ema_1_cs1.compute(source) : _ta_ema_1_cs1.recompute(source));
        }
        else if (__switch_val_1 == std::string("SMA")) {
            _func_ret = (is_first_tick_ ? _ta_sma_2_cs1.compute(source) : _ta_sma_2_cs1.recompute(source));
        }
        else if (__switch_val_1 == std::string("WMA")) {
            _func_ret = (is_first_tick_ ? _ta_wma_3_cs1.compute(source) : _ta_wma_3_cs1.recompute(source));
        }
        else if (__switch_val_1 == std::string("HMA")) {
            _func_ret = (is_first_tick_ ? _ta_hma_4_cs1.compute(source) : _ta_hma_4_cs1.recompute(source));
        }
        else {
            _func_ret = (is_first_tick_ ? _ta_sma_5_cs1.compute(source) : _ta_sma_5_cs1.recompute(source));
        }
        return _func_ret;
    }

    void on_bar(const Bar& bar) override {
        if (!_inputs_initialized_) {
            maType = get_input_string("MA Type", std::string("EMA"));
            fastLen = get_input_int("Fast Length", 10);
            slowLen = get_input_int("Slow Length", 30);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_ema_1 = ta::EMA(get_input_int("Fast Length", 10));
            _ta_sma_2 = ta::SMA(get_input_int("Fast Length", 10));
            _ta_wma_3 = ta::WMA(get_input_int("Fast Length", 10));
            _ta_hma_4 = ta::HMA(get_input_int("Fast Length", 10));
            _ta_sma_5 = ta::SMA(get_input_int("Fast Length", 10));
            _ta_ema_1_cs1 = ta::EMA(get_input_int("Slow Length", 30));
            _ta_sma_2_cs1 = ta::SMA(get_input_int("Slow Length", 30));
            _ta_wma_3_cs1 = ta::WMA(get_input_int("Slow Length", 30));
            _ta_hma_4_cs1 = ta::HMA(get_input_int("Slow Length", 30));
            _ta_sma_5_cs1 = ta::SMA(get_input_int("Slow Length", 30));
            _ta_initialized_ = true;
        }
        src = get_input_source("Source", _src_close_)[0];
        fastMA = getMA_cs0(src, fastLen);
        slowMA = getMA_cs1(src, slowLen);
        longCond = (is_first_tick_ ? _ta_crossover_6.compute(fastMA, slowMA) : _ta_crossover_6.recompute(fastMA, slowMA));
        shortCond = (is_first_tick_ ? _ta_crossunder_7.compute(fastMA, slowMA) : _ta_crossunder_7.recompute(fastMA, slowMA));
        if (longCond) {
            strategy_entry(std::string("Long"), true, na<double>(), na<double>(), na<double>(), "");
        }
        if (shortCond) {
            strategy_entry(std::string("Short"), false, na<double>(), na<double>(), na<double>(), "");
        }
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
