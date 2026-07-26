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
    ta::ATR _ta_atr_1;
    std::vector<double> _precalc__ta_atr_1;
    ta::Highest _ta_highest_2;
    std::vector<double> _precalc__ta_highest_2;
    ta::Highest _ta_highest_3;
    std::vector<double> _precalc__ta_highest_3;
    ta::Lowest _ta_lowest_4;
    std::vector<double> _precalc__ta_lowest_4;
    bool _use_precalc = false;
    int fillBar;
    Series<double> _strat_position_size;
    Series<double> _hist_call_1;
    Series<double> _hist_call_2;
    Series<double> _hist_call_3;
    int harborLength = 0;
    int atrLength = 0;
    double activationAtr = 0.0;
    double limitAtr = 0.0;
    double stopAtr = 0.0;
    int maximumBars = 0;
    double atrValue = 0.0;
    double rangeHigh = 0.0;
    double rangeMid = 0.0;
    double activationPrice = 0.0;
    double limitPrice = 0.0;
    bool validSetup = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        decltype(GeneratedStrategy::_ta_atr_1) _pf_value_0;
        decltype(GeneratedStrategy::_ta_highest_2) _pf_value_1;
        decltype(GeneratedStrategy::_ta_highest_3) _pf_value_2;
        decltype(GeneratedStrategy::_ta_lowest_4) _pf_value_3;
        decltype(GeneratedStrategy::fillBar) _pf_value_4;
        decltype(GeneratedStrategy::_strat_position_size) _pf_value_5;
        decltype(GeneratedStrategy::_hist_call_1) _pf_value_6;
        decltype(GeneratedStrategy::_hist_call_2) _pf_value_7;
        decltype(GeneratedStrategy::_hist_call_3) _pf_value_8;
        decltype(GeneratedStrategy::harborLength) _pf_value_9;
        decltype(GeneratedStrategy::atrLength) _pf_value_10;
        decltype(GeneratedStrategy::activationAtr) _pf_value_11;
        decltype(GeneratedStrategy::limitAtr) _pf_value_12;
        decltype(GeneratedStrategy::stopAtr) _pf_value_13;
        decltype(GeneratedStrategy::maximumBars) _pf_value_14;
        decltype(GeneratedStrategy::atrValue) _pf_value_15;
        decltype(GeneratedStrategy::rangeHigh) _pf_value_16;
        decltype(GeneratedStrategy::rangeMid) _pf_value_17;
        decltype(GeneratedStrategy::activationPrice) _pf_value_18;
        decltype(GeneratedStrategy::limitPrice) _pf_value_19;
        decltype(GeneratedStrategy::validSetup) _pf_value_20;
        decltype(GeneratedStrategy::_var_initialized) _pf_value_21;
        decltype(GeneratedStrategy::_ta_initialized_) _pf_value_22;
        decltype(GeneratedStrategy::_inputs_initialized_) _pf_value_23;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _ta_atr_1,
            _ta_highest_2,
            _ta_highest_3,
            _ta_lowest_4,
            fillBar,
            _strat_position_size,
            _hist_call_1,
            _hist_call_2,
            _hist_call_3,
            harborLength,
            atrLength,
            activationAtr,
            limitAtr,
            stopAtr,
            maximumBars,
            atrValue,
            rangeHigh,
            rangeMid,
            activationPrice,
            limitPrice,
            validSetup,
            _var_initialized,
            _ta_initialized_,
            _inputs_initialized_,
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        this->_ta_atr_1 = _pf_script_state_checkpoint_->_pf_value_0;
        this->_ta_highest_2 = _pf_script_state_checkpoint_->_pf_value_1;
        this->_ta_highest_3 = _pf_script_state_checkpoint_->_pf_value_2;
        this->_ta_lowest_4 = _pf_script_state_checkpoint_->_pf_value_3;
        this->fillBar = _pf_script_state_checkpoint_->_pf_value_4;
        this->_strat_position_size = _pf_script_state_checkpoint_->_pf_value_5;
        this->_hist_call_1 = _pf_script_state_checkpoint_->_pf_value_6;
        this->_hist_call_2 = _pf_script_state_checkpoint_->_pf_value_7;
        this->_hist_call_3 = _pf_script_state_checkpoint_->_pf_value_8;
        this->harborLength = _pf_script_state_checkpoint_->_pf_value_9;
        this->atrLength = _pf_script_state_checkpoint_->_pf_value_10;
        this->activationAtr = _pf_script_state_checkpoint_->_pf_value_11;
        this->limitAtr = _pf_script_state_checkpoint_->_pf_value_12;
        this->stopAtr = _pf_script_state_checkpoint_->_pf_value_13;
        this->maximumBars = _pf_script_state_checkpoint_->_pf_value_14;
        this->atrValue = _pf_script_state_checkpoint_->_pf_value_15;
        this->rangeHigh = _pf_script_state_checkpoint_->_pf_value_16;
        this->rangeMid = _pf_script_state_checkpoint_->_pf_value_17;
        this->activationPrice = _pf_script_state_checkpoint_->_pf_value_18;
        this->limitPrice = _pf_script_state_checkpoint_->_pf_value_19;
        this->validSetup = _pf_script_state_checkpoint_->_pf_value_20;
        this->_var_initialized = _pf_script_state_checkpoint_->_pf_value_21;
        this->_ta_initialized_ = _pf_script_state_checkpoint_->_pf_value_22;
        this->_inputs_initialized_ = _pf_script_state_checkpoint_->_pf_value_23;
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() : _ta_atr_1(14), _ta_highest_2(35), _ta_highest_3(35), _ta_lowest_4(35), fillBar(na<int>()) {
        initial_capital_ = 100000.0;
        default_qty_type_ = QtyType::PERCENT_OF_EQUITY;
        default_qty_value_ = 5.0;
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
        if (history_advances_new_bar() && bar_index_ == 0) _hist_call_1.clear();
        if (history_advances_new_bar() && bar_index_ == 0) _hist_call_2.clear();
        if (history_advances_new_bar() && bar_index_ == 0) _hist_call_3.clear();
        if (history_advances_new_bar()) _strat_position_size.push(signed_position_size());
        else _strat_position_size.update(signed_position_size());
        if (!_var_initialized) {
            _var_initialized = true;
        } else {
        }
        if (!_inputs_initialized_) {
            harborLength = get_input_int("Harbor Range", 35);
            atrLength = get_input_int("ATR Length", 14);
            activationAtr = get_input_double("Activation Buffer ATR", 0.1);
            limitAtr = get_input_double("Maximum Fill Distance ATR", 0.28);
            stopAtr = get_input_double("Protective Stop ATR", 2.4);
            maximumBars = get_input_int("Maximum Hold Bars", 64);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_atr_1 = ta::ATR(get_input_int("ATR Length", 14));
            _ta_highest_2 = ta::Highest(get_input_int("Harbor Range", 35));
            _ta_highest_3 = ta::Highest(get_input_int("Harbor Range", 35));
            _ta_lowest_4 = ta::Lowest(get_input_int("Harbor Range", 35));
            _ta_initialized_ = true;
        }
        atrValue = (history_advances_new_bar() ? _ta_atr_1.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_1.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        rangeHigh = ([&]() -> double { if (_use_precalc) { auto _pf_hist_offset_raw = (1); if (is_na(_pf_hist_offset_raw)) return na<double>(); long double _pf_hist_offset_numeric = static_cast<long double>(_pf_hist_offset_raw); if (bar_index_ < 0 || _pf_hist_offset_numeric < 0.0L || _pf_hist_offset_numeric > static_cast<long double>(bar_index_)) return na<double>(); std::size_t _pf_hist_offset = static_cast<std::size_t>(_pf_hist_offset_numeric); std::size_t _pf_hist_bar = static_cast<std::size_t>(bar_index_) - _pf_hist_offset; if (_pf_hist_bar >= _precalc__ta_highest_2.size()) return na<double>(); return _precalc__ta_highest_2[(std::size_t)_pf_hist_bar]; } double _hv = ((_use_precalc ? _precalc__ta_highest_2[bar_index_] : (history_advances_new_bar() ? _ta_highest_2.compute(current_bar_.high) : _ta_highest_2.recompute(current_bar_.high)))); if (history_advances_new_bar()) _hist_call_1.push(_hv); else _hist_call_1.update(_hv); return _hist_call_1[(int)(1)]; }());
        rangeMid = ((double)((([&]() -> double { if (_use_precalc) { auto _pf_hist_offset_raw = (1); if (is_na(_pf_hist_offset_raw)) return na<double>(); long double _pf_hist_offset_numeric = static_cast<long double>(_pf_hist_offset_raw); if (bar_index_ < 0 || _pf_hist_offset_numeric < 0.0L || _pf_hist_offset_numeric > static_cast<long double>(bar_index_)) return na<double>(); std::size_t _pf_hist_offset = static_cast<std::size_t>(_pf_hist_offset_numeric); std::size_t _pf_hist_bar = static_cast<std::size_t>(bar_index_) - _pf_hist_offset; if (_pf_hist_bar >= _precalc__ta_highest_3.size()) return na<double>(); return _precalc__ta_highest_3[(std::size_t)_pf_hist_bar]; } double _hv = ((_use_precalc ? _precalc__ta_highest_3[bar_index_] : (history_advances_new_bar() ? _ta_highest_3.compute(current_bar_.high) : _ta_highest_3.recompute(current_bar_.high)))); if (history_advances_new_bar()) _hist_call_2.push(_hv); else _hist_call_2.update(_hv); return _hist_call_2[(int)(1)]; }()) + ([&]() -> double { if (_use_precalc) { auto _pf_hist_offset_raw = (1); if (is_na(_pf_hist_offset_raw)) return na<double>(); long double _pf_hist_offset_numeric = static_cast<long double>(_pf_hist_offset_raw); if (bar_index_ < 0 || _pf_hist_offset_numeric < 0.0L || _pf_hist_offset_numeric > static_cast<long double>(bar_index_)) return na<double>(); std::size_t _pf_hist_offset = static_cast<std::size_t>(_pf_hist_offset_numeric); std::size_t _pf_hist_bar = static_cast<std::size_t>(bar_index_) - _pf_hist_offset; if (_pf_hist_bar >= _precalc__ta_lowest_4.size()) return na<double>(); return _precalc__ta_lowest_4[(std::size_t)_pf_hist_bar]; } double _hv = ((_use_precalc ? _precalc__ta_lowest_4[bar_index_] : (history_advances_new_bar() ? _ta_lowest_4.compute(current_bar_.low) : _ta_lowest_4.recompute(current_bar_.low)))); if (history_advances_new_bar()) _hist_call_3.push(_hv); else _hist_call_3.update(_hv); return _hist_call_3[(int)(1)]; }()))) / (double)(2.0));
        activationPrice = (rangeHigh + (atrValue * activationAtr));
        limitPrice = (activationPrice + (atrValue * limitAtr));
        validSetup = (([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (rangeMid); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (activationPrice); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()) && validSetup)) {
            strategy_entry(std::string("Harbor Breakout"), true, limitPrice, activationPrice, na<double>(), "", "", 0, -1);
        } else
        if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }())) {
            strategy_cancel(std::string("Harbor Breakout"));
        }
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (_strat_position_size[1]); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) || _pfc_eq); }()))) {
            fillBar = pine_bar_index();
        }
        if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }())) {
            fillBar = na<int>();
        }
        if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) {
            strategy_exit(std::string("Harbor Guard"), std::string("Harbor Breakout"), na<double>(), ((signed_position_size() == 0.0 ? na<double>() : position_entry_price_) - (atrValue * stopAtr)), na<double>(), na<double>(), na<double>(), 100.0, "", na<double>(), "", na<double>(), na<double>());
            if ((!(is_na(fillBar)) && ([&]{ auto _pna_l = ((pine_bar_index() - fillBar)); auto _pna_r = (maximumBars); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }()))) {
                strategy_close(std::string("Harbor Breakout"), std::string("Harbor time limit"), na<double>(), na<double>(), false, 188978561047ULL);
            }
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_atr_1.resize(n);
        _precalc__ta_highest_2.resize(n);
        _precalc__ta_highest_3.resize(n);
        _precalc__ta_lowest_4.resize(n);

        _ta_atr_1 = ta::ATR(14);
        _ta_highest_2 = ta::Highest(35);
        _ta_highest_3 = ta::Highest(35);
        _ta_lowest_4 = ta::Lowest(35);


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
            _precalc__ta_atr_1[i] = _ta_atr_1.compute(bars[i].high, bars[i].low, bars[i].close);
            _precalc__ta_highest_2[i] = _ta_highest_2.compute(bars[i].high);
            _precalc__ta_highest_3[i] = _ta_highest_3.compute(bars[i].high);
            _precalc__ta_lowest_4[i] = _ta_lowest_4.compute(bars[i].low);
        }

        _ta_atr_1 = ta::ATR(14);
        _ta_highest_2 = ta::Highest(35);
        _ta_highest_3 = ta::Highest(35);
        _ta_lowest_4 = ta::Lowest(35);

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
