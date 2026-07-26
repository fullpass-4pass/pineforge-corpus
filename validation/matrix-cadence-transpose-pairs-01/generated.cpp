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

template <typename _PFValue>
struct _PFCheckpointTraits {
    using snapshot_type = _PFValue;
    static snapshot_type take(const _PFValue& value) { return value; }
    static void restore(_PFValue& value, const snapshot_type& snapshot) {
        value = snapshot;
    }
};

template <>
struct _PFCheckpointTraits<PineMatrix> {
    using matrix_type = PineMatrix;
    using snapshot_type = std::optional<typename matrix_type::Snapshot>;
    static snapshot_type take(const matrix_type& value) {
        if (value.is_na()) return std::nullopt;
        return value.snapshot();
    }
    static void restore(matrix_type& value, const snapshot_type& snapshot) {
        if (!snapshot) {
            value = matrix_type{};
            return;
        }
        value.restore(*snapshot);
    }
};

template <typename _PFElement, typename _PFAllocator>
struct _PFCheckpointTraits<std::vector<_PFElement, _PFAllocator>> {
    using element_traits = _PFCheckpointTraits<_PFElement>;
    using element_snapshot = typename element_traits::snapshot_type;
    using snapshot_type = std::vector<element_snapshot>;
    static snapshot_type take(
            const std::vector<_PFElement, _PFAllocator>& value) {
        snapshot_type snapshot;
        snapshot.reserve(value.size());
        for (std::size_t index = 0; index < value.size(); ++index) {
            const _PFElement element = value[index];
            snapshot.push_back(element_traits::take(element));
        }
        return snapshot;
    }
    static void restore(
            std::vector<_PFElement, _PFAllocator>& value,
            const snapshot_type& snapshot) {
        value.clear();
        value.reserve(snapshot.size());
        for (const auto& element_snapshot_value : snapshot) {
            _PFElement element{};
            element_traits::restore(element, element_snapshot_value);
            value.push_back(element);
        }
    }
};

