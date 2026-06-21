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
    ta::TR _ta_tr_1;
    ta::RMA _ta_rma_2;
    ta::RMA _ta_rma_3;
    ta::RMA _ta_rma_4;
    ta::RMA _ta_rma_5;
    ta::ATR _ta_atr_6;
    std::vector<double> _precalc__ta_atr_6;
    ta::SMA _ta_sma_7;
    bool _use_precalc = false;
    Series<double> _s_high;
    Series<double> _s_low;
    Series<bool> trending_regime;
    int i_adx_len = 0;
    double i_adx_trend = 0.0;
    int i_atr_len = 0;
    double i_vol_exp = 0.0;
    double i_vol_con = 0.0;
    double atr_val = 0.0;
    double atr_ma = 0.0;
    double vol_ratio = 0.0;
    int regime = 0;
    bool long_entry = false;
    bool long_exit = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_tr_1(true), _ta_rma_2(14), _ta_rma_3(14), _ta_rma_4(14), _ta_rma_5(14), _ta_atr_6(14), _ta_sma_7(42) {
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

    std::tuple<double, double, double> f_adx_calc_cs0(int len) {
        double tr_val = (is_first_tick_ ? _ta_tr_1.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_tr_1.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        double plus_dm = std::max((double)((current_bar_.high - _s_high[1])), (double)(0));
        double minus_dm = std::max((double)((_s_low[1] - current_bar_.low)), (double)(0));
        if ((plus_dm > minus_dm)) {
            minus_dm = 0;
        } else {
            plus_dm = 0;
        }
        double smooth_tr = (is_first_tick_ ? _ta_rma_2.compute(tr_val) : _ta_rma_2.recompute(tr_val));
        double smooth_plus = (is_first_tick_ ? _ta_rma_3.compute(plus_dm) : _ta_rma_3.recompute(plus_dm));
        double smooth_minus = (is_first_tick_ ? _ta_rma_4.compute(minus_dm) : _ta_rma_4.recompute(minus_dm));
        double plus_di = (((smooth_tr > 0)) ? (((double)((100 * smooth_plus)) / (double)(smooth_tr))) : (0));
        double minus_di = (((smooth_tr > 0)) ? (((double)((100 * smooth_minus)) / (double)(smooth_tr))) : (0));
        double di_sum = (plus_di + minus_di);
        double dx = (((di_sum > 0)) ? (((double)((100 * std::abs((plus_di - minus_di)))) / (double)(di_sum))) : (0));
        double adx_val = (is_first_tick_ ? _ta_rma_5.compute(dx) : _ta_rma_5.recompute(dx));
        return std::make_tuple(adx_val, plus_di, minus_di);
    }

    void on_bar(const Bar& bar) override {
        if (is_first_tick_) _s_high.push(current_bar_.high);
        else _s_high.update(current_bar_.high);
        if (is_first_tick_) _s_low.push(current_bar_.low);
        else _s_low.update(current_bar_.low);
        if (!_inputs_initialized_) {
            i_adx_len = get_input_int("ADX Period", 14);
            i_adx_trend = get_input_double("Trend Threshold", 25);
            i_atr_len = get_input_int("ATR Period", 14);
            i_vol_exp = get_input_double("Volatility Expansion", 1.4);
            i_vol_con = get_input_double("Volatility Contraction", 0.6);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_rma_2 = ta::RMA(get_input_int("ADX Period", 14));
            _ta_rma_3 = ta::RMA(get_input_int("ADX Period", 14));
            _ta_rma_4 = ta::RMA(get_input_int("ADX Period", 14));
            _ta_rma_5 = ta::RMA(get_input_int("ADX Period", 14));
            _ta_atr_6 = ta::ATR(get_input_int("ATR Period", 14));
            _ta_sma_7 = ta::SMA(get_input_int("ATR Period", 14) * 3);
            _ta_initialized_ = true;
        }
        auto [adx, plus_di, minus_di] = f_adx_calc_cs0(i_adx_len);
        atr_val = (is_first_tick_ ? _ta_atr_6.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_6.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        atr_ma = (is_first_tick_ ? _ta_sma_7.compute(atr_val) : _ta_sma_7.recompute(atr_val));
        vol_ratio = (((atr_ma > 0)) ? (((double)(atr_val) / (double)(atr_ma))) : (1.0));
        regime = 0;
        if (((vol_ratio >= i_vol_exp) && (adx < i_adx_trend))) {
            regime = 3;
        } else
        if ((adx >= i_adx_trend)) {
            regime = 1;
        } else
        if ((vol_ratio <= i_vol_con)) {
            regime = 2;
        }
        trending_regime.push((regime == 1));
        long_entry = ((trending_regime[0] && !(trending_regime[1])) && (signed_position_size() == 0));
        long_exit = (!(trending_regime[0]) && (signed_position_size() > 0));
        if (long_entry) {
            strategy_entry(std::string("L"), true, na<double>(), na<double>(), 1, std::string("trending ON"), "", 0, -1);
        }
        if (long_exit) {
            strategy_close(std::string("L"), std::string("trending OFF"), na<double>(), na<double>(), false);
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_atr_6.resize(n);

        _ta_atr_6 = ta::ATR(14);

        _s_high.clear();
        _s_low.clear();

        for (int i = 0; i < n; ++i) {
            _s_high.push(bars[i].high);
            _s_low.push(bars[i].low);
            _precalc__ta_atr_6[i] = _ta_atr_6.compute(bars[i].high, bars[i].low, bars[i].close);
        }

        _ta_atr_6 = ta::ATR(14);
        _s_high.clear();
        _s_low.clear();

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
