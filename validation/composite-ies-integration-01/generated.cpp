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
    std::vector<double> _precalc__ta_ema_15;
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
        double tr_v = (is_first_tick_ ? _ta_tr_1.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_tr_1.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        double p_dm = std::max((double)((current_bar_.high - _s_high[1])), (double)(0));
        double m_dm = std::max((double)((_s_low[1] - current_bar_.low)), (double)(0));
        if ((p_dm > m_dm)) {
            m_dm = 0;
        } else {
            p_dm = 0;
        }
        double s_tr = (is_first_tick_ ? _ta_rma_2.compute(tr_v) : _ta_rma_2.recompute(tr_v));
        double s_p = (is_first_tick_ ? _ta_rma_3.compute(p_dm) : _ta_rma_3.recompute(p_dm));
        double s_m = (is_first_tick_ ? _ta_rma_4.compute(m_dm) : _ta_rma_4.recompute(m_dm));
        double p_di_v = (((s_tr > 0)) ? (((double)((100 * s_p)) / (double)(s_tr))) : (0));
        double m_di_v = (((s_tr > 0)) ? (((double)((100 * s_m)) / (double)(s_tr))) : (0));
        double di_sum = (p_di_v + m_di_v);
        double dx_v = (((di_sum > 0)) ? (((double)((100 * std::abs((p_di_v - m_di_v)))) / (double)(di_sum))) : (0));
        double adx_v = (is_first_tick_ ? _ta_rma_5.compute(dx_v) : _ta_rma_5.recompute(dx_v));
        return std::make_tuple(adx_v, p_di_v, m_di_v);
    }

    void on_bar(const Bar& bar) override {
        if (is_first_tick_) _s_high.push(current_bar_.high);
        else _s_high.update(current_bar_.high);
        if (is_first_tick_) _s_low.push(current_bar_.low);
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
            _ta_sma_7 = ta::SMA(get_input_int("ATR period", 14) * 3);
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
        atr_v = (is_first_tick_ ? _ta_atr_6.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_6.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        atr_avg = (is_first_tick_ ? _ta_sma_7.compute(atr_v) : _ta_sma_7.recompute(atr_v));
        vol_ratio = (((atr_avg > 0)) ? (((double)(atr_v) / (double)(atr_avg))) : (1.0));
        regime = 0;
        if (((vol_ratio >= i_vol_exp) && (adx_v < i_adx_trend))) {
            regime = 3;
        } else
        if ((adx_v >= i_adx_trend)) {
            regime = 1;
        } else
        if ((vol_ratio <= i_vol_con)) {
            regime = 2;
        }
        trending = (regime == 1);
        di_bull = (p_di_v > m_di_v);
        di_bear = (m_di_v > p_di_v);
        ma_fast = (is_first_tick_ ? _ta_ema_8.compute(current_bar_.close) : _ta_ema_8.recompute(current_bar_.close));
        ma_slow = (is_first_tick_ ? _ta_ema_9.compute(current_bar_.close) : _ta_ema_9.recompute(current_bar_.close));
        ma_trend = (is_first_tick_ ? _ta_ema_10.compute(current_bar_.close) : _ta_ema_10.recompute(current_bar_.close));
        stack_up = ((ma_fast > ma_slow) && (ma_slow > ma_trend));
        stack_dn = ((ma_fast < ma_slow) && (ma_slow < ma_trend));
        above_str = ((current_bar_.close > ma_fast) && (current_bar_.close > ma_slow));
        below_str = ((current_bar_.close < ma_fast) && (current_bar_.close < ma_slow));
        bull_bias = 0.0;
        if (stack_up) {
            bull_bias += 30;
        }
        if (above_str) {
            bull_bias += 20;
        }
        if ((current_bar_.close > ma_trend)) {
            bull_bias += 20;
        }
        bear_bias = 0.0;
        if (stack_dn) {
            bear_bias += 30;
        }
        if (below_str) {
            bear_bias += 20;
        }
        if ((current_bar_.close < ma_trend)) {
            bear_bias += 20;
        }
        bias_bull = (bull_bias >= i_bias_thresh);
        bias_bear = (bear_bias >= i_bias_thresh);
        rsi_v.push((is_first_tick_ ? _ta_rsi_11.compute(current_bar_.close) : _ta_rsi_11.recompute(current_bar_.close)));
        rsi_bull = (rsi_v[0] > i_rsi_bull);
        rsi_bear = (rsi_v[0] < i_rsi_bear);
        rsi_mom_up = (rsi_v[0] > rsi_v[3]);
        rsi_mom_dn = (rsi_v[0] < rsi_v[3]);
        macd_line = ((_use_precalc ? _precalc__ta_ema_12[bar_index_] : (is_first_tick_ ? _ta_ema_12.compute(current_bar_.close) : _ta_ema_12.recompute(current_bar_.close))) - (_use_precalc ? _precalc__ta_ema_13[bar_index_] : (is_first_tick_ ? _ta_ema_13.compute(current_bar_.close) : _ta_ema_13.recompute(current_bar_.close))));
        macd_sig = (is_first_tick_ ? _ta_ema_14.compute(macd_line) : _ta_ema_14.recompute(macd_line));
        macd_hist.push((macd_line - macd_sig));
        macd_bull = ((macd_hist[0] > 0) && (macd_hist[0] > macd_hist[1]));
        macd_bear = ((macd_hist[0] < 0) && (macd_hist[0] < macd_hist[1]));
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
        mom_bull_ok = (mom_bull >= 2);
        mom_bear_ok = (mom_bear >= 2);
        bar_range = (current_bar_.high - current_bar_.low);
        raw_press = (((bar_range > 0)) ? (((double)((current_bar_.close - current_bar_.low)) / (double)(bar_range))) : (0.5));
        press_r = (is_first_tick_ ? _ta_ema_15.compute(raw_press) : _ta_ema_15.recompute(raw_press));
        press_s.push((is_first_tick_ ? _ta_ema_16.compute(press_r) : _ta_ema_16.recompute(press_r)));
        press_mom = (press_s[0] - press_s[i_press_mom]);
        press_bull = ((press_s[0] > (0.5 + i_press_thr)) || (press_mom > i_press_thr));
        press_bear = ((press_s[0] < (0.5 - i_press_thr)) || (press_mom < (-i_press_thr)));
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
        cooldown_ok = (bars_since_trade >= i_cooldown);
        long_entry = ((((bull_total >= 5) && trending) && cooldown_ok) && (signed_position_size() <= 0));
        short_entry = ((((bear_total >= 5) && trending) && cooldown_ok) && (signed_position_size() >= 0));
        if (long_entry) {
            if ((signed_position_size() < 0)) {
                strategy_close(std::string("S"), std::string("flip"), na<double>(), na<double>(), false);
            }
            strategy_entry(std::string("L"), true, na<double>(), na<double>(), na<double>(), std::string("composite long"));
            bars_since_trade = 0;
        }
        if (short_entry) {
            if ((signed_position_size() > 0)) {
                strategy_close(std::string("L"), std::string("flip"), na<double>(), na<double>(), false);
            }
            strategy_entry(std::string("S"), false, na<double>(), na<double>(), na<double>(), std::string("composite short"));
            bars_since_trade = 0;
        }
        if ((!(trending) && (signed_position_size() != 0))) {
            strategy_close_all();
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
        _precalc__ta_ema_15.resize(n);

        _ta_atr_6 = ta::ATR(14);
        _ta_ema_8 = ta::EMA(21);
        _ta_ema_9 = ta::EMA(55);
        _ta_ema_10 = ta::EMA(200);
        _ta_rsi_11 = ta::RSI(14);
        _ta_ema_12 = ta::EMA(12);
        _ta_ema_13 = ta::EMA(26);
        _ta_ema_15 = ta::EMA(14);

        _s_high.clear();
        _s_low.clear();

        for (int i = 0; i < n; ++i) {
            _s_high.push(bars[i].high);
            _s_low.push(bars[i].low);
            _precalc__ta_atr_6[i] = _ta_atr_6.compute(bars[i].high, bars[i].low, bars[i].close);
            _precalc__ta_ema_8[i] = _ta_ema_8.compute(bars[i].close);
            _precalc__ta_ema_9[i] = _ta_ema_9.compute(bars[i].close);
            _precalc__ta_ema_10[i] = _ta_ema_10.compute(bars[i].close);
            _precalc__ta_rsi_11[i] = _ta_rsi_11.compute(bars[i].close);
            _precalc__ta_ema_12[i] = _ta_ema_12.compute(bars[i].close);
            _precalc__ta_ema_13[i] = _ta_ema_13.compute(bars[i].close);
            _precalc__ta_ema_15[i] = _ta_ema_15.compute(raw_press);
        }

        _ta_atr_6 = ta::ATR(14);
        _ta_ema_8 = ta::EMA(21);
        _ta_ema_9 = ta::EMA(55);
        _ta_ema_10 = ta::EMA(200);
        _ta_rsi_11 = ta::RSI(14);
        _ta_ema_12 = ta::EMA(12);
        _ta_ema_13 = ta::EMA(26);
        _ta_ema_15 = ta::EMA(14);
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
