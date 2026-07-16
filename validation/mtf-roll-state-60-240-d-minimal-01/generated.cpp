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
    double _req_sec_0 = na<double>();
    double _req_sec_1 = na<double>();
    double _req_sec_2 = na<double>();
    Series<double> _sec2_hist_high;
    std::unordered_map<std::string, Series<double>> _security_helper_series_;
    ta::Change _ta_change_1;
    ta::Change _ta_change_2;
    bool _use_precalc = false;
    Series<double> _s_close;
    Series<double> _s_high;
    Series<double> dHigh1;
    Series<double> h240;
    Series<double> h60;
    bool roll60 = false;
    bool roll240 = false;
    bool dailyBreak = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        decltype(GeneratedStrategy::_req_sec_0) _pf_value_0;
        decltype(GeneratedStrategy::_req_sec_1) _pf_value_1;
        decltype(GeneratedStrategy::_req_sec_2) _pf_value_2;
        decltype(GeneratedStrategy::_sec2_hist_high) _pf_value_3;
        decltype(GeneratedStrategy::_security_helper_series_) _pf_value_4;
        decltype(GeneratedStrategy::_ta_change_1) _pf_value_5;
        decltype(GeneratedStrategy::_ta_change_2) _pf_value_6;
        decltype(GeneratedStrategy::_s_close) _pf_value_7;
        decltype(GeneratedStrategy::_s_high) _pf_value_8;
        decltype(GeneratedStrategy::dHigh1) _pf_value_9;
        decltype(GeneratedStrategy::h240) _pf_value_10;
        decltype(GeneratedStrategy::h60) _pf_value_11;
        decltype(GeneratedStrategy::roll60) _pf_value_12;
        decltype(GeneratedStrategy::roll240) _pf_value_13;
        decltype(GeneratedStrategy::dailyBreak) _pf_value_14;
        decltype(GeneratedStrategy::_ta_initialized_) _pf_value_15;
        decltype(GeneratedStrategy::_inputs_initialized_) _pf_value_16;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _req_sec_0,
            _req_sec_1,
            _req_sec_2,
            _sec2_hist_high,
            _security_helper_series_,
            _ta_change_1,
            _ta_change_2,
            _s_close,
            _s_high,
            dHigh1,
            h240,
            h60,
            roll60,
            roll240,
            dailyBreak,
            _ta_initialized_,
            _inputs_initialized_,
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        this->_req_sec_0 = _pf_script_state_checkpoint_->_pf_value_0;
        this->_req_sec_1 = _pf_script_state_checkpoint_->_pf_value_1;
        this->_req_sec_2 = _pf_script_state_checkpoint_->_pf_value_2;
        this->_sec2_hist_high = _pf_script_state_checkpoint_->_pf_value_3;
        this->_security_helper_series_ = _pf_script_state_checkpoint_->_pf_value_4;
        this->_ta_change_1 = _pf_script_state_checkpoint_->_pf_value_5;
        this->_ta_change_2 = _pf_script_state_checkpoint_->_pf_value_6;
        this->_s_close = _pf_script_state_checkpoint_->_pf_value_7;
        this->_s_high = _pf_script_state_checkpoint_->_pf_value_8;
        this->dHigh1 = _pf_script_state_checkpoint_->_pf_value_9;
        this->h240 = _pf_script_state_checkpoint_->_pf_value_10;
        this->h60 = _pf_script_state_checkpoint_->_pf_value_11;
        this->roll60 = _pf_script_state_checkpoint_->_pf_value_12;
        this->roll240 = _pf_script_state_checkpoint_->_pf_value_13;
        this->dailyBreak = _pf_script_state_checkpoint_->_pf_value_14;
        this->_ta_initialized_ = _pf_script_state_checkpoint_->_pf_value_15;
        this->_inputs_initialized_ = _pf_script_state_checkpoint_->_pf_value_16;
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() {
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

    void configure_security_evaluators() override {
        security_eval_states_.clear();
        register_security_eval(0, "60", input_tf_, false, false);
        register_security_eval(1, "240", input_tf_, false, false);
        register_security_eval(2, "D", input_tf_, false, false);
    }

    void on_bar(const Bar& bar) override {
        if (history_advances_new_bar()) _s_close.push(current_bar_.close);
        else _s_close.update(current_bar_.close);
        if (history_advances_new_bar()) _s_high.push(current_bar_.high);
        else _s_high.update(current_bar_.high);
        if (history_advances_new_bar()) h60.push(_req_sec_0);
        else h60.update(_req_sec_0);
        if (history_advances_new_bar()) h240.push(_req_sec_1);
        else h240.update(_req_sec_1);
        if (history_advances_new_bar()) dHigh1.push(_req_sec_2);
        else dHigh1.update(_req_sec_2);
        roll60 = ([&]{ auto _pna_l = ((history_advances_new_bar() ? _ta_change_1.compute(h60[0]) : _ta_change_1.recompute(h60[0]))); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (!_pfc_eq); }());
        roll240 = ([&]{ auto _pna_l = ((history_advances_new_bar() ? _ta_change_2.compute(h240[0]) : _ta_change_2.recompute(h240[0]))); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (!_pfc_eq); }());
        dailyBreak = (([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (dHigh1[0]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (_s_close[1]); auto _pna_r = (dHigh1[1]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) || _pfc_eq); }()));
        if ((((roll60 && ([&]{ auto _pna_l = (h60[0]); auto _pna_r = (h60[1]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) && ([&]{ auto _pna_l = (h240[0]); auto _pna_r = (h240[1]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }())) && ([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()))) {
            strategy_entry(std::string("R60"), true, na<double>(), na<double>(), 1, std::string("60 up with 240 state"), "", 0, -1);
        }
        if (((roll240 && ([&]{ auto _pna_l = (h240[0]); auto _pna_r = (h240[1]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }())) && ([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()))) {
            strategy_close(std::string("R60"), std::string("240 down close"), na<double>(), na<double>(), false);
        }
        if ((dailyBreak && ([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()))) {
            strategy_entry(std::string("D"), true, na<double>(), na<double>(), 1, std::string("daily high break"), "", 0, -1);
        }
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = ((dHigh1[0] * 0.995)); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()))) {
            strategy_close("", std::string("daily ref close"), na<double>(), na<double>(), false);
        }
    }


    void _eval_security_0(const Bar& bar, bool is_complete) {
        _req_sec_0 = bar.close;
    }

    void _eval_security_1(const Bar& bar, bool is_complete) {
        _req_sec_1 = bar.close;
    }

    void _eval_security_2(const Bar& bar, bool is_complete) {
        _req_sec_2 = _sec2_hist_high[0];
        if (is_complete) {
            _sec2_hist_high.push(bar.high);
        }
    }

    void evaluate_security(int sec_id, const Bar& bar, bool is_complete) override {
        switch (sec_id) {
            case 0: _eval_security_0(bar, is_complete); break;
            case 1: _eval_security_1(bar, is_complete); break;
            case 2: _eval_security_2(bar, is_complete); break;
        }
    }
    void clear_security(int sec_id) override {
        switch (sec_id) {
            case 0: _req_sec_0 = na<double>(); break;
            case 1: _req_sec_1 = na<double>(); break;
            case 2:
                _req_sec_2 = na<double>();
                _sec2_hist_high.clear();
                break;
        }
    }
};

extern "C" {
    void* strategy_create(const char* params_json) {
        return new GeneratedStrategy();
    }
    void run_backtest(void* s, Bar* bars, int n, ReportC* out) {
        auto* strat = static_cast<GeneratedStrategy*>(s);
        strat->run(bars, n, "", "", false, 4, MagnifierDistribution::ENDPOINTS);
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
        strat->run(bars, n, itf, stf, bar_magnifier != 0, magnifier_samples,
                   static_cast<MagnifierDistribution>(magnifier_dist));
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
