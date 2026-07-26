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
#include <deque>
#include <functional>
#include <limits>
#include <tuple>
#include <optional>
#include <type_traits>
#include <stdexcept>
#include <utility>
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

struct SwitchGauge {
    int32_t __pf_id = -1;
};
inline bool is_na(const SwitchGauge& _z) { return _z.__pf_id < 0; }

template <typename _PFValue>
struct _PFCheckpointTraits;

class _PFUdtUndoCoordinator {
    std::vector<std::function<void()>> _pf_undo_;
    uint64_t _pf_generation_ = 0;
    bool _pf_active_ = false;
public:
    struct Snapshot { uint64_t generation; };
    _PFUdtUndoCoordinator() = default;
    _PFUdtUndoCoordinator(
        const _PFUdtUndoCoordinator&) = delete;
    _PFUdtUndoCoordinator& operator=(
        const _PFUdtUndoCoordinator&) = delete;
    _PFUdtUndoCoordinator(
        _PFUdtUndoCoordinator&&) = delete;
    _PFUdtUndoCoordinator& operator=(
        _PFUdtUndoCoordinator&&) = delete;

    Snapshot snapshot() {
        if (_pf_generation_ == std::numeric_limits<uint64_t>::max()) {
            throw std::overflow_error("UDT checkpoint generation exhausted");
        }
        ++_pf_generation_;
        _pf_undo_.clear();
        _pf_active_ = true;
        return Snapshot{_pf_generation_};
    }
    uint64_t generation() const { return _pf_generation_; }
    bool active() const { return _pf_active_; }
    bool empty() const { return _pf_undo_.empty(); }
    void record(uint64_t generation, std::function<void()> undo) {
        if (!_pf_active_ || generation != _pf_generation_) {
            throw std::runtime_error("invalid UDT undo generation");
        }
        _pf_undo_.push_back(std::move(undo));
    }
    void restore(const Snapshot& snapshot) {
        if (!_pf_active_ || snapshot.generation != _pf_generation_) {
            throw std::runtime_error("invalid UDT coordinator checkpoint token");
        }
        for (auto entry = _pf_undo_.rbegin();
                entry != _pf_undo_.rend(); ++entry) (*entry)();
        _pf_undo_.clear();
    }
};

template <typename _PFHandle, typename _PFRecord>
class _PFUdtArena {
    using _PFRecordTraits = _PFCheckpointTraits<_PFRecord>;
    using _PFRecordSnapshot = typename _PFRecordTraits::snapshot_type;
    struct _PFSlot {
        _PFRecord value;
        uint64_t logged_generation = 0;
    };
    std::deque<_PFSlot> _pf_records_;
    _PFUdtUndoCoordinator* _pf_coordinator_;
    std::size_t _pf_checkpoint_size_ = 0;
    uint64_t _pf_checkpoint_generation_ = 0;
    bool _pf_checkpoint_active_ = false;

    void capture(std::size_t index) {
        if (!_pf_checkpoint_active_) return;
        auto& slot = _pf_records_.at(index);
        if (slot.logged_generation
                == _pf_checkpoint_generation_) return;
        auto snapshot = _PFRecordTraits::take(slot.value);
        _pf_coordinator_->record(_pf_checkpoint_generation_,
            [this, index, snapshot = std::move(snapshot)]() mutable {
                auto& restore_slot = _pf_records_.at(index);
                _PFRecordTraits::restore(restore_slot.value, snapshot);
                restore_slot.logged_generation = 0;
            });
        slot.logged_generation = _pf_checkpoint_generation_;
    }
public:
    struct Snapshot {
        uint64_t generation;
        std::size_t size;
    };

    explicit _PFUdtArena(
            _PFUdtUndoCoordinator* coordinator)
            : _pf_coordinator_(coordinator) {
        if (!_pf_coordinator_)
            throw std::invalid_argument("UDT arena requires undo coordinator");
    }
    _PFUdtArena(
        const _PFUdtArena&) = delete;
    _PFUdtArena& operator=(
        const _PFUdtArena&) = delete;
    _PFUdtArena(
        _PFUdtArena&&) = delete;
    _PFUdtArena& operator=(
        _PFUdtArena&&) = delete;

