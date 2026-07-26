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
#include <optional>
#include <type_traits>
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
    ta::OBV _ta_obv_1;
    std::vector<double> _precalc__ta_obv_1;
    ta::AccDist _ta_accdist_2;
    std::vector<double> _precalc__ta_accdist_2;
    ta::EMA _ta_ema_3;
    ta::EMA _ta_ema_4;
    ta::EMA _ta_ema_5;
    std::vector<double> _precalc__ta_ema_5;
    ta::ATR _ta_atr_6;
    std::vector<double> _precalc__ta_atr_6;
    bool _use_precalc = false;
    Series<bool> flowAgreement;
    int obvSignalLength = 0;
    int accDistSignalLength = 0;
    int priceFilterLength = 0;
    int atrLength = 0;
    double maximumAtrLoss = 0.0;
    double obvValue = 0.0;
    double accDistValue = 0.0;
    double obvSignal = 0.0;
    double accDistSignal = 0.0;
    double priceFilter = 0.0;
    double atrValue = 0.0;
    bool enterLong = false;
    bool flowEnded = false;
    bool riskExceeded = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        decltype(GeneratedStrategy::_ta_obv_1) _pf_value_0;
        decltype(GeneratedStrategy::_ta_accdist_2) _pf_value_1;
        decltype(GeneratedStrategy::_ta_ema_3) _pf_value_2;
        decltype(GeneratedStrategy::_ta_ema_4) _pf_value_3;
        decltype(GeneratedStrategy::_ta_ema_5) _pf_value_4;
        decltype(GeneratedStrategy::_ta_atr_6) _pf_value_5;
        decltype(GeneratedStrategy::flowAgreement) _pf_value_6;
        decltype(GeneratedStrategy::obvSignalLength) _pf_value_7;
        decltype(GeneratedStrategy::accDistSignalLength) _pf_value_8;
        decltype(GeneratedStrategy::priceFilterLength) _pf_value_9;
        decltype(GeneratedStrategy::atrLength) _pf_value_10;
        decltype(GeneratedStrategy::maximumAtrLoss) _pf_value_11;
        decltype(GeneratedStrategy::obvValue) _pf_value_12;
        decltype(GeneratedStrategy::accDistValue) _pf_value_13;
        decltype(GeneratedStrategy::obvSignal) _pf_value_14;
        decltype(GeneratedStrategy::accDistSignal) _pf_value_15;
        decltype(GeneratedStrategy::priceFilter) _pf_value_16;
        decltype(GeneratedStrategy::atrValue) _pf_value_17;
        decltype(GeneratedStrategy::enterLong) _pf_value_18;
        decltype(GeneratedStrategy::flowEnded) _pf_value_19;
        decltype(GeneratedStrategy::riskExceeded) _pf_value_20;
        decltype(GeneratedStrategy::_ta_initialized_) _pf_value_21;
        decltype(GeneratedStrategy::_inputs_initialized_) _pf_value_22;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _ta_obv_1,
            _ta_accdist_2,
            _ta_ema_3,
            _ta_ema_4,
            _ta_ema_5,
            _ta_atr_6,
            flowAgreement,
            obvSignalLength,
            accDistSignalLength,
            priceFilterLength,
            atrLength,
            maximumAtrLoss,
            obvValue,
            accDistValue,
            obvSignal,
            accDistSignal,
            priceFilter,
            atrValue,
            enterLong,
            flowEnded,
            riskExceeded,
            _ta_initialized_,
            _inputs_initialized_,
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        this->_ta_obv_1 = _pf_script_state_checkpoint_->_pf_value_0;
        this->_ta_accdist_2 = _pf_script_state_checkpoint_->_pf_value_1;
        this->_ta_ema_3 = _pf_script_state_checkpoint_->_pf_value_2;
        this->_ta_ema_4 = _pf_script_state_checkpoint_->_pf_value_3;
        this->_ta_ema_5 = _pf_script_state_checkpoint_->_pf_value_4;
        this->_ta_atr_6 = _pf_script_state_checkpoint_->_pf_value_5;
        this->flowAgreement = _pf_script_state_checkpoint_->_pf_value_6;
        this->obvSignalLength = _pf_script_state_checkpoint_->_pf_value_7;
        this->accDistSignalLength = _pf_script_state_checkpoint_->_pf_value_8;
        this->priceFilterLength = _pf_script_state_checkpoint_->_pf_value_9;
        this->atrLength = _pf_script_state_checkpoint_->_pf_value_10;
        this->maximumAtrLoss = _pf_script_state_checkpoint_->_pf_value_11;
        this->obvValue = _pf_script_state_checkpoint_->_pf_value_12;
        this->accDistValue = _pf_script_state_checkpoint_->_pf_value_13;
        this->obvSignal = _pf_script_state_checkpoint_->_pf_value_14;
        this->accDistSignal = _pf_script_state_checkpoint_->_pf_value_15;
        this->priceFilter = _pf_script_state_checkpoint_->_pf_value_16;
        this->atrValue = _pf_script_state_checkpoint_->_pf_value_17;
        this->enterLong = _pf_script_state_checkpoint_->_pf_value_18;
        this->flowEnded = _pf_script_state_checkpoint_->_pf_value_19;
        this->riskExceeded = _pf_script_state_checkpoint_->_pf_value_20;
        this->_ta_initialized_ = _pf_script_state_checkpoint_->_pf_value_21;
        this->_inputs_initialized_ = _pf_script_state_checkpoint_->_pf_value_22;
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() : _ta_ema_3(29), _ta_ema_4(37), _ta_ema_5(52), _ta_atr_6(16) {
        initial_capital_ = 100000.0;
        default_qty_type_ = QtyType::FIXED;
        default_qty_value_ = 2.0;
        pyramiding_ = 0;
        commission_type_ = CommissionType::PERCENT;
        commission_value_ = 0.05;
        slippage_ = 1;
        margin_long_ = 100.0;
        margin_short_ = 100.0;
        script_has_strategy_close_ = true;
    }

    void set_strategy_override(const std::string& key, const std::string& value) {
        if (key == "initial_capital") { initial_capital_ = std::stod(value); return; }
        if (key == "commission_value") { commission_value_ = std::stod(value); return; }
        if (key == "default_qty_value") { default_qty_value_ = std::stod(value); return; }
        if (key == "pyramiding") { pyramiding_ = std::stoi(value); return; }
        if (key == "slippage") { slippage_ = std::stoi(value); return; }
        if (key == "process_orders_on_close") { process_orders_on_close_ = (value == "true" || value == "1"); return; }
        if (key == "calc_on_order_fills") { calc_on_order_fills_ = (value == "true" || value == "1"); return; }
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
        if (!_inputs_initialized_) {
            obvSignalLength = get_input_int("OBV Signal Length", 29);
            accDistSignalLength = get_input_int("AccDist Signal Length", 37);
            priceFilterLength = get_input_int("Price Filter Length", 52);
            atrLength = get_input_int("ATR Length", 16);
            maximumAtrLoss = get_input_double("Maximum ATR Loss", 2.4);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_ema_3 = ta::EMA(get_input_int("OBV Signal Length", 29));
            _ta_ema_4 = ta::EMA(get_input_int("AccDist Signal Length", 37));
            _ta_ema_5 = ta::EMA(get_input_int("Price Filter Length", 52));
            _ta_atr_6 = ta::ATR(get_input_int("ATR Length", 16));
            _ta_initialized_ = true;
        }
        obvValue = (history_advances_new_bar() ? _ta_obv_1.compute(current_bar_.close, current_bar_.volume) : _ta_obv_1.recompute(current_bar_.close, current_bar_.volume));
        accDistValue = (history_advances_new_bar() ? _ta_accdist_2.compute(current_bar_.high, current_bar_.low, current_bar_.close, current_bar_.volume) : _ta_accdist_2.recompute(current_bar_.high, current_bar_.low, current_bar_.close, current_bar_.volume));
        obvSignal = (history_advances_new_bar() ? _ta_ema_3.compute(obvValue) : _ta_ema_3.recompute(obvValue));
        accDistSignal = (history_advances_new_bar() ? _ta_ema_4.compute(accDistValue) : _ta_ema_4.recompute(accDistValue));
        priceFilter = (history_advances_new_bar() ? _ta_ema_5.compute(current_bar_.close) : _ta_ema_5.recompute(current_bar_.close));
        atrValue = (history_advances_new_bar() ? _ta_atr_6.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_6.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        if (history_advances_new_bar()) flowAgreement.push((([&]{ auto _pna_l = (obvValue); auto _pna_r = (obvSignal); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (accDistValue); auto _pna_r = (accDistSignal); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())));
        else flowAgreement.update((([&]{ auto _pna_l = (obvValue); auto _pna_r = (obvSignal); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (accDistValue); auto _pna_r = (accDistSignal); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())));
        enterLong = ((flowAgreement[0] && !(flowAgreement[1])) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (priceFilter); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()));
        flowEnded = (!(flowAgreement[0]) || ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (priceFilter); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        riskExceeded = (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (((signed_position_size() == 0.0 ? na<double>() : position_entry_price_) - (atrValue * maximumAtrLoss))); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()) && enterLong)) {
            strategy_entry(std::string("Conduit Long"), true, na<double>(), na<double>(), na<double>(), "");
        } else
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && (flowEnded || riskExceeded))) {
            strategy_close(std::string("Conduit Long"), ((riskExceeded) ? (std::string("ATR risk")) : (std::string("Flow ended"))), na<double>(), na<double>(), false, 150323855379ULL);
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_obv_1.resize(n);
        _precalc__ta_accdist_2.resize(n);
        _precalc__ta_ema_5.resize(n);
        _precalc__ta_atr_6.resize(n);

        _ta_obv_1 = ta::OBV();
        _ta_accdist_2 = ta::AccDist();
        _ta_ema_5 = ta::EMA(52);
        _ta_atr_6 = ta::ATR(16);


        for (int i = 0; i < n; ++i) {
            if (_src_series_active_) {
                const double _pc_o = bars[i].open;
                const double _pc_h = bars[i].high;
                const double _pc_l = bars[i].low;
                const double _pc_c = bars[i].close;
                const double _pc_v = bars[i].volume;
                _src_open_.push(_pc_o);   _src_high_.push(_pc_h);   _src_low_.push(_pc_l);
                _src_close_.push(_pc_c);  _src_volume_.push(_pc_v);
                _src_hl2_.push((_pc_h + _pc_l) / 2.0);
                _src_hlc3_.push((_pc_h + _pc_l + _pc_c) / 3.0);
                _src_ohlc4_.push((_pc_o + _pc_h + _pc_l + _pc_c) / 4.0);
                _src_hlcc4_.push((_pc_h + _pc_l + _pc_c + _pc_c) / 4.0);
            }
            _precalc__ta_obv_1[i] = _ta_obv_1.compute(bars[i].close, bars[i].volume);
            _precalc__ta_accdist_2[i] = _ta_accdist_2.compute(bars[i].high, bars[i].low, bars[i].close, bars[i].volume);
            _precalc__ta_ema_5[i] = _ta_ema_5.compute(bars[i].close);
            _precalc__ta_atr_6[i] = _ta_atr_6.compute(bars[i].high, bars[i].low, bars[i].close);
        }

        _ta_obv_1 = ta::OBV();
        _ta_accdist_2 = ta::AccDist();
        _ta_ema_5 = ta::EMA(52);
        _ta_atr_6 = ta::ATR(16);

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