class GeneratedStrategy : public BacktestEngine {
public:
    ta::SMA _ta_sma_1;
    std::vector<double> _precalc__ta_sma_1;
    ta::SMA _ta_sma_2;
    std::vector<double> _precalc__ta_sma_2;
    ta::SMA _ta_sma_3;
    std::vector<double> _precalc__ta_sma_3;
    ta::EMA _ta_ema_4;
    std::vector<double> _precalc__ta_ema_4;
    ta::ATR _ta_atr_5;
    std::vector<double> _precalc__ta_atr_5;
    bool _use_precalc = false;
    Series<double> _s_close;
    PineMatrix factorRows;
    Series<bool> consensusReady;
    int shortHorizon = 0;
    int mediumHorizon = 0;
    int longHorizon = 0;
    int requiredPairs = 0;
    int trendLength = 0;
    int atrLength = 0;
    double maximumAtrLoss = 0.0;
    double shortPriceUp = 0.0;
    double mediumPriceUp = 0.0;
    double longPriceUp = 0.0;
    double shortVolumeFirm = 0.0;
    double mediumVolumeFirm = 0.0;
    double longVolumeFirm = 0.0;
    PineMatrix horizonPairs;
    int confirmedPairs = 0;
    double trendLine = 0.0;
    double atrValue = 0.0;
    bool enterLong = false;
    bool consensusEnded = false;
    bool riskExceeded = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_1)>::snapshot_type _pf_value_0;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_2)>::snapshot_type _pf_value_1;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_3)>::snapshot_type _pf_value_2;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_4)>::snapshot_type _pf_value_3;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_atr_5)>::snapshot_type _pf_value_4;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_s_close)>::snapshot_type _pf_value_5;
        _PFCheckpointTraits<decltype(GeneratedStrategy::factorRows)>::snapshot_type _pf_value_6;
        _PFCheckpointTraits<decltype(GeneratedStrategy::consensusReady)>::snapshot_type _pf_value_7;
        _PFCheckpointTraits<decltype(GeneratedStrategy::shortHorizon)>::snapshot_type _pf_value_8;
        _PFCheckpointTraits<decltype(GeneratedStrategy::mediumHorizon)>::snapshot_type _pf_value_9;
        _PFCheckpointTraits<decltype(GeneratedStrategy::longHorizon)>::snapshot_type _pf_value_10;
        _PFCheckpointTraits<decltype(GeneratedStrategy::requiredPairs)>::snapshot_type _pf_value_11;
        _PFCheckpointTraits<decltype(GeneratedStrategy::trendLength)>::snapshot_type _pf_value_12;
        _PFCheckpointTraits<decltype(GeneratedStrategy::atrLength)>::snapshot_type _pf_value_13;
        _PFCheckpointTraits<decltype(GeneratedStrategy::maximumAtrLoss)>::snapshot_type _pf_value_14;
        _PFCheckpointTraits<decltype(GeneratedStrategy::shortPriceUp)>::snapshot_type _pf_value_15;
        _PFCheckpointTraits<decltype(GeneratedStrategy::mediumPriceUp)>::snapshot_type _pf_value_16;
        _PFCheckpointTraits<decltype(GeneratedStrategy::longPriceUp)>::snapshot_type _pf_value_17;
        _PFCheckpointTraits<decltype(GeneratedStrategy::shortVolumeFirm)>::snapshot_type _pf_value_18;
        _PFCheckpointTraits<decltype(GeneratedStrategy::mediumVolumeFirm)>::snapshot_type _pf_value_19;
        _PFCheckpointTraits<decltype(GeneratedStrategy::longVolumeFirm)>::snapshot_type _pf_value_20;
        _PFCheckpointTraits<decltype(GeneratedStrategy::horizonPairs)>::snapshot_type _pf_value_21;
        _PFCheckpointTraits<decltype(GeneratedStrategy::confirmedPairs)>::snapshot_type _pf_value_22;
        _PFCheckpointTraits<decltype(GeneratedStrategy::trendLine)>::snapshot_type _pf_value_23;
        _PFCheckpointTraits<decltype(GeneratedStrategy::atrValue)>::snapshot_type _pf_value_24;
        _PFCheckpointTraits<decltype(GeneratedStrategy::enterLong)>::snapshot_type _pf_value_25;
        _PFCheckpointTraits<decltype(GeneratedStrategy::consensusEnded)>::snapshot_type _pf_value_26;
        _PFCheckpointTraits<decltype(GeneratedStrategy::riskExceeded)>::snapshot_type _pf_value_27;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_var_initialized)>::snapshot_type _pf_value_28;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_initialized_)>::snapshot_type _pf_value_29;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_inputs_initialized_)>::snapshot_type _pf_value_30;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_1)>::take(_ta_sma_1),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_2)>::take(_ta_sma_2),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_3)>::take(_ta_sma_3),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_4)>::take(_ta_ema_4),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_atr_5)>::take(_ta_atr_5),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_s_close)>::take(_s_close),
            _PFCheckpointTraits<decltype(GeneratedStrategy::factorRows)>::take(factorRows),
            _PFCheckpointTraits<decltype(GeneratedStrategy::consensusReady)>::take(consensusReady),
            _PFCheckpointTraits<decltype(GeneratedStrategy::shortHorizon)>::take(shortHorizon),
            _PFCheckpointTraits<decltype(GeneratedStrategy::mediumHorizon)>::take(mediumHorizon),
            _PFCheckpointTraits<decltype(GeneratedStrategy::longHorizon)>::take(longHorizon),
            _PFCheckpointTraits<decltype(GeneratedStrategy::requiredPairs)>::take(requiredPairs),
            _PFCheckpointTraits<decltype(GeneratedStrategy::trendLength)>::take(trendLength),
            _PFCheckpointTraits<decltype(GeneratedStrategy::atrLength)>::take(atrLength),
            _PFCheckpointTraits<decltype(GeneratedStrategy::maximumAtrLoss)>::take(maximumAtrLoss),
            _PFCheckpointTraits<decltype(GeneratedStrategy::shortPriceUp)>::take(shortPriceUp),
            _PFCheckpointTraits<decltype(GeneratedStrategy::mediumPriceUp)>::take(mediumPriceUp),
            _PFCheckpointTraits<decltype(GeneratedStrategy::longPriceUp)>::take(longPriceUp),
            _PFCheckpointTraits<decltype(GeneratedStrategy::shortVolumeFirm)>::take(shortVolumeFirm),
            _PFCheckpointTraits<decltype(GeneratedStrategy::mediumVolumeFirm)>::take(mediumVolumeFirm),
            _PFCheckpointTraits<decltype(GeneratedStrategy::longVolumeFirm)>::take(longVolumeFirm),
            _PFCheckpointTraits<decltype(GeneratedStrategy::horizonPairs)>::take(horizonPairs),
            _PFCheckpointTraits<decltype(GeneratedStrategy::confirmedPairs)>::take(confirmedPairs),
            _PFCheckpointTraits<decltype(GeneratedStrategy::trendLine)>::take(trendLine),
            _PFCheckpointTraits<decltype(GeneratedStrategy::atrValue)>::take(atrValue),
            _PFCheckpointTraits<decltype(GeneratedStrategy::enterLong)>::take(enterLong),
            _PFCheckpointTraits<decltype(GeneratedStrategy::consensusEnded)>::take(consensusEnded),
            _PFCheckpointTraits<decltype(GeneratedStrategy::riskExceeded)>::take(riskExceeded),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_var_initialized)>::take(_var_initialized),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_initialized_)>::take(_ta_initialized_),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_inputs_initialized_)>::take(_inputs_initialized_),
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_1)>::restore(this->_ta_sma_1, _pf_script_state_checkpoint_->_pf_value_0);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_2)>::restore(this->_ta_sma_2, _pf_script_state_checkpoint_->_pf_value_1);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_sma_3)>::restore(this->_ta_sma_3, _pf_script_state_checkpoint_->_pf_value_2);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_4)>::restore(this->_ta_ema_4, _pf_script_state_checkpoint_->_pf_value_3);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_atr_5)>::restore(this->_ta_atr_5, _pf_script_state_checkpoint_->_pf_value_4);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_s_close)>::restore(this->_s_close, _pf_script_state_checkpoint_->_pf_value_5);
        _PFCheckpointTraits<decltype(GeneratedStrategy::factorRows)>::restore(this->factorRows, _pf_script_state_checkpoint_->_pf_value_6);
        _PFCheckpointTraits<decltype(GeneratedStrategy::consensusReady)>::restore(this->consensusReady, _pf_script_state_checkpoint_->_pf_value_7);
        _PFCheckpointTraits<decltype(GeneratedStrategy::shortHorizon)>::restore(this->shortHorizon, _pf_script_state_checkpoint_->_pf_value_8);
        _PFCheckpointTraits<decltype(GeneratedStrategy::mediumHorizon)>::restore(this->mediumHorizon, _pf_script_state_checkpoint_->_pf_value_9);
        _PFCheckpointTraits<decltype(GeneratedStrategy::longHorizon)>::restore(this->longHorizon, _pf_script_state_checkpoint_->_pf_value_10);
        _PFCheckpointTraits<decltype(GeneratedStrategy::requiredPairs)>::restore(this->requiredPairs, _pf_script_state_checkpoint_->_pf_value_11);
        _PFCheckpointTraits<decltype(GeneratedStrategy::trendLength)>::restore(this->trendLength, _pf_script_state_checkpoint_->_pf_value_12);
        _PFCheckpointTraits<decltype(GeneratedStrategy::atrLength)>::restore(this->atrLength, _pf_script_state_checkpoint_->_pf_value_13);
        _PFCheckpointTraits<decltype(GeneratedStrategy::maximumAtrLoss)>::restore(this->maximumAtrLoss, _pf_script_state_checkpoint_->_pf_value_14);
        _PFCheckpointTraits<decltype(GeneratedStrategy::shortPriceUp)>::restore(this->shortPriceUp, _pf_script_state_checkpoint_->_pf_value_15);
        _PFCheckpointTraits<decltype(GeneratedStrategy::mediumPriceUp)>::restore(this->mediumPriceUp, _pf_script_state_checkpoint_->_pf_value_16);
        _PFCheckpointTraits<decltype(GeneratedStrategy::longPriceUp)>::restore(this->longPriceUp, _pf_script_state_checkpoint_->_pf_value_17);
        _PFCheckpointTraits<decltype(GeneratedStrategy::shortVolumeFirm)>::restore(this->shortVolumeFirm, _pf_script_state_checkpoint_->_pf_value_18);
        _PFCheckpointTraits<decltype(GeneratedStrategy::mediumVolumeFirm)>::restore(this->mediumVolumeFirm, _pf_script_state_checkpoint_->_pf_value_19);
        _PFCheckpointTraits<decltype(GeneratedStrategy::longVolumeFirm)>::restore(this->longVolumeFirm, _pf_script_state_checkpoint_->_pf_value_20);
        _PFCheckpointTraits<decltype(GeneratedStrategy::horizonPairs)>::restore(this->horizonPairs, _pf_script_state_checkpoint_->_pf_value_21);
        _PFCheckpointTraits<decltype(GeneratedStrategy::confirmedPairs)>::restore(this->confirmedPairs, _pf_script_state_checkpoint_->_pf_value_22);
        _PFCheckpointTraits<decltype(GeneratedStrategy::trendLine)>::restore(this->trendLine, _pf_script_state_checkpoint_->_pf_value_23);
        _PFCheckpointTraits<decltype(GeneratedStrategy::atrValue)>::restore(this->atrValue, _pf_script_state_checkpoint_->_pf_value_24);
        _PFCheckpointTraits<decltype(GeneratedStrategy::enterLong)>::restore(this->enterLong, _pf_script_state_checkpoint_->_pf_value_25);
        _PFCheckpointTraits<decltype(GeneratedStrategy::consensusEnded)>::restore(this->consensusEnded, _pf_script_state_checkpoint_->_pf_value_26);
        _PFCheckpointTraits<decltype(GeneratedStrategy::riskExceeded)>::restore(this->riskExceeded, _pf_script_state_checkpoint_->_pf_value_27);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_var_initialized)>::restore(this->_var_initialized, _pf_script_state_checkpoint_->_pf_value_28);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_initialized_)>::restore(this->_ta_initialized_, _pf_script_state_checkpoint_->_pf_value_29);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_inputs_initialized_)>::restore(this->_inputs_initialized_, _pf_script_state_checkpoint_->_pf_value_30);
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() : _ta_sma_1(6), _ta_sma_2(18), _ta_sma_3(54), _ta_ema_4(43), _ta_atr_5(16) {
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
        if (!_var_initialized) {
            factorRows = PineMatrix::new_(2, 3, 0.0);
            _var_initialized = true;
        } else {
        }
        if (!_inputs_initialized_) {
            shortHorizon = get_input_int("Short Horizon", 6);
            mediumHorizon = get_input_int("Medium Horizon", 18);
            longHorizon = get_input_int("Long Horizon", 54);
            requiredPairs = get_input_int("Required Confirmed Pairs", 2);
            trendLength = get_input_int("Trend EMA Length", 43);
            atrLength = get_input_int("ATR Length", 16);
            maximumAtrLoss = get_input_double("Maximum ATR Loss", 2.4);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_sma_1 = ta::SMA(get_input_int("Short Horizon", 6));
            _ta_sma_2 = ta::SMA(get_input_int("Medium Horizon", 18));
            _ta_sma_3 = ta::SMA(get_input_int("Long Horizon", 54));
            _ta_ema_4 = ta::EMA(get_input_int("Trend EMA Length", 43));
            _ta_atr_5 = ta::ATR(get_input_int("ATR Length", 16));
            _ta_initialized_ = true;
        }
        shortPriceUp = ((([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (_s_close[shortHorizon]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (1.0) : (0.0));
        mediumPriceUp = ((([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (_s_close[mediumHorizon]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (1.0) : (0.0));
        longPriceUp = ((([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (_s_close[longHorizon]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (1.0) : (0.0));
        shortVolumeFirm = ((([&]{ auto _pna_l = (current_bar_.volume); auto _pna_r = ((_use_precalc ? _precalc__ta_sma_1[bar_index_] : (history_advances_new_bar() ? _ta_sma_1.compute(current_bar_.volume) : _ta_sma_1.recompute(current_bar_.volume)))); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (1.0) : (0.0));
        mediumVolumeFirm = ((([&]{ auto _pna_l = (current_bar_.volume); auto _pna_r = ((_use_precalc ? _precalc__ta_sma_2[bar_index_] : (history_advances_new_bar() ? _ta_sma_2.compute(current_bar_.volume) : _ta_sma_2.recompute(current_bar_.volume)))); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (1.0) : (0.0));
        longVolumeFirm = ((([&]{ auto _pna_l = (current_bar_.volume); auto _pna_r = ((_use_precalc ? _precalc__ta_sma_3[bar_index_] : (history_advances_new_bar() ? _ta_sma_3.compute(current_bar_.volume) : _ta_sma_3.recompute(current_bar_.volume)))); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (1.0) : (0.0));
        factorRows.set((int)(0), (int)(0), shortPriceUp);
        factorRows.set((int)(0), (int)(1), mediumPriceUp);
        factorRows.set((int)(0), (int)(2), longPriceUp);
        factorRows.set((int)(1), (int)(0), shortVolumeFirm);
        factorRows.set((int)(1), (int)(1), mediumVolumeFirm);
        factorRows.set((int)(1), (int)(2), longVolumeFirm);
        horizonPairs = factorRows.transpose();
        confirmedPairs = 0;
        int _for_start_0 = (0);
        int _for_end_0 = (2);
        int _for_step_0 = (1);
        if (_for_step_0 < 0) _for_step_0 = -_for_step_0;
        if (_for_step_0 == 0) _for_step_0 = 1;
        const bool _for_down_0 = (_for_start_0 > _for_end_0);
        for (int horizonIndex = _for_start_0; (_for_down_0 ? (horizonIndex >= _for_end_0) : (horizonIndex <= _for_end_0)); horizonIndex += (_for_down_0 ? -_for_step_0 : _for_step_0), _for_end_0 = (2)) {
            double priceVote = horizonPairs.get((int)(horizonIndex), (int)(0));
            double volumeVote = horizonPairs.get((int)(horizonIndex), (int)(1));
            if ((([&]{ auto _pna_l = (priceVote); auto _pna_r = (1.0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()) && ([&]{ auto _pna_l = (volumeVote); auto _pna_r = (1.0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()))) {
                confirmedPairs += 1;
            }
        }
        trendLine = (history_advances_new_bar() ? _ta_ema_4.compute(current_bar_.close) : _ta_ema_4.recompute(current_bar_.close));
        atrValue = (history_advances_new_bar() ? _ta_atr_5.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_5.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        if (history_advances_new_bar()) consensusReady.push(([&]{ auto _pna_l = (confirmedPairs); auto _pna_r = (requiredPairs); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l >= _pna_r); }()));
        else consensusReady.update(([&]{ auto _pna_l = (confirmedPairs); auto _pna_r = (requiredPairs); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l >= _pna_r); }()));
        enterLong = ((consensusReady[0] && !(consensusReady[1])) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (trendLine); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()));
        consensusEnded = (([&]{ auto _pna_l = (confirmedPairs); auto _pna_r = (0); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l == _pna_r); }()) || ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (trendLine); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        riskExceeded = (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (((signed_position_size() == 0.0 ? na<double>() : position_entry_price_) - (atrValue * maximumAtrLoss))); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()) && enterLong)) {
            strategy_entry(std::string("Cadence Long"), true, na<double>(), na<double>(), na<double>(), "");
        } else
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && (consensusEnded || riskExceeded))) {
            strategy_close(std::string("Cadence Long"), ((riskExceeded) ? (std::string("ATR risk")) : (std::string("Pair consensus"))), na<double>(), na<double>(), false, 240518168595ULL);
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_sma_1.resize(n);
        _precalc__ta_sma_2.resize(n);
        _precalc__ta_sma_3.resize(n);
        _precalc__ta_ema_4.resize(n);
        _precalc__ta_atr_5.resize(n);

        _ta_sma_1 = ta::SMA(6);
        _ta_sma_2 = ta::SMA(18);
        _ta_sma_3 = ta::SMA(54);
        _ta_ema_4 = ta::EMA(43);
        _ta_atr_5 = ta::ATR(16);

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
            _precalc__ta_sma_1[i] = _ta_sma_1.compute(bars[i].volume);
            _precalc__ta_sma_2[i] = _ta_sma_2.compute(bars[i].volume);
            _precalc__ta_sma_3[i] = _ta_sma_3.compute(bars[i].volume);
            _precalc__ta_ema_4[i] = _ta_ema_4.compute(bars[i].close);
            _precalc__ta_atr_5[i] = _ta_atr_5.compute(bars[i].high, bars[i].low, bars[i].close);
        }

        _ta_sma_1 = ta::SMA(6);
        _ta_sma_2 = ta::SMA(18);
        _ta_sma_3 = ta::SMA(54);
        _ta_ema_4 = ta::EMA(43);
        _ta_atr_5 = ta::ATR(16);
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
