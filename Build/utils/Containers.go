package utils

import (
	"flag"
	"fmt"
	"hash/fnv"
	"sort"
	"strings"
	"sync"
	"sync/atomic"
)

func CopySlice[T any](in ...T) []T {
	out := make([]T, len(in))
	copy(out, in)
	return out
}

func CopyMap[K comparable, V any](in map[K]V) map[K]V {
	out := make(map[K]V, len(in))
	for key, value := range in {
		out[key] = value
	}
	return out
}

func Where[T any](pred func(T) bool, values ...T) (result T) {
	if i, ok := IndexIf(pred, values...); ok {
		result = values[i]
	}
	return result
}

func IndexOf[T comparable](match T, values ...T) (int, bool) {
	for i, x := range values {
		if x == match {
			return i, true
		}
	}
	return -1, false
}

func IndexIf[T any](pred func(T) bool, values ...T) (int, bool) {
	for i, x := range values {
		if pred(x) {
			return i, true
		}
	}
	return -1, false
}

func InsertAt[T any](arr []T, index int, value T) []T {
	if len(arr) == index {
		return append(arr, value)
	}
	arr = append(arr[:index+1], arr[index:]...)
	arr[index] = value
	return arr
}

func Contains[T comparable](arr []T, values ...T) bool {
	for _, x := range values {
		if _, ok := IndexOf(x, arr...); !ok {
			return false
		}
	}
	return true
}

func AppendUniq[T comparable](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		if _, ok := IndexOf(x, result...); !ok {
			result = append(result, x)
		}
	}
	return result
}

func Remove[T comparable](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		if i, ok := IndexOf(x, result...); !ok {
			result = append(result[:i], result[i+1:]...)
		}
	}
	return result
}

func RemoveUnless[T any](pred func(T) bool, src ...T) (result []T) {
	off := 0
	result = make([]T, len(src))
	for i, x := range src {
		if pred(x) {
			result[off] = src[i]
			off += 1
		}
	}
	return result[:off]
}

func Keys[K comparable, V any](elts ...map[K]V) []K {
	n := 0
	for _, it := range elts {
		n += len(it)
	}
	off := 0
	result := make([]K, n)
	for _, it := range elts {
		for key := range it {
			result[off] = key
			off += 1
		}
	}
	return result
}

/***************************************
 * Containers
 ***************************************/

type SetT[T comparable] []T

func NewSet[T comparable](it ...T) SetT[T] {
	return SetT[T](it)
}

func (set SetT[T]) Empty() bool {
	return len(set) == 0
}
func (set SetT[T]) Len() int {
	return len(set)
}
func (set SetT[T]) At(i int) T {
	return set[i]
}
func (set SetT[T]) Swap(i, j int) {
	set[i], set[j] = set[j], set[i]
}
func (set SetT[T]) Range(each func(T)) {
	for _, x := range set {
		each(x)
	}
}
func (set *SetT[T]) Ref() *[]T {
	return (*[]T)(set)
}
func (set SetT[T]) Slice() []T {
	return set
}
func (set SetT[T]) IndexOf(it T) (int, bool) {
	for i, x := range set {
		if x == it {
			return i, true
		}
	}
	return len(set), false
}

func (set SetT[T]) Contains(it ...T) bool {
	for _, x := range it {
		if _, ok := set.IndexOf(x); !ok {
			return false
		}
	}
	return true
}
func (set *SetT[T]) Append(it ...T) *SetT[T] {
	*set = AppendComparable_CheckUniq(*set, it...)
	return set
}
func (set *SetT[T]) Prepend(it ...T) *SetT[T] {
	*set = PrependComparable_CheckUniq(it, (*set)...)
	return set
}
func (set *SetT[T]) AppendUniq(it ...T) bool {
	for _, x := range it {
		if !set.Contains(x) {
			set.Append(x)
			return true
		}
	}
	return false
}
func (set *SetT[T]) Remove(x T) *SetT[T] {
	if i, ok := set.IndexOf(x); ok {
		set.Delete(i)
	} else {
		LogPanic("could not find item in set")
	}
	return set
}
func (set *SetT[T]) RemoveAll(it ...T) *SetT[T] {
	for _, x := range it {
		if i, ok := set.IndexOf(x); ok {
			set.Delete(i)
		}
	}
	return set
}
func (set *SetT[T]) RemoveUnless(pred func(T) bool) (result SetT[T]) {
	return SetT[T](RemoveUnless(pred, set.Slice()...))
}
func (set *SetT[T]) Delete(i int) *SetT[T] {
	*set = append((*set)[:i], (*set)[i+1:]...)
	return set
}
func (set *SetT[T]) Clear() *SetT[T] {
	*set = []T{}
	return set
}
func (set *SetT[T]) Assign(arr []T) *SetT[T] {
	*set = arr
	return set
}
func (set SetT[T]) Sort(less func(a, b T) bool) {
	sort.Slice(set, func(i, j int) bool {
		return less(set[i], set[j])
	})
}

/***************************************
 * String interner
 ***************************************/

