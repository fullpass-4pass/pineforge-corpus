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
#include <pineforge/matrix.hpp>

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
    ta::SMA _ta_sma_1;
    ta::SMA _ta_sma_1_cs1;
    ta::SMA _ta_sma_1_cs2;
    ta::EMA _ta_ema_2;
    std::vector<double> _precalc__ta_ema_2;
    ta::EMA _ta_ema_3;
    std::vector<double> _precalc__ta_ema_3;
    ta::Crossover _ta_crossover_4;
    ta::Crossunder _ta_crossunder_5;
    bool _use_precalc = false;
    PineMatrix cov3;
    int N = 0;
    double c1 = 0.0;
    double c2 = 0.0;
    double c3 = 0.0;
    double m1 = 0.0;
    double m2 = 0.0;
    double m3 = 0.0;
    double d1 = 0.0;
    double d2 = 0.0;
    double d3 = 0.0;
    bool warmedUp = false;
    std::vector<double> evals;
    double emin = 0.0;
    bool eigenOk = false;
    double emaFast = 0.0;
    double emaSlow = 0.0;
    bool baseEntry = false;
    bool baseExit = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_sma_1(32), _ta_sma_1_cs1(32), _ta_sma_1_cs2(32), _ta_ema_2(9), _ta_ema_3(21) {
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

    double mean_cs0(double src, int len) {
        return (is_first_tick_ ? _ta_sma_1.compute(src) : _ta_sma_1.recompute(src));
    }

    double mean_cs1(double src, int len) {
        return (is_first_tick_ ? _ta_sma_1_cs1.compute(src) : _ta_sma_1_cs1.recompute(src));
    }

    double mean_cs2(double src, int len) {
        return (is_first_tick_ ? _ta_sma_1_cs2.compute(src) : _ta_sma_1_cs2.recompute(src));
    }

    void on_bar(const Bar& bar) override {
        if (!_var_initialized) {
            cov3 = PineMatrix::new_(3, 3, 0.0);
            _var_initialized = true;
        } else {
        }
        N = 32;
        c1 = current_bar_.close;
        c2 = (current_bar_.close * 2.0);
        c3 = (current_bar_.close * 3.0);
        m1 = mean_cs0(c1, 32);
        m2 = mean_cs1(c2, 32);
        m3 = mean_cs2(c3, 32);
        d1 = (is_na((c1 - m1)) ? 0.0 : (c1 - m1));
        d2 = (is_na((c2 - m2)) ? 0.0 : (c2 - m2));
        d3 = (is_na((c3 - m3)) ? 0.0 : (c3 - m3));
        cov3.set((int)(0), (int)(0), (d1 * d1));
        cov3.set((int)(0), (int)(1), (d1 * d2));
        cov3.set((int)(0), (int)(2), (d1 * d3));
        cov3.set((int)(1), (int)(0), (d2 * d1));
        cov3.set((int)(1), (int)(1), (d2 * d2));
        cov3.set((int)(1), (int)(2), (d2 * d3));
        cov3.set((int)(2), (int)(0), (d3 * d1));
        cov3.set((int)(2), (int)(1), (d3 * d2));
        cov3.set((int)(2), (int)(2), (d3 * d3));
        warmedUp = (bar_index_ >= 32);
        evals = ((warmedUp) ? (cov3.eigenvalues()) : (std::vector<double>((size_t)(0), 0.0)));
        emin = ((((double)evals.size() > 0)) ? (*std::min_element(evals.begin(),evals.end())) : (na<double>()));
        eigenOk = (!(is_na(emin)) && (emin > 1e-09));
        emaFast = (is_first_tick_ ? _ta_ema_2.compute(current_bar_.close) : _ta_ema_2.recompute(current_bar_.close));
        emaSlow = (is_first_tick_ ? _ta_ema_3.compute(current_bar_.close) : _ta_ema_3.recompute(current_bar_.close));
        baseEntry = (is_first_tick_ ? _ta_crossover_4.compute(emaFast, emaSlow) : _ta_crossover_4.recompute(emaFast, emaSlow));
        baseExit = (is_first_tick_ ? _ta_crossunder_5.compute(emaFast, emaSlow) : _ta_crossunder_5.recompute(emaFast, emaSlow));
        if ((baseEntry && (signed_position_size() == 0))) {
            if (eigenOk) {
                strategy_entry(std::string("L_eig"), true, na<double>(), na<double>(), 1, std::string("entry long eig-ok"), "", 0, -1);
            } else {
                strategy_entry(std::string("L_fb"), true, na<double>(), na<double>(), 1, std::string("entry long fallback"), "", 0, -1);
            }
        }
        if ((baseExit && (signed_position_size() > 0))) {
            strategy_close_all();
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_ema_2.resize(n);
        _precalc__ta_ema_3.resize(n);

        _ta_ema_2 = ta::EMA(9);
        _ta_ema_3 = ta::EMA(21);


        for (int i = 0; i < n; ++i) {
            _precalc__ta_ema_2[i] = _ta_ema_2.compute(bars[i].close);
            _precalc__ta_ema_3[i] = _ta_ema_3.compute(bars[i].close);
        }

        _ta_ema_2 = ta::EMA(9);
        _ta_ema_3 = ta::EMA(21);

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
