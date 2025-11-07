#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <cstdint>

namespace improved {
    template <typename TKey, typename TVal>
    class Container {
    public:
        using ValueType = std::decay_t<TVal>;
        using KeyType = std::decay_t<TKey>;

        using PtrType = std::unique_ptr<ValueType>;

        using MapType = std::unordered_map<KeyType, ValueType*>;
        using VecType = std::vector<PtrType>;
    public:
        Container() = default;
        Container(const Container&) = delete;
        Container& operator=(const Container&) = delete;

        // Yeni eleman ekler, duplicate key varsa false döner.
        template <typename... Args>
        bool Emplace(const KeyType& key, Args&&... args) {
            if (map_.find(key) != map_.end())
                return false;

            PtrType ptr = std::make_unique<ValueType>(std::forward<Args>(args)...);

            if (!ptr) return false;

            ValueType* raw = ptr.get();
            map_.try_emplace(key, raw);

            if (vec_.capacity() == vec_.size())
                vec_.reserve(vec_.capacity() + 8);

            vec_.emplace_back(std::move(ptr));
            return true;
        }

        // Key’e göre eleman bulur.
        ValueType* FindByKey(const KeyType& key) noexcept {
            auto it = map_.find(key);
            return (it != map_.end()) ? it->second : nullptr;
        }

        const ValueType* FindByKey(const KeyType& key) const noexcept {
            auto it = map_.find(key);
            return (it != map_.end()) ? it->second : nullptr;
        }

        // Key’e göre eleman siler.
        bool EraseByKey(const KeyType& key) noexcept {
            auto it = map_.find(key);
            if (it == map_.end())
                return false;

            ValueType* ptr = it->second;
            map_.erase(it);

            auto vit = std::find_if(vec_.begin(), vec_.end(),
                [&](const PtrType& p) { return p.get() == ptr; });

            //sync diff: impossible case
            if (vit == vec_.end()) return false;

            //Eğer vec_.back()’teki eleman baska bir key’e bagliysa, map_ hala eski ptr'i tutmaya devam eder.

            /*if (vit != vec_.end() - 1) {
                *vit = std::move(vec_.back());
            }
            vec_.pop_back();*/
            vec_.erase(vit);
            return true;
        }

        // Lambda fonksiyonuyla tüm elemanları işler.
        template<typename Func, typename = std::enable_if_t<std::is_invocable_v<Func, ValueType&>>>
        void ForEach(Func&& f) noexcept {
            for (PtrType& elem : vec_) {
                auto* ptr = elem.get();
                if (!ptr) continue;
                f(*ptr);
            }
        }

        // Range-based for desteği.
        auto begin() noexcept { return vec_.begin(); }
        auto end() noexcept { return vec_.end(); }
        auto begin() const noexcept { return vec_.begin(); }
        auto end() const noexcept { return vec_.end(); }

        // Bilgi fonksiyonları.
        size_t Size() const noexcept { return vec_.size(); }
        bool Empty() const noexcept { return vec_.empty(); }

        // Tüm container’ı temizler.
        void Clear() noexcept {
            map_.clear();
            vec_.clear();
        }
    private:
        MapType map_;
        VecType vec_;
    };

}