const (
	STRING_INTERNER_ENABLED        = false
	STRING_INTERNER_BUCKETS uint32 = 512
	STRING_INTERNER_MINLEN  int    = 12
)

var stringInterner [STRING_INTERNER_BUCKETS]SharedMapT[string, string]

func FNV32a(in string) uint32 {
	buffer := TransientBuffer.Allocate()
	defer TransientBuffer.Release(buffer)

	buffer.WriteString(in)

	h := fnv.New32a()
	h.Write(buffer.Bytes())
	return h.Sum32()
}

var stringInterner_CacheHit atomic.Int64
var stringInterner_CacheMiss atomic.Int64

func InternString(in string) string {
	if !STRING_INTERNER_ENABLED || len(in) < STRING_INTERNER_MINLEN {
		return in
	}
	h := FNV32a(in) % STRING_INTERNER_BUCKETS
	interner := &stringInterner[h]
	if out, ok := interner.Get(in); !ok {
		stringInterner_CacheMiss.Add(1)
		interner.Add(in, in)
		return in
	} else {
		stringInterner_CacheHit.Add(1)
		return out
	}
}

/***************************************
 * Enum Flags
 ***************************************/

type EnumFlag interface {
	Ord() int32
	comparable
	fmt.Stringer
}

type EnumSet[T EnumFlag, E interface {
	*T
	FromOrd(int32)
	EnumFlag
	flag.Value
}] int32

func (x EnumSet[T, E]) Ord() int32  { return int32(x) }
func (x EnumSet[T, E]) Empty() bool { return x == 0 }
func (x EnumSet[T, E]) Has(it T) bool {
	return (x.Ord() & (1 << it.Ord())) == (1 << it.Ord())
}
func (x EnumSet[T, E]) Elements() (result []T) {
	var it T
	for i := int32(0); i < 32; i += 1 {
		E(&it).FromOrd(i)
		if x.Has(it) {
			result = append(result, it)
		}
	}
	return
}
func (x EnumSet[T, E]) Intersect(other EnumSet[T, E]) EnumSet[T, E] {
	return EnumSet[T, E](x.Ord() & other.Ord())
}
func (x *EnumSet[T, E]) Clear() {
	*x = EnumSet[T, E](0)
}
func (x *EnumSet[T, E]) Append(other EnumSet[T, E]) {
	*x = EnumSet[T, E](x.Ord() | other.Ord())
}
func (x *EnumSet[T, E]) Add(elements ...T) {
	for _, it := range elements {
		*x = EnumSet[T, E](x.Ord() | (1 << E(&it).Ord()))
	}
}
func (x *EnumSet[T, E]) Remove(elements ...T) {
	for _, it := range elements {
		*x = EnumSet[T, E](x.Ord() & ^(1 << E(&it).Ord()))
	}
}
func (x EnumSet[T, E]) Compare(other EnumSet[T, E]) int {
	if x.Ord() < other.Ord() {
		return -1
	} else if x.Ord() == other.Ord() {
		return 0
	} else {
		return 1
	}
}
func (x EnumSet[T, E]) String() string {
	return JoinString("|", x.Elements()...)
}
func (x *EnumSet[T, E]) Set(in string) error {
	*x = EnumSet[T, E](0)
	for _, s := range strings.Split(in, "|") {
		var it T
		if err := E(&it).Set(strings.TrimSpace(s)); err == nil {
			x.Add(it)
		} else {
			return err
		}
	}
	return nil
}
func (x *EnumSet[T, E]) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}
func (x EnumSet[T, E]) MarshalText() ([]byte, error) {
	return UnsafeBytesFromString(x.String()), nil
}
func (x *EnumSet[T, E]) UnmarshalText(data []byte) error {
	return x.Set(UnsafeStringFromBytes(data))
}

func MakeEnumSet[T EnumFlag, E interface {
	*T
	FromOrd(int32)
	EnumFlag
	flag.Value
}](elements ...T) (result EnumSet[T, E]) {
	result.Add(elements...)
	return result
}

/***************************************
 * String set
 ***************************************/

type StringSet SetT[string]

type StringSetable interface {
	StringSet() StringSet
}

func (set StringSet) Len() int {
	return len(set)
}
func (set StringSet) At(i int) string {
	return set[i]
}
func (set StringSet) Swap(i, j int) {
	set[i], set[j] = set[j], set[i]
}
func (set StringSet) Range(each func(string)) {
	for _, x := range set {
		each(x)
	}
}
func (set StringSet) Slice() []string {
	return set
}
func (set StringSet) IndexOf(it string) (int, bool) {
	for i, x := range set {
		if x == it {
			return i, true
		}
	}
	return len(set), false
}

