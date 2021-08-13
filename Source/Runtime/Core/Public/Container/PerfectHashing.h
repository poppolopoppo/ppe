#pragma once

#include "Core_fwd.h"

#include "Container/Array.h"
#include "Container/Hash.h"
#include "Container/Pair.h"
#include "HAL/PlatformMaths.h"
#include "Meta/Algorithm.h"
#include "Meta/Hash_fwd.h"
#include "Meta/Utility.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key, typename _Value,
    size_t G, size_t N,
    bool _AlwaysExists = false,
    typename _Hash = Meta::THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key> >
class TPerfectHashMap : _Hash, _EqualTo {
public:
    using hash_type = _Hash;
    using equalto_type = _EqualTo;

    CONSTEXPR TPerfectHashMap() NOEXCEPT
        : _offsets{0}
    {}
    CONSTEXPR TPerfectHashMap(_Hash&& hasher, _EqualTo&& equalTo) NOEXCEPT
        : _Hash(hasher)
        , _EqualTo(equalTo)
        , _offsets{0}
    {}

    CONSTEXPR auto operator [](_Key key) const NOEXCEPT {
        return Lookup(key);
    }

    CONSTEXPR auto Lookup(_Key key) const NOEXCEPT {
        const size_t h = static_cast<const hash_type&>(*this)(key);
        const size_t i = (_offsets[h % G] < 0
            ? size_t(-_offsets[h % G]) - 1ul
            : hash_size_t_constexpr(h, _offsets[h % G]) % N);
        return _map.get(i, key);
    }

    template <typename _Pred>
    CONSTEXPR auto Lookup(size_t h, _Pred pred) const NOEXCEPT {
        const size_t i = (_offsets[h % G] < 0
            ? size_t(-_offsets[h % G]) - 1ul
            : hash_size_t_constexpr(h, _offsets[h % G]) % N);
        return _map.get(i, pred);
    }

    using pair_type = TPair<_Key, _Value>;

    template <size_t M, typename = std::enable_if_t<M <= N> >
    CONSTEXPR bool Build(const TStaticArray<pair_type, M>& values) NOEXCEPT {
        // 1 -- prepare nodes and buckets to collect collisions
        node_t nodes[M] = {};
        list_t buckets[G] = {};

        for (u32 i = 0; i < M; ++i) {
            nodes[i].It = &values[i];
            nodes[i].Hash = static_cast<const hash_type&>(*this)(values[i].first);
            nodes[i].Next = nullptr;
        }

        for (u32 i = 0; i < G; ++i) {
            buckets[i].Head = nullptr;
            buckets[i].Count = 0;
            buckets[i].Index = i;
        }

        for (u32 i = 0; i < G; ++i) {
            _offsets[i] = 0;
        }

        // 2 -- hash each node and register in buckets
        for (node_t& n : nodes) {
            list_t& b = buckets[n.Hash % G];
            n.Next = b.Head;
            b.Head = &n;
            b.Count++;
        }

        // 3 -- sort buckets by descending node count
        Meta::StaticQuicksort(buckets, int(G),
            [](const list_t& a, const list_t& b) CONSTEXPR NOEXCEPT {
                return (a.Count > b.Count);
            });

        // 4 -- resolve collisions with greedy offset search
        size_t free = 0;
        Meta::TStaticBitset<N> slots;
        for (list_t& b : buckets) {
            if (0 == b.Count)
                /* stop processing due to descending order */
                break;
            else if (1 == b.Count) {
                /* buckets without collision have an absolute offset encoded as a negative value */
                free = slots.first_free(free);
                if (Meta::TStaticBitset<N>::NPos == free)
                    return false;

                slots.set(free);
                _offsets[b.Index] = -i32(free) - 1;
            }
            else {
                /* buckets with collisions search for an offset which solves all collisions */
                Meta::TStaticBitset<N> tmp(slots);
                const node_t* n = b.Head;

                size_t d = 0;

                while (n) {
                    /* expecting linear time according to academia */
                    const size_t h = (hash_size_t_constexpr(n->Hash, d) % N);
                    if (tmp.test(h)) {
                        tmp = slots;
                        n = b.Head;
                        d++;
                    }
                    else {
                        tmp.set(h);
                        n = n->Next;
                    }
                }

                slots = tmp;
                _offsets[b.Index] = i32(d);
            }
        }

        // 5 -- finally construct the hash map tuples
        for (const node_t& n : nodes) {
            const size_t h = n.Hash % G;
            const size_t g = (_offsets[h] < 0
                ? size_t(-_offsets[h % G]) - 1ul
                : hash_size_t_constexpr(n.Hash, _offsets[h]) % N);
            _map.set(g, n.It->first, n.It->second);
        }

        return true;
    }

private:
    static CONSTEXPR auto addressof(const _Value& v) NOEXCEPT {
        IF_CONSTEXPR(std::is_pointer_v<_Value>)
            return v;
        else
            return std::addressof(v);
    }

    struct keys_values_t {
        _Key Keys[N];
        _Value Values[N];
        CONSTEXPR keys_values_t() NOEXCEPT
            : Keys{}
            , Values{}
        {}
        CONSTEXPR auto get(size_t i, const _Key& k) const NOEXCEPT {
            return (equalto_type()(Keys[i], k) ? addressof(Values[i]) : nullptr);
        }
        template <typename _Pred>
        CONSTEXPR auto get(size_t i, const _Pred& pred) const NOEXCEPT {
            return (pred(Keys[i]) ? addressof(Values[i]) : nullptr);
        }
        CONSTEXPR void set(size_t i, const _Key& k, const _Value& v) NOEXCEPT {
            Keys[i] = k;
            Values[i] = v;
        }
    };

    struct values_only_t {
        _Value Values[N];
        CONSTEXPR values_only_t() NOEXCEPT
            : Values{}
        {}
        CONSTEXPR const _Value& get(size_t i, const _Key&) const NOEXCEPT {
            return Values[i];
        }
        template <typename _Pred>
        CONSTEXPR const _Value& get(size_t i, const _Pred&) const NOEXCEPT {
            return Values[i];
        }
        CONSTEXPR void set(size_t i, const _Key&, const _Value& v) NOEXCEPT {
            Values[i] = v;
        }
    };

    using map_t = std::conditional_t<
        _AlwaysExists,
        values_only_t, keys_values_t >;

    struct node_t {
        const pair_type* It;
        size_t Hash;
        node_t* Next;
    };

    struct list_t {
        node_t* Head;
        u32 Count;
        u32 Index;
        CONSTEXPR inline friend void swap(list_t& a, list_t& b) NOEXCEPT {
            const list_t tmp = a;
            a = b;
            b = tmp;
        }
    };

    i32 _offsets[G];
    map_t _map;
};
//----------------------------------------------------------------------------
template <bool _AlwaysExists, typename _Key, typename _Value, size_t N,
    typename _Hash = Meta::THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key> >
CONSTEXPR auto MinimalPerfectHashMap(const TStaticArray<TPair<_Key, _Value>, N>& values,
    _Hash hasher = _Hash{},
    _EqualTo equalTo = _EqualTo{} ) NOEXCEPT{
    CONSTEXPR size_t R = 5;
    CONSTEXPR size_t G =
#if 1
        FPlatformMaths::NextPrime(N / R);
#else
        FPlatformMaths::NextPow2(N / R);
#endif
    TPerfectHashMap<_Key, _Value, G, N, _AlwaysExists, _Hash, _EqualTo> mph(
        std::move(hasher),
        std::move(equalTo) );
    mph.Build(values);
    return mph;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
