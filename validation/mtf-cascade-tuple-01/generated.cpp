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
    double _req_sec_0_0 = na<double>();
    double _req_sec_0_1 = na<double>();
    std::unordered_map<std::string, Series<double>> _security_helper_series_;
    ta::EMA _ta_ema_1;
    std::vector<double> _precalc__ta_ema_1;
    ta::ATR _ta_atr_2;
    std::vector<double> _precalc__ta_atr_2;
    ta::Crossover _ta_crossover_3;
    bool _use_precalc = false;
    ta::EMA _sec0__ta_ema_1;
    ta::ATR _sec0__ta_atr_2;
    std::string higherTimeframe = std::string("");
    int centerLength = 0;
    int atrLength = 0;
    double entryDistance = 0.0;
    double stopAtr = 0.0;
    double higherCenter = 0.0;
    double higherAtr = 0.0;
    double entryLine = 0.0;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        decltype(GeneratedStrategy::_req_sec_0_0) _pf_value_0;
        decltype(GeneratedStrategy::_req_sec_0_1) _pf_value_1;
        decltype(GeneratedStrategy::_security_helper_series_) _pf_value_2;
        decltype(GeneratedStrategy::_ta_ema_1) _pf_value_3;
        decltype(GeneratedStrategy::_ta_atr_2) _pf_value_4;
        decltype(GeneratedStrategy::_ta_crossover_3) _pf_value_5;
        decltype(GeneratedStrategy::_sec0__ta_ema_1) _pf_value_6;
        decltype(GeneratedStrategy::_sec0__ta_atr_2) _pf_value_7;
        decltype(GeneratedStrategy::higherTimeframe) _pf_value_8;
        decltype(GeneratedStrategy::centerLength) _pf_value_9;
        decltype(GeneratedStrategy::atrLength) _pf_value_10;
        decltype(GeneratedStrategy::entryDistance) _pf_value_11;
        decltype(GeneratedStrategy::stopAtr) _pf_value_12;
        decltype(GeneratedStrategy::higherCenter) _pf_value_13;
        decltype(GeneratedStrategy::higherAtr) _pf_value_14;
        decltype(GeneratedStrategy::entryLine) _pf_value_15;
        decltype(GeneratedStrategy::_ta_initialized_) _pf_value_16;
        decltype(GeneratedStrategy::_inputs_initialized_) _pf_value_17;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _req_sec_0_0,
            _req_sec_0_1,
            _security_helper_series_,
            _ta_ema_1,
            _ta_atr_2,
            _ta_crossover_3,
            _sec0__ta_ema_1,
            _sec0__ta_atr_2,
            higherTimeframe,
            centerLength,
            atrLength,
            entryDistance,
            stopAtr,
            higherCenter,
            higherAtr,
            entryLine,
            _ta_initialized_,
            _inputs_initialized_,
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        this->_req_sec_0_0 = _pf_script_state_checkpoint_->_pf_value_0;
        this->_req_sec_0_1 = _pf_script_state_checkpoint_->_pf_value_1;
        this->_security_helper_series_ = _pf_script_state_checkpoint_->_pf_value_2;
        this->_ta_ema_1 = _pf_script_state_checkpoint_->_pf_value_3;
        this->_ta_atr_2 = _pf_script_state_checkpoint_->_pf_value_4;
        this->_ta_crossover_3 = _pf_script_state_checkpoint_->_pf_value_5;
        this->_sec0__ta_ema_1 = _pf_script_state_checkpoint_->_pf_value_6;
        this->_sec0__ta_atr_2 = _pf_script_state_checkpoint_->_pf_value_7;
        this->higherTimeframe = _pf_script_state_checkpoint_->_pf_value_8;
        this->centerLength = _pf_script_state_checkpoint_->_pf_value_9;
        this->atrLength = _pf_script_state_checkpoint_->_pf_value_10;
        this->entryDistance = _pf_script_state_checkpoint_->_pf_value_11;
        this->stopAtr = _pf_script_state_checkpoint_->_pf_value_12;
        this->higherCenter = _pf_script_state_checkpoint_->_pf_value_13;
        this->higherAtr = _pf_script_state_checkpoint_->_pf_value_14;
        this->entryLine = _pf_script_state_checkpoint_->_pf_value_15;
        this->_ta_initialized_ = _pf_script_state_checkpoint_->_pf_value_16;
        this->_inputs_initialized_ = _pf_script_state_checkpoint_->_pf_value_17;
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() : _ta_ema_1(27), _ta_atr_2(15), _sec0__ta_ema_1(27), _sec0__ta_atr_2(15) {
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

    void configure_security_evaluators() override {
        security_eval_states_.clear();
        register_security_eval(0, get_input_string("Higher Timeframe", std::string("240")), input_tf_, false, false);
    }

    void on_bar(const Bar& bar) override {
        if (!_inputs_initialized_) {
            higherTimeframe = get_input_string("Higher Timeframe", std::string("240"));
            centerLength = get_input_int("Higher EMA", 27);
            atrLength = get_input_int("Higher ATR", 15);
            entryDistance = get_input_double("Entry Distance ATR", 0.35);
            stopAtr = get_input_double("Stop Higher ATR", 2.0);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_ema_1 = ta::EMA(get_input_int("Higher EMA", 27));
            _ta_atr_2 = ta::ATR(get_input_int("Higher ATR", 15));
            _sec0__ta_ema_1 = ta::EMA(get_input_int("Higher EMA", 27));
            _sec0__ta_atr_2 = ta::ATR(get_input_int("Higher ATR", 15));
            _ta_initialized_ = true;
        }
        auto _tuple_result_0 = std::make_tuple(_req_sec_0_0, _req_sec_0_1);
        higherCenter = std::get<0>(_tuple_result_0);
        higherAtr = std::get<1>(_tuple_result_0);
        entryLine = (higherCenter + (higherAtr * entryDistance));
        if ((history_advances_new_bar() ? _ta_crossover_3.compute(current_bar_.close, entryLine) : _ta_crossover_3.recompute(current_bar_.close, entryLine))) {
            strategy_entry(std::string("Cascade Long"), true, na<double>(), na<double>(), na<double>(), "");
        }
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (higherCenter); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()))) {
            strategy_close(std::string("Cascade Long"), std::string("Higher center lost"), na<double>(), na<double>(), false, 120259084307ULL);
        }
        if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) {
            strategy_exit(std::string("Cascade Guard"), std::string("Cascade Long"), na<double>(), ((signed_position_size() == 0.0 ? na<double>() : position_entry_price_) - (higherAtr * stopAtr)), na<double>(), na<double>(), na<double>(), 100.0, "", na<double>(), "", na<double>(), na<double>());
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_ema_1.resize(n);
        _precalc__ta_atr_2.resize(n);

        _ta_ema_1 = ta::EMA(27);
        _ta_atr_2 = ta::ATR(15);


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
            _precalc__ta_ema_1[i] = _ta_ema_1.compute(bars[i].close);
            _precalc__ta_atr_2[i] = _ta_atr_2.compute(bars[i].high, bars[i].low, bars[i].close);
        }

        _ta_ema_1 = ta::EMA(27);
        _ta_atr_2 = ta::ATR(15);

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

    void _eval_security_0(const Bar& bar, bool is_complete) {
        auto _secval_0 = security_series_slot_is_new(0) ? _sec0__ta_ema_1.compute(bar.close) : _sec0__ta_ema_1.recompute(bar.close);
        auto _secval_1 = security_series_slot_is_new(0) ? _sec0__ta_atr_2.compute(bar.high, bar.low, bar.close) : _sec0__ta_atr_2.recompute(bar.high, bar.low, bar.close);
        _req_sec_0_0 = _secval_0;
        _req_sec_0_1 = _secval_1;
    }

    void evaluate_security(int sec_id, const Bar& bar, bool is_complete) override {
        if (!_ta_initialized_) {
            _ta_ema_1 = ta::EMA(get_input_int("Higher EMA", 27));
            _ta_atr_2 = ta::ATR(get_input_int("Higher ATR", 15));
            _sec0__ta_ema_1 = ta::EMA(get_input_int("Higher EMA", 27));
            _sec0__ta_atr_2 = ta::ATR(get_input_int("Higher ATR", 15));
            _ta_initialized_ = true;
        }
        switch (sec_id) {
            case 0: _eval_security_0(bar, is_complete); break;
        }
    }
    void clear_security(int sec_id) override {
        switch (sec_id) {
            case 0:
                _req_sec_0_0 = na<double>();
                _req_sec_0_1 = na<double>();
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
