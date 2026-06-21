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
    ta::ATR _ta_atr_1;
    std::vector<double> _precalc__ta_atr_1;
    ta::PivotHigh _ta_pivothigh_2;
    std::vector<double> _precalc__ta_pivothigh_2;
    ta::PivotLow _ta_pivotlow_3;
    std::vector<double> _precalc__ta_pivotlow_3;
    ta::RSI _ta_rsi_4;
    std::vector<double> _precalc__ta_rsi_4;
    ta::EMA _ta_ema_5;
    ta::SMA _ta_sma_6;
    std::vector<double> _precalc__ta_sma_6;
    ta::StdDev _ta_stdev_7;
    std::vector<double> _precalc__ta_stdev_7;
    math::Sum _ta_sum_8;
    std::vector<double> _precalc__ta_sum_8;
    ta::Change _ta_change_9;
    std::vector<double> _precalc__ta_change_9;
    ta::Change _ta_change_10;
    std::vector<double> _precalc__ta_change_10;
    ta::RMA _ta_rma_11;
    ta::RMA _ta_rma_12;
    ta::RMA _ta_rma_13;
    ta::RMA _ta_rma_14;
    bool _use_precalc = false;
    Series<double> _s_close;
    Series<double> _s_high;
    Series<double> _s_low;
    Series<double> _s_open;
    double last_ph;
    double last_pl;
    std::vector<double> z_top;
    std::vector<double> z_bot;
    std::vector<bool> z_isb;
    Series<bool> in_session;
    Series<double> rsi_smooth;
    int i_pivot = 0;
    double i_fvg_atr = 0.0;
    int i_rsi_len = 0;
    int i_vol_ma_len = 0;
    double i_vol_z = 0.0;
    int i_cd_sum = 0;
    int i_adx_len = 0;
    double i_adx_thr = 0.0;
    std::string i_session = std::string("");
    std::string i_tz = std::string("");
    double atr_v = 0.0;
    double ph_v = 0.0;
    double pl_v = 0.0;
    bool pivot_break_up = false;
    bool pivot_break_dn = false;
    bool bull_fvg_event = false;
    bool bear_fvg_event = false;
    double fvg_min_w = 0.0;
    bool in_bull_fvg = false;
    bool in_bear_fvg = false;
    double rsi_v = 0.0;
    bool rsi_div_bull = false;
    bool rsi_div_bear = false;
    double vol_ma = 0.0;
    double vol_std = 0.0;
    double vol_z = 0.0;
    bool vol_anom_bull = false;
    bool vol_anom_bear = false;
    double buy_vol = 0.0;
    double sell_vol = 0.0;
    double vol_d = 0.0;
    double cum_d = 0.0;
    bool cd_up = false;
    bool cd_dn = false;
    double up_mv = 0.0;
    double dn_mv = 0.0;
    double p_dm_v = 0.0;
    double m_dm_v = 0.0;
    double tr_smo = 0.0;
    double p_di_v = 0.0;
    double m_di_v = 0.0;
    double dx_v = 0.0;
    double adx_v = 0.0;
    bool trending_bull = false;
    bool trending_bear = false;
    bool long_setup = false;
    bool short_setup = false;
    bool _var_initialized = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    explicit GeneratedStrategy() : _ta_atr_1(14), _ta_pivothigh_2(5, 5), _ta_pivotlow_3(5, 5), _ta_rsi_4(14), _ta_ema_5(3), _ta_sma_6(20), _ta_stdev_7(20), _ta_sum_8(10), _ta_rma_11(14), _ta_rma_12(14), _ta_rma_13(14), _ta_rma_14(14), last_ph(na<double>()), last_pl(na<double>()) {
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

    void on_bar(const Bar& bar) override {
        if (is_first_tick_) _s_close.push(current_bar_.close);
        else _s_close.update(current_bar_.close);
        if (is_first_tick_) _s_high.push(current_bar_.high);
        else _s_high.update(current_bar_.high);
        if (is_first_tick_) _s_low.push(current_bar_.low);
        else _s_low.update(current_bar_.low);
        if (is_first_tick_) _s_open.push(current_bar_.open);
        else _s_open.update(current_bar_.open);
        if (!_var_initialized) {
            z_top = std::vector<double>();
            z_bot = std::vector<double>();
            z_isb = std::vector<bool>();
            _var_initialized = true;
        } else {
        }
        if (!_inputs_initialized_) {
            i_pivot = get_input_int("Pivot strength", 5);
            i_fvg_atr = get_input_double("FVG min size (atr fraction)", 0.3);
            i_rsi_len = get_input_int("RSI length", 14);
            i_vol_ma_len = get_input_int("Volume MA length", 20);
            i_vol_z = get_input_double("Volume z threshold", 2.0);
            i_cd_sum = get_input_int("Cum-delta window", 10);
            i_adx_len = get_input_int("ADX length", 14);
            i_adx_thr = get_input_double("ADX trend threshold", 25);
            i_session = get_input_string("Active session", std::string("0800-1600"));
            i_tz = get_input_string("Timezone", std::string("America/New_York"));
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_pivothigh_2 = ta::PivotHigh(get_input_int("Pivot strength", 5), get_input_int("Pivot strength", 5));
            _ta_pivotlow_3 = ta::PivotLow(get_input_int("Pivot strength", 5), get_input_int("Pivot strength", 5));
            _ta_rsi_4 = ta::RSI(get_input_int("RSI length", 14));
            _ta_sma_6 = ta::SMA(get_input_int("Volume MA length", 20));
            _ta_sum_8 = math::Sum(get_input_int("Cum-delta window", 10));
            _ta_rma_11 = ta::RMA(get_input_int("ADX length", 14));
            _ta_rma_12 = ta::RMA(get_input_int("ADX length", 14));
            _ta_rma_13 = ta::RMA(get_input_int("ADX length", 14));
            _ta_rma_14 = ta::RMA(get_input_int("ADX length", 14));
            _ta_initialized_ = true;
        }
        atr_v = (is_first_tick_ ? _ta_atr_1.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_1.recompute(current_bar_.high, current_bar_.low, current_bar_.close));
        ph_v = (is_first_tick_ ? _ta_pivothigh_2.compute(current_bar_.high) : _ta_pivothigh_2.recompute(current_bar_.high));
        pl_v = (is_first_tick_ ? _ta_pivotlow_3.compute(current_bar_.low) : _ta_pivotlow_3.recompute(current_bar_.low));
        if (!(is_na(ph_v))) {
            last_ph = ph_v;
        }
        if (!(is_na(pl_v))) {
            last_pl = pl_v;
        }
        pivot_break_up = (!(is_na(last_ph)) && (current_bar_.close > last_ph));
        pivot_break_dn = (!(is_na(last_pl)) && (current_bar_.close < last_pl));
        bull_fvg_event = ((current_bar_.low > _s_high[2]) && (_s_close[1] > _s_open[1]));
        bear_fvg_event = ((current_bar_.high < _s_low[2]) && (_s_close[1] < _s_open[1]));
        fvg_min_w = (atr_v * i_fvg_atr);
        if ((bull_fvg_event && ((current_bar_.low - _s_high[2]) >= fvg_min_w))) {
            z_top.push_back(current_bar_.low);
            z_bot.push_back(_s_high[2]);
            z_isb.push_back(true);
        }
        if ((bear_fvg_event && ((_s_low[2] - current_bar_.high) >= fvg_min_w))) {
            z_top.push_back(_s_low[2]);
            z_bot.push_back(current_bar_.high);
            z_isb.push_back(false);
        }
        while (((double)z_top.size() > 30)) {
            [&](){ auto v=z_top.front(); z_top.erase(z_top.begin()); return v; }();
            [&](){ auto v=z_bot.front(); z_bot.erase(z_bot.begin()); return v; }();
            [&](){ auto v=z_isb.front(); z_isb.erase(z_isb.begin()); return v; }();
        }
        in_bull_fvg = false;
        in_bear_fvg = false;
        if (((double)z_top.size() > 0)) {
            for (int k = 0; k <= ((double)z_top.size() - 1); k += 1) {
                double t = z_top[k];
                double b = z_bot[k];
                bool s = z_isb[k];
                if (((current_bar_.low <= t) && (current_bar_.high >= b))) {
                    if (s) {
                        in_bull_fvg = true;
                    } else {
                        in_bear_fvg = true;
                    }
                }
            }
        }
        rsi_v = (is_first_tick_ ? _ta_rsi_4.compute(current_bar_.close) : _ta_rsi_4.recompute(current_bar_.close));
        rsi_smooth.push((is_first_tick_ ? _ta_ema_5.compute(rsi_v) : _ta_ema_5.recompute(rsi_v)));
        rsi_div_bull = (((rsi_smooth[0] < 40) && (current_bar_.close > _s_close[5])) && (rsi_smooth[0] > rsi_smooth[5]));
        rsi_div_bear = (((rsi_smooth[0] > 60) && (current_bar_.close < _s_close[5])) && (rsi_smooth[0] < rsi_smooth[5]));
        vol_ma = (is_first_tick_ ? _ta_sma_6.compute(current_bar_.volume) : _ta_sma_6.recompute(current_bar_.volume));
        vol_std = (is_first_tick_ ? _ta_stdev_7.compute(current_bar_.volume) : _ta_stdev_7.recompute(current_bar_.volume));
        vol_z = (((vol_std > 0)) ? (((double)((current_bar_.volume - vol_ma)) / (double)(vol_std))) : (0));
        vol_anom_bull = ((std::abs(vol_z) > i_vol_z) && (current_bar_.close > current_bar_.open));
        vol_anom_bear = ((std::abs(vol_z) > i_vol_z) && (current_bar_.close < current_bar_.open));
        buy_vol = (((current_bar_.close > current_bar_.open)) ? (current_bar_.volume) : (((double)((current_bar_.volume * (current_bar_.close - current_bar_.low))) / (double)(((current_bar_.high - current_bar_.low) + 0.0001)))));
        sell_vol = (((current_bar_.close < current_bar_.open)) ? (current_bar_.volume) : (((double)((current_bar_.volume * (current_bar_.high - current_bar_.close))) / (double)(((current_bar_.high - current_bar_.low) + 0.0001)))));
        vol_d = (buy_vol - sell_vol);
        cum_d = (is_first_tick_ ? _ta_sum_8.compute(vol_d) : _ta_sum_8.recompute(vol_d));
        cd_up = (cum_d > 0);
        cd_dn = (cum_d < 0);
        up_mv = (is_first_tick_ ? _ta_change_9.compute(current_bar_.high) : _ta_change_9.recompute(current_bar_.high));
        dn_mv = (-(_use_precalc ? _precalc__ta_change_10[bar_index_] : (is_first_tick_ ? _ta_change_10.compute(current_bar_.low) : _ta_change_10.recompute(current_bar_.low))));
        p_dm_v = ((is_na(up_mv)) ? (na<double>()) : (((((up_mv > dn_mv) && (up_mv > 0))) ? (up_mv) : (0))));
        m_dm_v = ((is_na(dn_mv)) ? (na<double>()) : (((((dn_mv > up_mv) && (dn_mv > 0))) ? (dn_mv) : (0))));
        tr_smo = (is_first_tick_ ? _ta_rma_11.compute((std::isnan(_s_close[1]) ? (current_bar_.high - current_bar_.low) : std::max(current_bar_.high - current_bar_.low, std::max(std::abs(current_bar_.high - _s_close[1]), std::abs(current_bar_.low - _s_close[1]))))) : _ta_rma_11.recompute((std::isnan(_s_close[1]) ? (current_bar_.high - current_bar_.low) : std::max(current_bar_.high - current_bar_.low, std::max(std::abs(current_bar_.high - _s_close[1]), std::abs(current_bar_.low - _s_close[1]))))));
        p_di_v = (((tr_smo > 0)) ? (((double)((100 * (is_first_tick_ ? _ta_rma_12.compute(p_dm_v) : _ta_rma_12.recompute(p_dm_v)))) / (double)(tr_smo))) : (0));
        m_di_v = (((tr_smo > 0)) ? (((double)((100 * (is_first_tick_ ? _ta_rma_13.compute(m_dm_v) : _ta_rma_13.recompute(m_dm_v)))) / (double)(tr_smo))) : (0));
        dx_v = ((((p_di_v + m_di_v) > 0)) ? (((double)((100 * std::abs((p_di_v - m_di_v)))) / (double)((p_di_v + m_di_v)))) : (0));
        adx_v = (is_first_tick_ ? _ta_rma_14.compute(dx_v) : _ta_rma_14.recompute(dx_v));
        trending_bull = ((adx_v > i_adx_thr) && (p_di_v > m_di_v));
        trending_bear = ((adx_v > i_adx_thr) && (m_di_v > p_di_v));
        in_session.push(!(is_na(pine_time(current_bar_.timestamp, script_tf_, i_session, i_tz, script_tf_))));
        long_setup = ((((pivot_break_up && in_bull_fvg) && (vol_anom_bull || cd_up)) && trending_bull) && in_session[0]);
        short_setup = ((((pivot_break_dn && in_bear_fvg) && (vol_anom_bear || cd_dn)) && trending_bear) && in_session[0]);
        if ((long_setup && (signed_position_size() <= 0))) {
            if ((signed_position_size() < 0)) {
                strategy_close(std::string("S"), std::string("flip flat"), na<double>(), na<double>(), false);
            }
            strategy_entry(std::string("L"), true, na<double>(), na<double>(), na<double>(), std::string("vcp confluence long"));
        }
        if ((short_setup && (signed_position_size() >= 0))) {
            if ((signed_position_size() > 0)) {
                strategy_close(std::string("L"), std::string("flip flat"), na<double>(), na<double>(), false);
            }
            strategy_entry(std::string("S"), false, na<double>(), na<double>(), na<double>(), std::string("vcp confluence short"));
        }
        if (((!(in_session[0]) && in_session[1]) && (signed_position_size() != 0))) {
            strategy_close_all();
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_atr_1.resize(n);
        _precalc__ta_pivothigh_2.resize(n);
        _precalc__ta_pivotlow_3.resize(n);
        _precalc__ta_rsi_4.resize(n);
        _precalc__ta_sma_6.resize(n);
        _precalc__ta_stdev_7.resize(n);
        _precalc__ta_sum_8.resize(n);
        _precalc__ta_change_9.resize(n);
        _precalc__ta_change_10.resize(n);

        _ta_atr_1 = ta::ATR(14);
        _ta_pivothigh_2 = ta::PivotHigh(5, 5);
        _ta_pivotlow_3 = ta::PivotLow(5, 5);
        _ta_rsi_4 = ta::RSI(14);
        _ta_sma_6 = ta::SMA(20);
        _ta_stdev_7 = ta::StdDev(20);
        _ta_sum_8 = math::Sum(10);
        _ta_change_9 = ta::Change();
        _ta_change_10 = ta::Change();

        _s_close.clear();
        _s_high.clear();
        _s_low.clear();
        _s_open.clear();

        for (int i = 0; i < n; ++i) {
            _s_close.push(bars[i].close);
            _s_high.push(bars[i].high);
            _s_low.push(bars[i].low);
            _s_open.push(bars[i].open);
            _precalc__ta_atr_1[i] = _ta_atr_1.compute(bars[i].high, bars[i].low, bars[i].close);
            _precalc__ta_pivothigh_2[i] = _ta_pivothigh_2.compute(bars[i].high);
            _precalc__ta_pivotlow_3[i] = _ta_pivotlow_3.compute(bars[i].low);
            _precalc__ta_rsi_4[i] = _ta_rsi_4.compute(bars[i].close);
            _precalc__ta_sma_6[i] = _ta_sma_6.compute(bars[i].volume);
            _precalc__ta_stdev_7[i] = _ta_stdev_7.compute(bars[i].volume);
            _precalc__ta_sum_8[i] = _ta_sum_8.compute(vol_d);
            _precalc__ta_change_9[i] = _ta_change_9.compute(bars[i].high);
            _precalc__ta_change_10[i] = _ta_change_10.compute(bars[i].low);
        }

        _ta_atr_1 = ta::ATR(14);
        _ta_pivothigh_2 = ta::PivotHigh(5, 5);
        _ta_pivotlow_3 = ta::PivotLow(5, 5);
        _ta_rsi_4 = ta::RSI(14);
        _ta_sma_6 = ta::SMA(20);
        _ta_stdev_7 = ta::StdDev(20);
        _ta_sum_8 = math::Sum(10);
        _ta_change_9 = ta::Change();
        _ta_change_10 = ta::Change();
        _s_close.clear();
        _s_high.clear();
        _s_low.clear();
        _s_open.clear();

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
