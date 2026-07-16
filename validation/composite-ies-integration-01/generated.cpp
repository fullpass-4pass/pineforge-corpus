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
    ta::TR _ta_tr_1;
    ta::RMA _ta_rma_2;
    ta::RMA _ta_rma_3;
    ta::RMA _ta_rma_4;
    ta::RMA _ta_rma_5;
    ta::ATR _ta_atr_6;
    std::vector<double> _precalc__ta_atr_6;
    ta::SMA _ta_sma_7;
    ta::EMA _ta_ema_8;
    std::vector<double> _precalc__ta_ema_8;
    ta::EMA _ta_ema_9;
    std::vector<double> _precalc__ta_ema_9;
    ta::EMA _ta_ema_10;
    std::vector<double> _precalc__ta_ema_10;
    ta::RSI _ta_rsi_11;
    std::vector<double> _precalc__ta_rsi_11;
    ta::EMA _ta_ema_12;
    std::vector<double> _precalc__ta_ema_12;
    ta::EMA _ta_ema_13;
    std::vector<double> _precalc__ta_ema_13;
    ta::EMA _ta_ema_14;
    ta::EMA _ta_ema_15;
    ta::EMA _ta_ema_16;
    bool _use_precalc = false;
    Series<double> _s_high;
    Series<double> _s_low;
    int bars_since_trade;
    Series<double> macd_hist;
    Series<double> press_s;
    Series<double> rsi_v;
    int i_adx_len = 0;
    double i_adx_trend = 0.0;
    int i_atr_len = 0;
    double i_vol_exp = 0.0;
    double i_vol_con = 0.0;
    int i_ma_fast = 0;
    int i_ma_slow = 0;
    int i_ma_trend = 0;
    double i_bias_thresh = 0.0;
    int i_rsi_len = 0;
    double i_rsi_bull = 0.0;
    double i_rsi_bear = 0.0;
    int i_macd_fast = 0;
    int i_macd_slow = 0;
    int i_macd_sig = 0;
    int i_press_len = 0;
    int i_press_smo = 0;
    int i_press_mom = 0;
    double i_press_thr = 0.0;
    int i_cooldown = 0;
    double adx_v = 0.0;
    double p_di_v = 0.0;
    double m_di_v = 0.0;
    double atr_v = 0.0;
    double atr_avg = 0.0;
    double vol_ratio = 0.0;
    int regime = 0;
    bool trending = false;
    bool di_bull = false;
    bool di_bear = false;
    double ma_fast = 0.0;
    double ma_slow = 0.0;
    double ma_trend = 0.0;
    bool stack_up = false;
    bool stack_dn = false;
    bool above_str = false;
    bool below_str = false;
    double bull_bias = 0.0;
    double bear_bias = 0.0;
    bool bias_bull = false;
    bool bias_bear = false;
    bool rsi_bull = false;
    bool rsi_bear = false;
    bool rsi_mom_up = false;
    bool rsi_mom_dn = false;
    double macd_line = 0.0;
    double macd_sig = 0.0;
    bool macd_bull = false;
    bool macd_bear = false;
    int mom_bull = 0;
    int mom_bear = 0;
    bool mom_bull_ok = false;
    bool mom_bear_ok = false;
    double bar_range = 0.0;
    double raw_press = 0.0;
    double press_r = 0.0;
    double press_mom = 0.0;
    bool press_bull = false;
    bool press_bear = false;
    int bull_total = 0;
    int bear_total = 0;
    bool cooldown_ok = false;
    bool long_entry = false;
    bool short_entry = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        decltype(GeneratedStrategy::_ta_tr_1) _pf_value_0;
        decltype(GeneratedStrategy::_ta_rma_2) _pf_value_1;
        decltype(GeneratedStrategy::_ta_rma_3) _pf_value_2;
        decltype(GeneratedStrategy::_ta_rma_4) _pf_value_3;
        decltype(GeneratedStrategy::_ta_rma_5) _pf_value_4;
        decltype(GeneratedStrategy::_ta_atr_6) _pf_value_5;
        decltype(GeneratedStrategy::_ta_sma_7) _pf_value_6;
        decltype(GeneratedStrategy::_ta_ema_8) _pf_value_7;
        decltype(GeneratedStrategy::_ta_ema_9) _pf_value_8;
        decltype(GeneratedStrategy::_ta_ema_10) _pf_value_9;
        decltype(GeneratedStrategy::_ta_rsi_11) _pf_value_10;
        decltype(GeneratedStrategy::_ta_ema_12) _pf_value_11;
        decltype(GeneratedStrategy::_ta_ema_13) _pf_value_12;
        decltype(GeneratedStrategy::_ta_ema_14) _pf_value_13;
        decltype(GeneratedStrategy::_ta_ema_15) _pf_value_14;
        decltype(GeneratedStrategy::_ta_ema_16) _pf_value_15;
        decltype(GeneratedStrategy::_s_high) _pf_value_16;
        decltype(GeneratedStrategy::_s_low) _pf_value_17;
        decltype(GeneratedStrategy::bars_since_trade) _pf_value_18;
        decltype(GeneratedStrategy::macd_hist) _pf_value_19;
        decltype(GeneratedStrategy::press_s) _pf_value_20;
        decltype(GeneratedStrategy::rsi_v) _pf_value_21;
        decltype(GeneratedStrategy::i_adx_len) _pf_value_22;
        decltype(GeneratedStrategy::i_adx_trend) _pf_value_23;
        decltype(GeneratedStrategy::i_atr_len) _pf_value_24;
        decltype(GeneratedStrategy::i_vol_exp) _pf_value_25;
        decltype(GeneratedStrategy::i_vol_con) _pf_value_26;
        decltype(GeneratedStrategy::i_ma_fast) _pf_value_27;
        decltype(GeneratedStrategy::i_ma_slow) _pf_value_28;
        decltype(GeneratedStrategy::i_ma_trend) _pf_value_29;
        decltype(GeneratedStrategy::i_bias_thresh) _pf_value_30;
        decltype(GeneratedStrategy::i_rsi_len) _pf_value_31;
        decltype(GeneratedStrategy::i_rsi_bull) _pf_value_32;
        decltype(GeneratedStrategy::i_rsi_bear) _pf_value_33;
        decltype(GeneratedStrategy::i_macd_fast) _pf_value_34;
        decltype(GeneratedStrategy::i_macd_slow) _pf_value_35;
        decltype(GeneratedStrategy::i_macd_sig) _pf_value_36;
        decltype(GeneratedStrategy::i_press_len) _pf_value_37;
        decltype(GeneratedStrategy::i_press_smo) _pf_value_38;
        decltype(GeneratedStrategy::i_press_mom) _pf_value_39;
        decltype(GeneratedStrategy::i_press_thr) _pf_value_40;
        decltype(GeneratedStrategy::i_cooldown) _pf_value_41;
        decltype(GeneratedStrategy::adx_v) _pf_value_42;
        decltype(GeneratedStrategy::p_di_v) _pf_value_43;
        decltype(GeneratedStrategy::m_di_v) _pf_value_44;
        decltype(GeneratedStrategy::atr_v) _pf_value_45;
        decltype(GeneratedStrategy::atr_avg) _pf_value_46;
        decltype(GeneratedStrategy::vol_ratio) _pf_value_47;
        decltype(GeneratedStrategy::regime) _pf_value_48;
        decltype(GeneratedStrategy::trending) _pf_value_49;
        decltype(GeneratedStrategy::di_bull) _pf_value_50;
        decltype(GeneratedStrategy::di_bear) _pf_value_51;
        decltype(GeneratedStrategy::ma_fast) _pf_value_52;
        decltype(GeneratedStrategy::ma_slow) _pf_value_53;
        decltype(GeneratedStrategy::ma_trend) _pf_value_54;
        decltype(GeneratedStrategy::stack_up) _pf_value_55;
        decltype(GeneratedStrategy::stack_dn) _pf_value_56;
        decltype(GeneratedStrategy::above_str) _pf_value_57;
        decltype(GeneratedStrategy::below_str) _pf_value_58;
        decltype(GeneratedStrategy::bull_bias) _pf_value_59;
        decltype(GeneratedStrategy::bear_bias) _pf_value_60;
        decltype(GeneratedStrategy::bias_bull) _pf_value_61;
        decltype(GeneratedStrategy::bias_bear) _pf_value_62;
        decltype(GeneratedStrategy::rsi_bull) _pf_value_63;
        decltype(GeneratedStrategy::rsi_bear) _pf_value_64;
        decltype(GeneratedStrategy::rsi_mom_up) _pf_value_65;
        decltype(GeneratedStrategy::rsi_mom_dn) _pf_value_66;
        decltype(GeneratedStrategy::macd_line) _pf_value_67;
        decltype(GeneratedStrategy::macd_sig) _pf_value_68;
        decltype(GeneratedStrategy::macd_bull) _pf_value_69;
        decltype(GeneratedStrategy::macd_bear) _pf_value_70;
        decltype(GeneratedStrategy::mom_bull) _pf_value_71;
        decltype(GeneratedStrategy::mom_bear) _pf_value_72;
        decltype(GeneratedStrategy::mom_bull_ok) _pf_value_73;
        decltype(GeneratedStrategy::mom_bear_ok) _pf_value_74;
        decltype(GeneratedStrategy::bar_range) _pf_value_75;
        decltype(GeneratedStrategy::raw_press) _pf_value_76;
        decltype(GeneratedStrategy::press_r) _pf_value_77;
        decltype(GeneratedStrategy::press_mom) _pf_value_78;
        decltype(GeneratedStrategy::press_bull) _pf_value_79;
        decltype(GeneratedStrategy::press_bear) _pf_value_80;
        decltype(GeneratedStrategy::bull_total) _pf_value_81;
        decltype(GeneratedStrategy::bear_total) _pf_value_82;
        decltype(GeneratedStrategy::cooldown_ok) _pf_value_83;
        decltype(GeneratedStrategy::long_entry) _pf_value_84;
        decltype(GeneratedStrategy::short_entry) _pf_value_85;
        decltype(GeneratedStrategy::_var_initialized) _pf_value_86;
        decltype(GeneratedStrategy::_ta_initialized_) _pf_value_87;
        decltype(GeneratedStrategy::_inputs_initialized_) _pf_value_88;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _ta_tr_1,
            _ta_rma_2,
            _ta_rma_3,
            _ta_rma_4,
            _ta_rma_5,
            _ta_atr_6,
            _ta_sma_7,
            _ta_ema_8,
            _ta_ema_9,
            _ta_ema_10,
            _ta_rsi_11,
            _ta_ema_12,
            _ta_ema_13,
            _ta_ema_14,
            _ta_ema_15,
            _ta_ema_16,
            _s_high,
            _s_low,
            bars_since_trade,
            macd_hist,
            press_s,
            rsi_v,
            i_adx_len,
            i_adx_trend,
            i_atr_len,
            i_vol_exp,
            i_vol_con,
            i_ma_fast,
            i_ma_slow,
            i_ma_trend,
            i_bias_thresh,
            i_rsi_len,
            i_rsi_bull,
            i_rsi_bear,
            i_macd_fast,
            i_macd_slow,
            i_macd_sig,
            i_press_len,
            i_press_smo,
            i_press_mom,
            i_press_thr,
            i_cooldown,
            adx_v,
            p_di_v,
            m_di_v,
            atr_v,
            atr_avg,
            vol_ratio,
            regime,
            trending,
            di_bull,
            di_bear,
            ma_fast,
            ma_slow,
            ma_trend,
            stack_up,
            stack_dn,
            above_str,
            below_str,
            bull_bias,
            bear_bias,
            bias_bull,
            bias_bear,
            rsi_bull,
            rsi_bear,
            rsi_mom_up,
            rsi_mom_dn,
            macd_line,
            macd_sig,
            macd_bull,
            macd_bear,
            mom_bull,
            mom_bear,
            mom_bull_ok,
            mom_bear_ok,
            bar_range,
            raw_press,
            press_r,
            press_mom,
            press_bull,
            press_bear,
            bull_total,
            bear_total,
            cooldown_ok,
            long_entry,
            short_entry,
            _var_initialized,
            _ta_initialized_,
            _inputs_initialized_,
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        this->_ta_tr_1 = _pf_script_state_checkpoint_->_pf_value_0;
        this->_ta_rma_2 = _pf_script_state_checkpoint_->_pf_value_1;
        this->_ta_rma_3 = _pf_script_state_checkpoint_->_pf_value_2;
        this->_ta_rma_4 = _pf_script_state_checkpoint_->_pf_value_3;
        this->_ta_rma_5 = _pf_script_state_checkpoint_->_pf_value_4;
        this->_ta_atr_6 = _pf_script_state_checkpoint_->_pf_value_5;
        this->_ta_sma_7 = _pf_script_state_checkpoint_->_pf_value_6;
        this->_ta_ema_8 = _pf_script_state_checkpoint_->_pf_value_7;
        this->_ta_ema_9 = _pf_script_state_checkpoint_->_pf_value_8;
        this->_ta_ema_10 = _pf_script_state_checkpoint_->_pf_value_9;
        this->_ta_rsi_11 = _pf_script_state_checkpoint_->_pf_value_10;
        this->_ta_ema_12 = _pf_script_state_checkpoint_->_pf_value_11;
        this->_ta_ema_13 = _pf_script_state_checkpoint_->_pf_value_12;
        this->_ta_ema_14 = _pf_script_state_checkpoint_->_pf_value_13;
        this->_ta_ema_15 = _pf_script_state_checkpoint_->_pf_value_14;
        this->_ta_ema_16 = _pf_script_state_checkpoint_->_pf_value_15;
        this->_s_high = _pf_script_state_checkpoint_->_pf_value_16;
        this->_s_low = _pf_script_state_checkpoint_->_pf_value_17;
        this->bars_since_trade = _pf_script_state_checkpoint_->_pf_value_18;
        this->macd_hist = _pf_script_state_checkpoint_->_pf_value_19;
        this->press_s = _pf_script_state_checkpoint_->_pf_value_20;
        this->rsi_v = _pf_script_state_checkpoint_->_pf_value_21;
        this->i_adx_len = _pf_script_state_checkpoint_->_pf_value_22;
        this->i_adx_trend = _pf_script_state_checkpoint_->_pf_value_23;
        this->i_atr_len = _pf_script_state_checkpoint_->_pf_value_24;
        this->i_vol_exp = _pf_script_state_checkpoint_->_pf_value_25;
        this->i_vol_con = _pf_script_state_checkpoint_->_pf_value_26;
        this->i_ma_fast = _pf_script_state_checkpoint_->_pf_value_27;
        this->i_ma_slow = _pf_script_state_checkpoint_->_pf_value_28;
        this->i_ma_trend = _pf_script_state_checkpoint_->_pf_value_29;
        this->i_bias_thresh = _pf_script_state_checkpoint_->_pf_value_30;
        this->i_rsi_len = _pf_script_state_checkpoint_->_pf_value_31;
        this->i_rsi_bull = _pf_script_state_checkpoint_->_pf_value_32;
        this->i_rsi_bear = _pf_script_state_checkpoint_->_pf_value_33;
        this->i_macd_fast = _pf_script_state_checkpoint_->_pf_value_34;
        this->i_macd_slow = _pf_script_state_checkpoint_->_pf_value_35;
        this->i_macd_sig = _pf_script_state_checkpoint_->_pf_value_36;
        this->i_press_len = _pf_script_state_checkpoint_->_pf_value_37;
        this->i_press_smo = _pf_script_state_checkpoint_->_pf_value_38;
        this->i_press_mom = _pf_script_state_checkpoint_->_pf_value_39;
        this->i_press_thr = _pf_script_state_checkpoint_->_pf_value_40;
        this->i_cooldown = _pf_script_state_checkpoint_->_pf_value_41;
        this->adx_v = _pf_script_state_checkpoint_->_pf_value_42;
        this->p_di_v = _pf_script_state_checkpoint_->_pf_value_43;
        this->m_di_v = _pf_script_state_checkpoint_->_pf_value_44;
        this->atr_v = _pf_script_state_checkpoint_->_pf_value_45;
        this->atr_avg = _pf_script_state_checkpoint_->_pf_value_46;
        this->vol_ratio = _pf_script_state_checkpoint_->_pf_value_47;
        this->regime = _pf_script_state_checkpoint_->_pf_value_48;
        this->trending = _pf_script_state_checkpoint_->_pf_value_49;
        this->di_bull = _pf_script_state_checkpoint_->_pf_value_50;
        this->di_bear = _pf_script_state_checkpoint_->_pf_value_51;
        this->ma_fast = _pf_script_state_checkpoint_->_pf_value_52;
        this->ma_slow = _pf_script_state_checkpoint_->_pf_value_53;
        this->ma_trend = _pf_script_state_checkpoint_->_pf_value_54;
        this->stack_up = _pf_script_state_checkpoint_->_pf_value_55;
        this->stack_dn = _pf_script_state_checkpoint_->_pf_value_56;
        this->above_str = _pf_script_state_checkpoint_->_pf_value_57;
        this->below_str = _pf_script_state_checkpoint_->_pf_value_58;
        this->bull_bias = _pf_script_state_checkpoint_->_pf_value_59;
        this->bear_bias = _pf_script_state_checkpoint_->_pf_value_60;
        this->bias_bull = _pf_script_state_checkpoint_->_pf_value_61;
        this->bias_bear = _pf_script_state_checkpoint_->_pf_value_62;
        this->rsi_bull = _pf_script_state_checkpoint_->_pf_value_63;
        this->rsi_bear = _pf_script_state_checkpoint_->_pf_value_64;
        this->rsi_mom_up = _pf_script_state_checkpoint_->_pf_value_65;
        this->rsi_mom_dn = _pf_script_state_checkpoint_->_pf_value_66;
        this->macd_line = _pf_script_state_checkpoint_->_pf_value_67;
        this->macd_sig = _pf_script_state_checkpoint_->_pf_value_68;
        this->macd_bull = _pf_script_state_checkpoint_->_pf_value_69;
        this->macd_bear = _pf_script_state_checkpoint_->_pf_value_70;
        this->mom_bull = _pf_script_state_checkpoint_->_pf_value_71;
        this->mom_bear = _pf_script_state_checkpoint_->_pf_value_72;
        this->mom_bull_ok = _pf_script_state_checkpoint_->_pf_value_73;
        this->mom_bear_ok = _pf_script_state_checkpoint_->_pf_value_74;
        this->bar_range = _pf_script_state_checkpoint_->_pf_value_75;
        this->raw_press = _pf_script_state_checkpoint_->_pf_value_76;
        this->press_r = _pf_script_state_checkpoint_->_pf_value_77;
        this->press_mom = _pf_script_state_checkpoint_->_pf_value_78;
        this->press_bull = _pf_script_state_checkpoint_->_pf_value_79;
        this->press_bear = _pf_script_state_checkpoint_->_pf_value_80;
        this->bull_total = _pf_script_state_checkpoint_->_pf_value_81;
        this->bear_total = _pf_script_state_checkpoint_->_pf_value_82;
        this->cooldown_ok = _pf_script_state_checkpoint_->_pf_value_83;
        this->long_entry = _pf_script_state_checkpoint_->_pf_value_84;
        this->short_entry = _pf_script_state_checkpoint_->_pf_value_85;
        this->_var_initialized = _pf_script_state_checkpoint_->_pf_value_86;
        this->_ta_initialized_ = _pf_script_state_checkpoint_->_pf_value_87;
        this->_inputs_initialized_ = _pf_script_state_checkpoint_->_pf_value_88;
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() : _ta_tr_1(true), _ta_rma_2(14), _ta_rma_3(14), _ta_rma_4(14), _ta_rma_5(14), _ta_atr_6(14), _ta_sma_7(42), _ta_ema_8(21), _ta_ema_9(55), _ta_ema_10(200), _ta_rsi_11(14), _ta_ema_12(12), _ta_ema_13(26), _ta_ema_14(9), _ta_ema_15(14), _ta_ema_16(5), bars_since_trade(999) {
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

    std::tuple<double, double, double> f_adx_chain_cs0(int len) {
        double tr_v = (history_advances_new_bar() ? _ta_tr_1.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_tr_1.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        double p_dm = ([&]() -> double { double _v0 = (double)((current_bar_.high - _s_high[1])); double _v1 = (double)(0); if (is_na(_v0) || is_na(_v1)) return na<double>(); double _out = _v0; _out = std::max(_out, _v1); return _out; }());
        double m_dm = ([&]() -> double { double _v0 = (double)((_s_low[1] - current_bar_.low)); double _v1 = (double)(0); if (is_na(_v0) || is_na(_v1)) return na<double>(); double _out = _v0; _out = std::max(_out, _v1); return _out; }());
        if (([&]{ auto _pna_l = (p_dm); auto _pna_r = (m_dm); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) {
            m_dm = 0;
        } else {
            p_dm = 0;
        }
        double s_tr = (history_advances_new_bar() ? _ta_rma_2.compute(tr_v) : _ta_rma_2.recompute(tr_v));
        double s_p = (history_advances_new_bar() ? _ta_rma_3.compute(p_dm) : _ta_rma_3.recompute(p_dm));
        double s_m = (history_advances_new_bar() ? _ta_rma_4.compute(m_dm) : _ta_rma_4.recompute(m_dm));
        p_di_v = ((([&]{ auto _pna_l = (s_tr); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (((double)((100 * s_p)) / (double)(s_tr))) : (0));
        m_di_v = ((([&]{ auto _pna_l = (s_tr); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (((double)((100 * s_m)) / (double)(s_tr))) : (0));
        double di_sum = (p_di_v + m_di_v);
        double dx_v = ((([&]{ auto _pna_l = (di_sum); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (((double)((100 * std::abs((p_di_v - m_di_v)))) / (double)(di_sum))) : (0));
        adx_v = (history_advances_new_bar() ? _ta_rma_5.compute(dx_v) : _ta_rma_5.recompute(dx_v));
        return std::make_tuple(adx_v, p_di_v, m_di_v);
    }

    void on_bar(const Bar& bar) override {
        if (history_advances_new_bar()) _s_high.push(current_bar_.high);
        else _s_high.update(current_bar_.high);
        if (history_advances_new_bar()) _s_low.push(current_bar_.low);
        else _s_low.update(current_bar_.low);
        if (!_var_initialized) {
            _var_initialized = true;
        } else {
        }
        if (!_inputs_initialized_) {
            i_adx_len = get_input_int("ADX period", 14);
            i_adx_trend = get_input_double("ADX trend threshold", 25);
            i_atr_len = get_input_int("ATR period", 14);
            i_vol_exp = get_input_double("Vol expansion", 1.4);
            i_vol_con = get_input_double("Vol contraction", 0.6);
            i_ma_fast = get_input_int("Bias fast EMA", 21);
            i_ma_slow = get_input_int("Bias slow EMA", 55);
            i_ma_trend = get_input_int("Bias trend EMA", 200);
            i_bias_thresh = get_input_double("Bias score threshold", 30);
            i_rsi_len = get_input_int("RSI period", 14);
            i_rsi_bull = get_input_double("RSI bullish level", 55);
            i_rsi_bear = get_input_double("RSI bearish level", 45);
            i_macd_fast = get_input_int("MACD fast", 12);
            i_macd_slow = get_input_int("MACD slow", 26);
            i_macd_sig = get_input_int("MACD signal", 9);
            i_press_len = get_input_int("Pressure period", 14);
            i_press_smo = get_input_int("Pressure smoothing", 5);
            i_press_mom = get_input_int("Pressure momentum", 10);
            i_press_thr = get_input_double("Pressure mom thresh", 0.05);
            i_cooldown = get_input_int("Cooldown bars", 8);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_rma_2 = ta::RMA(get_input_int("ADX period", 14));
            _ta_rma_3 = ta::RMA(get_input_int("ADX period", 14));
            _ta_rma_4 = ta::RMA(get_input_int("ADX period", 14));
            _ta_rma_5 = ta::RMA(get_input_int("ADX period", 14));
            _ta_atr_6 = ta::ATR(get_input_int("ATR period", 14));
            _ta_sma_7 = ta::SMA((get_input_int("ATR period", 14) * 3));
            _ta_ema_8 = ta::EMA(get_input_int("Bias fast EMA", 21));
            _ta_ema_9 = ta::EMA(get_input_int("Bias slow EMA", 55));
            _ta_ema_10 = ta::EMA(get_input_int("Bias trend EMA", 200));
            _ta_rsi_11 = ta::RSI(get_input_int("RSI period", 14));
            _ta_ema_12 = ta::EMA(get_input_int("MACD fast", 12));
            _ta_ema_13 = ta::EMA(get_input_int("MACD slow", 26));
            _ta_ema_14 = ta::EMA(get_input_int("MACD signal", 9));
            _ta_ema_15 = ta::EMA(get_input_int("Pressure period", 14));
            _ta_ema_16 = ta::EMA(get_input_int("Pressure smoothing", 5));
            _ta_initialized_ = true;
        }
        auto [adx_v, p_di_v, m_di_v] = f_adx_chain_cs0(i_adx_len);
        atr_v = (history_advances_new_bar() ? _ta_atr_6.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_6.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        atr_avg = (history_advances_new_bar() ? _ta_sma_7.compute(atr_v) : _ta_sma_7.recompute(atr_v));
        vol_ratio = ((([&]{ auto _pna_l = (atr_avg); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (((double)(atr_v) / (double)(atr_avg))) : (1.0));
        regime = 0;
        if ((([&]{ auto _pna_l = (vol_ratio); auto _pna_r = (i_vol_exp); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }()) && ([&]{ auto _pna_l = (adx_v); auto _pna_r = (i_adx_trend); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()))) {
            regime = 3;
        } else
        if (([&]{ auto _pna_l = (adx_v); auto _pna_r = (i_adx_trend); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }())) {
            regime = 1;
        } else
        if (([&]{ auto _pna_l = (vol_ratio); auto _pna_r = (i_vol_con); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) || _pfc_eq); }())) {
            regime = 2;
        }
        trending = ([&]{ auto _pna_l = (regime); auto _pna_r = (1); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l == _pna_r); }());
        di_bull = ([&]{ auto _pna_l = (p_di_v); auto _pna_r = (m_di_v); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }());
        di_bear = ([&]{ auto _pna_l = (m_di_v); auto _pna_r = (p_di_v); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }());
        ma_fast = (history_advances_new_bar() ? _ta_ema_8.compute(current_bar_.close) : _ta_ema_8.recompute(current_bar_.close));
        ma_slow = (history_advances_new_bar() ? _ta_ema_9.compute(current_bar_.close) : _ta_ema_9.recompute(current_bar_.close));
        ma_trend = (history_advances_new_bar() ? _ta_ema_10.compute(current_bar_.close) : _ta_ema_10.recompute(current_bar_.close));
        stack_up = (([&]{ auto _pna_l = (ma_fast); auto _pna_r = (ma_slow); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (ma_slow); auto _pna_r = (ma_trend); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()));
        stack_dn = (([&]{ auto _pna_l = (ma_fast); auto _pna_r = (ma_slow); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (ma_slow); auto _pna_r = (ma_trend); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        above_str = (([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (ma_fast); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (ma_slow); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()));
        below_str = (([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (ma_fast); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (ma_slow); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        bull_bias = 0.0;
        if (stack_up) {
            bull_bias += 30;
        }
        if (above_str) {
            bull_bias += 20;
        }
        if (([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (ma_trend); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) {
            bull_bias += 20;
        }
        bear_bias = 0.0;
        if (stack_dn) {
            bear_bias += 30;
        }
        if (below_str) {
            bear_bias += 20;
        }
        if (([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (ma_trend); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }())) {
            bear_bias += 20;
        }
        bias_bull = ([&]{ auto _pna_l = (bull_bias); auto _pna_r = (i_bias_thresh); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }());
        bias_bear = ([&]{ auto _pna_l = (bear_bias); auto _pna_r = (i_bias_thresh); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }());
        if (history_advances_new_bar()) rsi_v.push((history_advances_new_bar() ? _ta_rsi_11.compute(current_bar_.close) : _ta_rsi_11.recompute(current_bar_.close)));
        else rsi_v.update((history_advances_new_bar() ? _ta_rsi_11.compute(current_bar_.close) : _ta_rsi_11.recompute(current_bar_.close)));
        rsi_bull = ([&]{ auto _pna_l = (rsi_v[0]); auto _pna_r = (i_rsi_bull); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }());
        rsi_bear = ([&]{ auto _pna_l = (rsi_v[0]); auto _pna_r = (i_rsi_bear); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }());
        rsi_mom_up = ([&]{ auto _pna_l = (rsi_v[0]); auto _pna_r = (rsi_v[3]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }());
        rsi_mom_dn = ([&]{ auto _pna_l = (rsi_v[0]); auto _pna_r = (rsi_v[3]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }());
        macd_line = ((_use_precalc ? _precalc__ta_ema_12[bar_index_] : (history_advances_new_bar() ? _ta_ema_12.compute(current_bar_.close) : _ta_ema_12.recompute(current_bar_.close))) - (_use_precalc ? _precalc__ta_ema_13[bar_index_] : (history_advances_new_bar() ? _ta_ema_13.compute(current_bar_.close) : _ta_ema_13.recompute(current_bar_.close))));
        macd_sig = (history_advances_new_bar() ? _ta_ema_14.compute(macd_line) : _ta_ema_14.recompute(macd_line));
        if (history_advances_new_bar()) macd_hist.push((macd_line - macd_sig));
        else macd_hist.update((macd_line - macd_sig));
        macd_bull = (([&]{ auto _pna_l = (macd_hist[0]); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (macd_hist[0]); auto _pna_r = (macd_hist[1]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()));
        macd_bear = (([&]{ auto _pna_l = (macd_hist[0]); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (macd_hist[0]); auto _pna_r = (macd_hist[1]); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        mom_bull = 0;
        if (rsi_bull) {
            mom_bull += 1;
        }
        if (rsi_mom_up) {
            mom_bull += 1;
        }
        if (macd_bull) {
            mom_bull += 1;
        }
        mom_bear = 0;
        if (rsi_bear) {
            mom_bear += 1;
        }
        if (rsi_mom_dn) {
            mom_bear += 1;
        }
        if (macd_bear) {
            mom_bear += 1;
        }
        mom_bull_ok = ([&]{ auto _pna_l = (mom_bull); auto _pna_r = (2); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l >= _pna_r); }());
        mom_bear_ok = ([&]{ auto _pna_l = (mom_bear); auto _pna_r = (2); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l >= _pna_r); }());
        bar_range = (current_bar_.high - current_bar_.low);
        raw_press = ((([&]{ auto _pna_l = (bar_range); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) ? (((double)((current_bar_.close - current_bar_.low)) / (double)(bar_range))) : (0.5));
        press_r = (history_advances_new_bar() ? _ta_ema_15.compute(raw_press) : _ta_ema_15.recompute(raw_press));
        if (history_advances_new_bar()) press_s.push((history_advances_new_bar() ? _ta_ema_16.compute(press_r) : _ta_ema_16.recompute(press_r)));
        else press_s.update((history_advances_new_bar() ? _ta_ema_16.compute(press_r) : _ta_ema_16.recompute(press_r)));
        press_mom = (press_s[0] - press_s[i_press_mom]);
        press_bull = (([&]{ auto _pna_l = (press_s[0]); auto _pna_r = ((0.5 + i_press_thr)); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) || ([&]{ auto _pna_l = (press_mom); auto _pna_r = (i_press_thr); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()));
        press_bear = (([&]{ auto _pna_l = (press_s[0]); auto _pna_r = ((0.5 - i_press_thr)); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()) || ([&]{ auto _pna_l = (press_mom); auto _pna_r = ((-i_press_thr)); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        bull_total = 0;
        if ((trending && di_bull)) {
            bull_total += 2;
        }
        if (bias_bull) {
            bull_total += 1;
        }
        if (mom_bull_ok) {
            bull_total += 1;
        }
        if (press_bull) {
            bull_total += 1;
        }
        bear_total = 0;
        if ((trending && di_bear)) {
            bear_total += 2;
        }
        if (bias_bear) {
            bear_total += 1;
        }
        if (mom_bear_ok) {
            bear_total += 1;
        }
        if (press_bear) {
            bear_total += 1;
        }
        bars_since_trade = (bars_since_trade + 1);
        cooldown_ok = ([&]{ auto _pna_l = (bars_since_trade); auto _pna_r = (i_cooldown); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l >= _pna_r); }());
        long_entry = (((([&]{ auto _pna_l = (bull_total); auto _pna_r = (5); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l >= _pna_r); }()) && trending) && cooldown_ok) && ([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) || _pfc_eq); }()));
        short_entry = (((([&]{ auto _pna_l = (bear_total); auto _pna_r = (5); return !is_na(_pna_l) && !is_na(_pna_r) && (_pna_l >= _pna_r); }()) && trending) && cooldown_ok) && ([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) || _pfc_eq); }()));
        if (long_entry) {
            if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }())) {
                strategy_close(std::string("S"), std::string("flip"), na<double>(), na<double>(), false);
            }
            strategy_entry(std::string("L"), true, na<double>(), na<double>(), na<double>(), std::string("composite long"));
            bars_since_trade = 0;
        }
        if (short_entry) {
            if (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }())) {
                strategy_close(std::string("L"), std::string("flip"), na<double>(), na<double>(), false);
            }
            strategy_entry(std::string("S"), false, na<double>(), na<double>(), na<double>(), std::string("composite short"));
            bars_since_trade = 0;
        }
        if ((!(trending) && ([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (!_pfc_eq); }()))) {
            strategy_close("", std::string("regime exit"), na<double>(), na<double>(), false);
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_atr_6.resize(n);
        _precalc__ta_ema_8.resize(n);
        _precalc__ta_ema_9.resize(n);
        _precalc__ta_ema_10.resize(n);
        _precalc__ta_rsi_11.resize(n);
        _precalc__ta_ema_12.resize(n);
        _precalc__ta_ema_13.resize(n);

        _ta_atr_6 = ta::ATR(14);
        _ta_ema_8 = ta::EMA(21);
        _ta_ema_9 = ta::EMA(55);
        _ta_ema_10 = ta::EMA(200);
        _ta_rsi_11 = ta::RSI(14);
        _ta_ema_12 = ta::EMA(12);
        _ta_ema_13 = ta::EMA(26);

        _s_high.clear();
        _s_low.clear();

        for (int i = 0; i < n; ++i) {
            _s_high.push(bars[i].high);
            _s_low.push(bars[i].low);
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
            _precalc__ta_atr_6[i] = _ta_atr_6.compute(bars[i].high, bars[i].low, bars[i].close);
            _precalc__ta_ema_8[i] = _ta_ema_8.compute(bars[i].close);
            _precalc__ta_ema_9[i] = _ta_ema_9.compute(bars[i].close);
            _precalc__ta_ema_10[i] = _ta_ema_10.compute(bars[i].close);
            _precalc__ta_rsi_11[i] = _ta_rsi_11.compute(bars[i].close);
            _precalc__ta_ema_12[i] = _ta_ema_12.compute(bars[i].close);
            _precalc__ta_ema_13[i] = _ta_ema_13.compute(bars[i].close);
        }

        _ta_atr_6 = ta::ATR(14);
        _ta_ema_8 = ta::EMA(21);
        _ta_ema_9 = ta::EMA(55);
        _ta_ema_10 = ta::EMA(200);
        _ta_rsi_11 = ta::RSI(14);
        _ta_ema_12 = ta::EMA(12);
        _ta_ema_13 = ta::EMA(26);
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