    _PFHandle create(_PFRecord value) {
        if (_pf_records_.size() > static_cast<std::size_t>(
                std::numeric_limits<int32_t>::max())) {
            throw std::length_error("UDT object-ID capacity exceeded");
        }
        const auto id = static_cast<int32_t>(_pf_records_.size());
        _pf_records_.push_back(_PFSlot{std::move(value), 0});
        return _PFHandle{id};
    }
    _PFHandle copy(_PFHandle value) {
        return create(static_cast<const _PFUdtArena&>(*this).get(value));
    }
    _PFRecord& get(_PFHandle value) {
        if (value.__pf_id < 0
                || static_cast<std::size_t>(value.__pf_id) >= _pf_records_.size()) {
            throw std::runtime_error("UDT access on na or invalid object ID");
        }
        const auto index = static_cast<std::size_t>(value.__pf_id);
        capture(index);
        return _pf_records_[index].value;
    }
    const _PFRecord& get(_PFHandle value) const {
        if (value.__pf_id < 0
                || static_cast<std::size_t>(value.__pf_id) >= _pf_records_.size()) {
            throw std::runtime_error("UDT access on na or invalid object ID");
        }
        return _pf_records_[static_cast<std::size_t>(value.__pf_id)].value;
    }
    const _PFRecord& read(_PFHandle value) const {
        return get(value);
    }
    std::size_t size() const { return _pf_records_.size(); }
    _PFRecord& record_at(std::size_t index) {
        capture(index);
        return _pf_records_.at(index).value;
    }
    const _PFRecord& record_at(std::size_t index) const {
        return _pf_records_.at(index).value;
    }
    Snapshot snapshot() {
        if (!_pf_coordinator_->active()) {
            throw std::runtime_error("UDT coordinator checkpoint is not active");
        }
        _pf_checkpoint_generation_ = _pf_coordinator_->generation();
        _pf_checkpoint_size_ = _pf_records_.size();
        _pf_checkpoint_active_ = true;
        return Snapshot{_pf_checkpoint_generation_,
                        _pf_checkpoint_size_};
    }
    void restore(const Snapshot& snapshot) {
        if (!_pf_checkpoint_active_
                || snapshot.generation != _pf_checkpoint_generation_
                || snapshot.generation != _pf_coordinator_->generation()
                || snapshot.size != _pf_checkpoint_size_
                || _pf_records_.size() < snapshot.size
                || !_pf_coordinator_->empty()) {
            throw std::runtime_error("invalid UDT checkpoint token");
        }
        _pf_records_.resize(snapshot.size);
    }
};

struct _PFUdtRecord_SwitchGauge {
    double fast = 0.0;
    double slow = 0.0;
    double volatility = 0.0;
};

template <typename _PFValue>
struct _PFCheckpointTraits {
    using snapshot_type = _PFValue;
    static snapshot_type take(const _PFValue& value) { return value; }
    static void restore(_PFValue& value, const snapshot_type& snapshot) {
        value = snapshot;
    }
};

template <>
struct _PFCheckpointTraits<_PFUdtUndoCoordinator> {
    using coordinator_type = _PFUdtUndoCoordinator;
    using snapshot_type = typename coordinator_type::Snapshot;
    static snapshot_type take(coordinator_type& value) {
        return value.snapshot();
    }
    static void restore(coordinator_type& value,
                        const snapshot_type& snapshot) {
        value.restore(snapshot);
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

template <>
struct _PFCheckpointTraits<_PFUdtRecord_SwitchGauge> {
    struct snapshot_type {
        _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::fast)>::snapshot_type _pf_field_0;
        _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::slow)>::snapshot_type _pf_field_1;
        _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::volatility)>::snapshot_type _pf_field_2;
    };
    static snapshot_type take(const _PFUdtRecord_SwitchGauge& value) {
        return snapshot_type{
            _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::fast)>::take(value.fast),
            _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::slow)>::take(value.slow),
            _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::volatility)>::take(value.volatility),
        };
    }
    static void restore(_PFUdtRecord_SwitchGauge& value, const snapshot_type& snapshot) {
        _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::fast)>::restore(value.fast, snapshot._pf_field_0);
        _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::slow)>::restore(value.slow, snapshot._pf_field_1);
        _PFCheckpointTraits<decltype(_PFUdtRecord_SwitchGauge::volatility)>::restore(value.volatility, snapshot._pf_field_2);
    }
};

template <>
struct _PFCheckpointTraits<_PFUdtArena<SwitchGauge, _PFUdtRecord_SwitchGauge>> {
    using arena_type = _PFUdtArena<SwitchGauge, _PFUdtRecord_SwitchGauge>;
    using snapshot_type = typename arena_type::Snapshot;
    static snapshot_type take(arena_type& value) {
        return value.snapshot();
    }
    static void restore(arena_type& value, const snapshot_type& snapshot) {
        value.restore(snapshot);
    }
};

