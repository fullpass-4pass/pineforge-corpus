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
    ta::StdDev _ta_stdev_1;
    std::vector<double> _precalc__ta_stdev_1;
    ta::ATR _ta_atr_2;
    std::vector<double> _precalc__ta_atr_2;
    ta::Crossover _ta_crossover_3;
    ta::Crossunder _ta_crossunder_4;
    bool _use_precalc = false;
    Series<double> _s_close;
    int openedAt;
    Series<double> _strat_position_size;
    int channelLength = 0;
    double bandWidth = 0.0;
    int atrLength = 0;
    double stopAtr = 0.0;
    int maximumHoldBars = 0;
    double weightedSum = 0.0;
    double weightTotal = 0.0;
    int cursor = 0;
    double sample = 0.0;
    double weight = 0.0;
    double center = 0.0;
    double dispersion = 0.0;
    double upperBand = 0.0;
    double atrValue = 0.0;
    bool entrySignal = false;
    bool exitSignal = false;
    bool holdExpired = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        decltype(GeneratedStrategy::_ta_stdev_1) _pf_value_0;
        decltype(GeneratedStrategy::_ta_atr_2) _pf_value_1;
        decltype(GeneratedStrategy::_ta_crossover_3) _pf_value_2;
        decltype(GeneratedStrategy::_ta_crossunder_4) _pf_value_3;
        decltype(GeneratedStrategy::_s_close) _pf_value_4;
        decltype(GeneratedStrategy::openedAt) _pf_value_5;
        decltype(GeneratedStrategy::_strat_position_size) _pf_value_6;
        decltype(GeneratedStrategy::channelLength) _pf_value_7;
        decltype(GeneratedStrategy::bandWidth) _pf_value_8;
        decltype(GeneratedStrategy::atrLength) _pf_value_9;
        decltype(GeneratedStrategy::stopAtr) _pf_value_10;
        decltype(GeneratedStrategy::maximumHoldBars) _pf_value_11;
        decltype(GeneratedStrategy::weightedSum) _pf_value_12;
        decltype(GeneratedStrategy::weightTotal) _pf_value_13;
        decltype(GeneratedStrategy::cursor) _pf_value_14;
        decltype(GeneratedStrategy::sample) _pf_value_15;
        decltype(GeneratedStrategy::weight) _pf_value_16;
        decltype(GeneratedStrategy::center) _pf_value_17;
        decltype(GeneratedStrategy::dispersion) _pf_value_18;
        decltype(GeneratedStrategy::upperBand) _pf_value_19;
        decltype(GeneratedStrategy::atrValue) _pf_value_20;
        decltype(GeneratedStrategy::entrySignal) _pf_value_21;
        decltype(GeneratedStrategy::exitSignal) _pf_value_22;
        decltype(GeneratedStrategy::holdExpired) _pf_value_23;
        decltype(GeneratedStrategy::_var_initialized) _pf_value_24;
        decltype(GeneratedStrategy::_ta_initialized_) _pf_value_25;
        decltype(GeneratedStrategy::_inputs_initialized_) _pf_value_26;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _ta_stdev_1,
            _ta_atr_2,
            _ta_crossover_3,
            _ta_crossunder_4,
            _s_close,
            openedAt,
            _strat_position_size,
            channelLength,
            bandWidth,
            atrLength,
            stopAtr,
            maximumHoldBars,
            weightedSum,
            weightTotal,
            cursor,
            sample,
            weight,
            center,
            dispersion,
            upperBand,
            atrValue,
            entrySignal,
            exitSignal,
            holdExpired,
            _var_initialized,
            _ta_initialized_,
            _inputs_initialized_,
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        this->_ta_stdev_1 = _pf_script_state_checkpoint_->_pf_value_0;
        this->_ta_atr_2 = _pf_script_state_checkpoint_->_pf_value_1;
        this->_ta_crossover_3 = _pf_script_state_checkpoint_->_pf_value_2;
        this->_ta_crossunder_4 = _pf_script_state_checkpoint_->_pf_value_3;
        this->_s_close = _pf_script_state_checkpoint_->_pf_value_4;
        this->openedAt = _pf_script_state_checkpoint_->_pf_value_5;
        this->_strat_position_size = _pf_script_state_checkpoint_->_pf_value_6;
        this->channelLength = _pf_script_state_checkpoint_->_pf_value_7;
        this->bandWidth = _pf_script_state_checkpoint_->_pf_value_8;
        this->atrLength = _pf_script_state_checkpoint_->_pf_value_9;
        this->stopAtr = _pf_script_state_checkpoint_->_pf_value_10;
        this->maximumHoldBars = _pf_script_state_checkpoint_->_pf_value_11;
        this->weightedSum = _pf_script_state_checkpoint_->_pf_value_12;
        this->weightTotal = _pf_script_state_checkpoint_->_pf_value_13;
        this->cursor = _pf_script_state_checkpoint_->_pf_value_14;
        this->sample = _pf_script_state_checkpoint_->_pf_value_15;
        this->weight = _pf_script_state_checkpoint_->_pf_value_16;
        this->center = _pf_script_state_checkpoint_->_pf_value_17;
        this->dispersion = _pf_script_state_checkpoint_->_pf_value_18;
        this->upperBand = _pf_script_state_checkpoint_->_pf_value_19;
        this->atrValue = _pf_script_state_checkpoint_->_pf_value_20;
        this->entrySignal = _pf_script_state_checkpoint_->_pf_value_21;
        this->exitSignal = _pf_script_state_checkpoint_->_pf_value_22;
        this->holdExpired = _pf_script_state_checkpoint_->_pf_value_23;
        this->_var_initialized = _pf_script_state_checkpoint_->_pf_value_24;
        this->_ta_initialized_ = _pf_script_state_checkpoint_->_pf_value_25;
        this->_inputs_initialized_ = _pf_script_state_checkpoint_->_pf_value_26;
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() : _ta_stdev_1(28), _ta_atr_2(17), openedAt(na<int>()) {
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
        if (history_advances_new_bar()) _s_close.push(current_bar_.close);
        else _s_close.update(current_bar_.close);
        if (history_advances_new_bar()) _strat_position_size.push(signed_position_size());
        else _strat_position_size.update(signed_position_size());
        if (!_var_initialized) {
            _var_initialized = true;
        } else {
        }
        if (!_inputs_initialized_) {
            channelLength = get_input_int("Channel Length", 28);
            bandWidth = get_input_double("Band Width", 1.25);
            atrLength = get_input_int("ATR Length", 17);
            stopAtr = get_input_double("Stop ATR", 2.4);
            maximumHoldBars = get_input_int("Maximum Hold Bars", 160);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_stdev_1 = ta::StdDev(get_input_int("Channel Length", 28));
            _ta_atr_2 = ta::ATR(get_input_int("ATR Length", 17));
            _ta_initialized_ = true;
        }
        weightedSum = 0.0;
        weightTotal = 0.0;
        cursor = 0;
        while (([&]{ auto _pna_l = (cursor); auto _pna_r = (channelLength); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l < _pna_r); }())) {
            sample = _s_close[cursor];
            weight = (channelLength - cursor);
            if (!(is_na(sample))) {
                weightedSum += (sample * weight);
                weightTotal += weight;
            }
            cursor += 1;
        }
        center = ((([&]{ auto _pna_l = (weightTotal); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (((double)(weightedSum) / (double)(weightTotal))) : (na<double>()));
        dispersion = (history_advances_new_bar() ? _ta_stdev_1.compute(current_bar_.close) : _ta_stdev_1.recompute(current_bar_.close));
        upperBand = (center + (dispersion * bandWidth));
        atrValue = (history_advances_new_bar() ? _ta_atr_2.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_2.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (_strat_position_size[1]); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()))) {
            openedAt = pine_bar_index();
        }
        entrySignal = (history_advances_new_bar() ? _ta_crossover_3.compute(current_bar_.close, upperBand) : _ta_crossover_3.recompute(current_bar_.close, upperBand));
        exitSignal = (history_advances_new_bar() ? _ta_crossunder_4.compute(current_bar_.close, center) : _ta_crossunder_4.recompute(current_bar_.close, center));
        holdExpired = ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && !(is_na(openedAt))) && ([&]{ auto _pna_l = ((pine_bar_index() - openedAt)); auto _pna_r = (maximumHoldBars); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }()));
        if (entrySignal) {
            strategy_entry(std::string("Heliograph Long"), true, na<double>(), na<double>(), na<double>(), "");
        }
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && (exitSignal || holdExpired))) {
            strategy_close(std::string("Heliograph Long"), ((holdExpired) ? (std::string("Maximum hold")) : (std::string("Centerline exit"))), na<double>(), na<double>(), false, 206158430227ULL);
        }
        if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) {
            strategy_exit(std::string("Heliograph Guard"), std::string("Heliograph Long"), na<double>(), ((signed_position_size() == 0.0 ? na<double>() : position_entry_price_) - (atrValue * stopAtr)), na<double>(), na<double>(), na<double>(), 100.0, "", na<double>(), "", na<double>(), na<double>());
        }
        if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }())) {
            openedAt = na<int>();
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_stdev_1.resize(n);
        _precalc__ta_atr_2.resize(n);

        _ta_stdev_1 = ta::StdDev(28);
        _ta_atr_2 = ta::ATR(17);

        _s_close.clear();

        for (int i = 0; i < n; ++i) {
            _s_close.push(bars[i].close);
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
            _precalc__ta_stdev_1[i] = _ta_stdev_1.compute(bars[i].close);
            _precalc__ta_atr_2[i] = _ta_atr_2.compute(bars[i].high, bars[i].low, bars[i].close);
        }

        _ta_stdev_1 = ta::StdDev(28);
        _ta_atr_2 = ta::ATR(17);
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