func (set StringSet) Any(it ...string) bool {
	for _, x := range it {
		if _, ok := set.IndexOf(x); ok {
			return true
		}
	}
	return false
}
func (set StringSet) Contains(it ...string) bool {
	for _, x := range it {
		if _, ok := set.IndexOf(x); !ok {
			return false
		}
	}
	return true
}
func (set *StringSet) Concat(other StringSet) StringSet {
	return NewStringSet(append(set.Slice(), other.Slice()...)...)
}
func (set *StringSet) Append(it ...string) *StringSet {
	for _, x := range it {
		Assert(func() bool { return len(x) > 0 })
		*set = AppendComparable_CheckUniq(*set, InternString(x))
	}
	return set
}
func (set *StringSet) Prepend(it ...string) *StringSet {
	for _, x := range it {
		Assert(func() bool { return len(x) > 0 })
		*set = PrependComparable_CheckUniq(*set, InternString(x))
	}
	return set
}
func (set *StringSet) AppendUniq(it ...string) *StringSet {
	for _, x := range it {
		if !set.Contains(x) {
			set.Append(InternString(x))
		}
	}
	return set
}
func (set *StringSet) Remove(it ...string) *StringSet {
	for _, x := range it {
		Assert(func() bool { return len(x) > 0 })
		if i, ok := set.IndexOf(x); ok {
			set.Delete(i)
		}
	}
	return set
}
func (set *StringSet) Delete(i int) *StringSet {
	*set = append((*set)[:i], (*set)[i+1:]...)
	return set
}
func (set *StringSet) Clear() *StringSet {
	*set = []string{}
	return set
}
func (set *StringSet) Assign(arr []string) *StringSet {
	*set = arr
	return set
}
func (set StringSet) Sort() {
	sort.Strings(set)
}
func (set StringSet) StringSet() StringSet {
	return set
}
func (set *StringSet) Serialize(ar Archive) {
	SerializeMany(ar, ar.String, (*[]string)(set))
}

func NewStringSet(x ...string) StringSet {
	return x
}

func MakeStringerSet[T fmt.Stringer](it ...T) (result StringSet) {
	if !enableDiagnostics {
		result = make(StringSet, len(it))
		for i, x := range it {
			result[i] = x.String()
		}
	} else {
		for _, x := range it {
			result.Append(x.String())
		}
	}
	return result
}

func (set StringSet) Join(sep string) string {
	return strings.Join(set.Slice(), sep)
}
func (set StringSet) String() string {
	return set.Join(",")
}
func (set *StringSet) Set(in string) error {
	set.Clear()
	for _, x := range strings.Split(in, ",") {
		set.Append(x)
	}
	return nil
}

func (set StringSet) ToDirSet(root Directory) (result DirSet) {
	return Map(func(x string) Directory {
		return root.AbsoluteFolder(x).Normalize()
	}, set.Slice()...)
}
func (set StringSet) ToFileSet(root Directory) (result FileSet) {
	return Map(func(x string) Filename {
		return root.AbsoluteFile(x).Normalize()
	}, set.Slice()...)
}

/***************************************
 * Shared set
 ***************************************/

type SharedSliceT[T any] struct {
	intern []T
	sync.Mutex
}

func (x *SharedSliceT[T]) Append(it ...T) {
	x.Lock()
	x.intern = append(x.intern, it...)
	x.Unlock()
}
func (x *SharedSliceT[T]) Slice() []T {
	x.Lock()
	defer x.Unlock()
	return x.intern
}

/***************************************
 * Shared map
 ***************************************/

type SharedMapT[K comparable, V any] struct {
	intern sync.Map
}

func NewSharedMapT[K comparable, V any]() *SharedMapT[K, V] {
	return &SharedMapT[K, V]{sync.Map{}}
}
func (shared *SharedMapT[K, V]) Len() (count int) {
	shared.intern.Range(func(_, _ interface{}) bool {
		count += 1
		return true
	})
	return count
}
func (shared *SharedMapT[K, V]) Keys() (result []K) {
	result = []K{}
	shared.intern.Range(func(k, _ interface{}) bool {
		result = append(result, k.(K))
		return true
	})
	return result
}
func (shared *SharedMapT[K, V]) Values() (result []V) {
	result = []V{}
	shared.intern.Range(func(_, v interface{}) bool {
		result = append(result, v.(V))
		return true
	})
	return result
}
func (shared *SharedMapT[K, V]) Range(each func(K, V)) {
	shared.intern.Range(func(k, v interface{}) bool {
		each(k.(K), v.(V))
		return true
	})
}
func (shared *SharedMapT[K, V]) Add(key K, value V) V {
	shared.intern.Store(key, value)
	return value
}
func (shared *SharedMapT[K, V]) FindOrAdd(key K, value V) (V, bool) {
	actual, loaded := shared.intern.LoadOrStore(key, value)
	return actual.(V), loaded
}
func (shared *SharedMapT[K, V]) Get(key K) (result V, ok bool) {
	if value, ok := shared.intern.Load(key); ok {
		return value.(V), true
	} else {
		return result, false
	}
}
func (shared *SharedMapT[K, V]) Delete(key K) {
	shared.intern.Delete(key)
}
func (shared *SharedMapT[K, V]) Pin() map[K]V {
	result := make(map[K]V, shared.Len())
	shared.Range(func(k K, v V) {
		result[k] = v
	})
	return result
}
func (shared *SharedMapT[K, V]) Make(fn func() (K, V)) V {
	k, v := fn()
	shared.Add(k, v)
	return v
}
func (shared *SharedMapT[K, V]) Clear() {
	shared.intern = sync.Map{}
}