class GeneratedStrategy : public BacktestEngine {
public:
    _PFUdtUndoCoordinator _pf_udt_undo;
    _PFUdtArena<SwitchGauge, _PFUdtRecord_SwitchGauge> _pf_udt_SwitchGauge{&_pf_udt_undo};
    ta::EMA _ta_ema_1;
    std::vector<double> _precalc__ta_ema_1;
    ta::EMA _ta_ema_2;
    std::vector<double> _precalc__ta_ema_2;
    ta::ATR _ta_atr_3;
    std::vector<double> _precalc__ta_atr_3;
    ta::Crossover _ta_crossover_4;
    bool _use_precalc = false;
    std::string mode = std::string("");
    int fastLength = 0;
    int slowLength = 0;
    int atrLength = 0;
    double entryScore = 0.0;
    double maximumAtrLoss = 0.0;
    SwitchGauge gauge = SwitchGauge{};
    double score = 0.0;
    bool enterLong = false;
    bool regimeEnded = false;
    bool riskExceeded = false;
    bool _ta_initialized_ = false;
    bool _inputs_initialized_ = false;

    struct _PFScriptState {
        _PFCheckpointTraits<decltype(GeneratedStrategy::_pf_udt_undo)>::snapshot_type _pf_value_0;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_pf_udt_SwitchGauge)>::snapshot_type _pf_value_1;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_1)>::snapshot_type _pf_value_2;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_2)>::snapshot_type _pf_value_3;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_atr_3)>::snapshot_type _pf_value_4;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_crossover_4)>::snapshot_type _pf_value_5;
        _PFCheckpointTraits<decltype(GeneratedStrategy::mode)>::snapshot_type _pf_value_6;
        _PFCheckpointTraits<decltype(GeneratedStrategy::fastLength)>::snapshot_type _pf_value_7;
        _PFCheckpointTraits<decltype(GeneratedStrategy::slowLength)>::snapshot_type _pf_value_8;
        _PFCheckpointTraits<decltype(GeneratedStrategy::atrLength)>::snapshot_type _pf_value_9;
        _PFCheckpointTraits<decltype(GeneratedStrategy::entryScore)>::snapshot_type _pf_value_10;
        _PFCheckpointTraits<decltype(GeneratedStrategy::maximumAtrLoss)>::snapshot_type _pf_value_11;
        _PFCheckpointTraits<decltype(GeneratedStrategy::gauge)>::snapshot_type _pf_value_12;
        _PFCheckpointTraits<decltype(GeneratedStrategy::score)>::snapshot_type _pf_value_13;
        _PFCheckpointTraits<decltype(GeneratedStrategy::enterLong)>::snapshot_type _pf_value_14;
        _PFCheckpointTraits<decltype(GeneratedStrategy::regimeEnded)>::snapshot_type _pf_value_15;
        _PFCheckpointTraits<decltype(GeneratedStrategy::riskExceeded)>::snapshot_type _pf_value_16;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_initialized_)>::snapshot_type _pf_value_17;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_inputs_initialized_)>::snapshot_type _pf_value_18;
    };
    static_assert(std::is_copy_constructible_v<_PFScriptState>, "generated Pine state must be deep-copy constructible");
    static_assert(std::is_copy_assignable_v<_PFScriptState>, "generated Pine state must be deep-copy assignable");
    std::optional<_PFScriptState> _pf_script_state_checkpoint_;

    void snapshot_script_state() override {
        _pf_script_state_checkpoint_.emplace(_PFScriptState{
            _PFCheckpointTraits<decltype(GeneratedStrategy::_pf_udt_undo)>::take(_pf_udt_undo),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_pf_udt_SwitchGauge)>::take(_pf_udt_SwitchGauge),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_1)>::take(_ta_ema_1),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_2)>::take(_ta_ema_2),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_atr_3)>::take(_ta_atr_3),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_crossover_4)>::take(_ta_crossover_4),
            _PFCheckpointTraits<decltype(GeneratedStrategy::mode)>::take(mode),
            _PFCheckpointTraits<decltype(GeneratedStrategy::fastLength)>::take(fastLength),
            _PFCheckpointTraits<decltype(GeneratedStrategy::slowLength)>::take(slowLength),
            _PFCheckpointTraits<decltype(GeneratedStrategy::atrLength)>::take(atrLength),
            _PFCheckpointTraits<decltype(GeneratedStrategy::entryScore)>::take(entryScore),
            _PFCheckpointTraits<decltype(GeneratedStrategy::maximumAtrLoss)>::take(maximumAtrLoss),
            _PFCheckpointTraits<decltype(GeneratedStrategy::gauge)>::take(gauge),
            _PFCheckpointTraits<decltype(GeneratedStrategy::score)>::take(score),
            _PFCheckpointTraits<decltype(GeneratedStrategy::enterLong)>::take(enterLong),
            _PFCheckpointTraits<decltype(GeneratedStrategy::regimeEnded)>::take(regimeEnded),
            _PFCheckpointTraits<decltype(GeneratedStrategy::riskExceeded)>::take(riskExceeded),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_initialized_)>::take(_ta_initialized_),
            _PFCheckpointTraits<decltype(GeneratedStrategy::_inputs_initialized_)>::take(_inputs_initialized_),
        });
    }

    void restore_script_state() override {
        if (!_pf_script_state_checkpoint_) return;
        _PFCheckpointTraits<decltype(GeneratedStrategy::_pf_udt_undo)>::restore(this->_pf_udt_undo, _pf_script_state_checkpoint_->_pf_value_0);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_pf_udt_SwitchGauge)>::restore(this->_pf_udt_SwitchGauge, _pf_script_state_checkpoint_->_pf_value_1);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_1)>::restore(this->_ta_ema_1, _pf_script_state_checkpoint_->_pf_value_2);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_ema_2)>::restore(this->_ta_ema_2, _pf_script_state_checkpoint_->_pf_value_3);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_atr_3)>::restore(this->_ta_atr_3, _pf_script_state_checkpoint_->_pf_value_4);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_crossover_4)>::restore(this->_ta_crossover_4, _pf_script_state_checkpoint_->_pf_value_5);
        _PFCheckpointTraits<decltype(GeneratedStrategy::mode)>::restore(this->mode, _pf_script_state_checkpoint_->_pf_value_6);
        _PFCheckpointTraits<decltype(GeneratedStrategy::fastLength)>::restore(this->fastLength, _pf_script_state_checkpoint_->_pf_value_7);
        _PFCheckpointTraits<decltype(GeneratedStrategy::slowLength)>::restore(this->slowLength, _pf_script_state_checkpoint_->_pf_value_8);
        _PFCheckpointTraits<decltype(GeneratedStrategy::atrLength)>::restore(this->atrLength, _pf_script_state_checkpoint_->_pf_value_9);
        _PFCheckpointTraits<decltype(GeneratedStrategy::entryScore)>::restore(this->entryScore, _pf_script_state_checkpoint_->_pf_value_10);
        _PFCheckpointTraits<decltype(GeneratedStrategy::maximumAtrLoss)>::restore(this->maximumAtrLoss, _pf_script_state_checkpoint_->_pf_value_11);
        _PFCheckpointTraits<decltype(GeneratedStrategy::gauge)>::restore(this->gauge, _pf_script_state_checkpoint_->_pf_value_12);
        _PFCheckpointTraits<decltype(GeneratedStrategy::score)>::restore(this->score, _pf_script_state_checkpoint_->_pf_value_13);
        _PFCheckpointTraits<decltype(GeneratedStrategy::enterLong)>::restore(this->enterLong, _pf_script_state_checkpoint_->_pf_value_14);
        _PFCheckpointTraits<decltype(GeneratedStrategy::regimeEnded)>::restore(this->regimeEnded, _pf_script_state_checkpoint_->_pf_value_15);
        _PFCheckpointTraits<decltype(GeneratedStrategy::riskExceeded)>::restore(this->riskExceeded, _pf_script_state_checkpoint_->_pf_value_16);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_ta_initialized_)>::restore(this->_ta_initialized_, _pf_script_state_checkpoint_->_pf_value_17);
        _PFCheckpointTraits<decltype(GeneratedStrategy::_inputs_initialized_)>::restore(this->_inputs_initialized_, _pf_script_state_checkpoint_->_pf_value_18);
    }

    void commit_script_state() override {
        snapshot_script_state();
    }

    explicit GeneratedStrategy() : _ta_ema_1(15), _ta_ema_2(47), _ta_atr_3(18) {
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

    double _udt_SwitchGauge_weightedSpread(SwitchGauge self, double sensitivity) {
        return (((double)((_pf_udt_SwitchGauge.read(self).fast - _pf_udt_SwitchGauge.read(self).slow)) / (double)(([&]() -> double { double _v0 = (double)(_pf_udt_SwitchGauge.read(self).volatility); double _v1 = (double)(syminfo_.mintick); if (is_na(_v0) || is_na(_v1)) return na<double>(); double _out = _v0; _out = std::max(_out, _v1); return _out; }()))) * sensitivity);
    }

    void on_bar(const Bar& bar) override {
        if (!_inputs_initialized_) {
            mode = get_input_string("Dispatch Mode", std::string("Balanced"));
            fastLength = get_input_int("Fast EMA Length", 15);
            slowLength = get_input_int("Slow EMA Length", 47);
            atrLength = get_input_int("ATR Length", 18);
            entryScore = get_input_double("Entry Score", 0.32);
            maximumAtrLoss = get_input_double("Maximum ATR Loss", 2.3);
            _inputs_initialized_ = true;
        }
        if (!_ta_initialized_) {
            _ta_ema_1 = ta::EMA(get_input_int("Fast EMA Length", 15));
            _ta_ema_2 = ta::EMA(get_input_int("Slow EMA Length", 47));
            _ta_atr_3 = ta::ATR(get_input_int("ATR Length", 18));
            _ta_initialized_ = true;
        }
        gauge = _pf_udt_SwitchGauge.create(_PFUdtRecord_SwitchGauge{.fast = (_use_precalc ? _precalc__ta_ema_1[bar_index_] : (history_advances_new_bar() ? _ta_ema_1.compute(current_bar_.close) : _ta_ema_1.recompute(current_bar_.close))), .slow = (_use_precalc ? _precalc__ta_ema_2[bar_index_] : (history_advances_new_bar() ? _ta_ema_2.compute(current_bar_.close) : _ta_ema_2.recompute(current_bar_.close))), .volatility = (_use_precalc ? _precalc__ta_atr_3[bar_index_] : (history_advances_new_bar() ? _ta_atr_3.compute(current_bar_.high, current_bar_.low, current_bar_.close) : _ta_atr_3.recompute(current_bar_.high, current_bar_.low, current_bar_.close)))});
        auto __switch_val_0 = mode;
        if (__switch_val_0 == std::string("Responsive")) {
            score = _udt_SwitchGauge_weightedSpread(gauge, 1.55);
        }
        else if (__switch_val_0 == std::string("Balanced")) {
            score = _udt_SwitchGauge_weightedSpread(gauge, 1.0);
        }
        else {
            score = _udt_SwitchGauge_weightedSpread(gauge, 0.68);
        }
        enterLong = (history_advances_new_bar() ? _ta_crossover_4.compute(score, entryScore) : _ta_crossover_4.recompute(score, entryScore));
        regimeEnded = ([&]{ auto _pna_l = (score); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }());
        riskExceeded = (([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && ([&]{ auto _pna_l = (current_bar_.close); auto _pna_r = (((signed_position_size() == 0.0 ? na<double>() : position_entry_price_) - (_pf_udt_SwitchGauge.read(gauge).volatility * maximumAtrLoss))); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l < _pfc_r) && !_pfc_eq); }()));
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && (_pfc_eq); }()) && enterLong)) {
            strategy_entry(std::string("Switchboard Long"), true, na<double>(), na<double>(), na<double>(), "");
        } else
        if ((([&]{ auto _pna_l = (signed_position_size()); auto _pna_r = (0); double _pfc_l = static_cast<double>(_pna_l); double _pfc_r = static_cast<double>(_pna_r); bool _pfc_eq = (_pfc_l == _pfc_r) || (std::isfinite(_pfc_l) && std::isfinite(_pfc_r) && std::fabs(_pfc_l - _pfc_r) <= 1e-10); return !is_na(_pna_l) && !is_na(_pna_r) && ((_pfc_l > _pfc_r) && !_pfc_eq); }()) && (regimeEnded || riskExceeded))) {
            strategy_close(std::string("Switchboard Long"), ((riskExceeded) ? (std::string("ATR risk")) : (std::string("Dispatch ended"))), na<double>(), na<double>(), false, 180388626451ULL);
        }
    }

    void precalculate(const Bar* bars, int n) {
        _use_precalc = false;
        if (n <= 0 || bars == nullptr) return;

        _precalc__ta_ema_1.resize(n);
        _precalc__ta_ema_2.resize(n);
        _precalc__ta_atr_3.resize(n);

        _ta_ema_1 = ta::EMA(15);
        _ta_ema_2 = ta::EMA(47);
        _ta_atr_3 = ta::ATR(18);


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
            _precalc__ta_ema_2[i] = _ta_ema_2.compute(bars[i].close);
            _precalc__ta_atr_3[i] = _ta_atr_3.compute(bars[i].high, bars[i].low, bars[i].close);
        }

        _ta_ema_1 = ta::EMA(15);
        _ta_ema_2 = ta::EMA(47);
        _ta_atr_3 = ta::ATR(18);

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
