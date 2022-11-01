package utils

import (
	"bytes"
	"fmt"
	"reflect"
	"sort"
	"strings"
	"sync"
)

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

func Keys[K comparable, V any](it map[K]V) []K {
	off := 0
	result := make([]K, len(it))
	for key := range it {
		result[off] = key
		off += 1
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
func (set *SetT[T]) AppendUniq(it ...T) *SetT[T] {
	for _, x := range it {
		if !set.Contains(x) {
			set.Append(x)
		}
	}
	return set
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

const STRING_INTERNER_BUCKETS uint32 = 16
const STRING_INTERNER_MINLEN int = 12

var stringInterner [STRING_INTERNER_BUCKETS]SharedMapT[string, string]

func InternString(in string) string {
	if len(in) < STRING_INTERNER_MINLEN {
		return in
	}
	h := FNV32a(in) % STRING_INTERNER_BUCKETS
	interner := &stringInterner[h]
	if out, ok := interner.Get(in); !ok {
		interner.Add(in, in)
		return in
	} else {
		return out
	}
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
		*set = AppendComparable_CheckUniq(*set, InternString(x))
	}
	return set
}
func (set *StringSet) Prepend(it ...string) *StringSet {
	for _, x := range it {
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

func (set StringSet) GetDigestable(o *bytes.Buffer) {
	for i, x := range set {
		o.WriteByte(byte(i))
		o.WriteString(x)
	}
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

/***************************************
 * Services
 ***************************************/
type ServiceLocator[V any] struct {
	*SharedMapT[string, V]
}
type ServiceAccessor[T, V any] struct {
	key     string
	factory func() *T
}

func NewServiceLocator[V any]() ServiceLocator[V] {
	return ServiceLocator[V]{NewSharedMapT[string, V]()}
}

func MakeServiceAccessor[V, T any](factory func() *T) ServiceAccessor[T, V] {
	var dummy *T
	rt := reflect.TypeOf(dummy).Elem()
	return ServiceAccessor[T, V]{
		key:     rt.Name(),
		factory: factory,
	}
}

func (acc ServiceAccessor[T, V]) Get(services ServiceLocator[V]) (*T, error) {
	if anon, ok := services.Get(acc.key); ok {
		var x interface{} = anon
		return x.(*T), nil
	} else {
		return nil, fmt.Errorf("service unavailable: '%v'", acc.key)
	}
}
func (acc ServiceAccessor[T, V]) Need(services ServiceLocator[V]) *T {
	if x, err := acc.Get(services); err == nil {
		return x
	} else {
		LogPanicErr(err)
		return nil
	}
}
func (acc ServiceAccessor[T, V]) Create(services ServiceLocator[V]) *T {
	concrete := acc.factory()
	var x interface{} = concrete
	services.Add(acc.key, x.(V))
	return concrete
}
func (acc ServiceAccessor[T, V]) Add(services ServiceLocator[V]) {
	acc.Create(services)
}
func (acc ServiceAccessor[T, V]) FindOrAdd(services ServiceLocator[V]) *T {
	if x, err := acc.Get(services); err == nil {
		return x
	} else {
		return acc.Create(services)
	}
}

type ServiceEvent[V any] struct {
	barrier sync.Mutex
	arr     []func(ServiceLocator[V])
}

func NewServiceEvent[V any]() *ServiceEvent[V] {
	return &ServiceEvent[V]{
		barrier: sync.Mutex{},
	}
}

func (evt *ServiceEvent[V]) Append(it ...func(ServiceLocator[V])) {
	evt.barrier.Lock()
	defer evt.barrier.Unlock()
	evt.arr = append(evt.arr, it...)
}
func (evt *ServiceEvent[V]) Needed(services ServiceLocator[V]) {
	evt.barrier.Lock()
	defer evt.barrier.Unlock()
	for _, x := range evt.arr {
		x(services)
	}
}

/***************************************
 * Collector
 ***************************************/

type KeyValuePair[K, V any] struct {
	Key   K
	Value V
}

type Collector[K Comparable[K], V any] struct {
	keys   []KeyValuePair[K, int]
	values []*[]V
}

func NewCollector[K Comparable[K], V any]() *Collector[K, V] {
	return &Collector[K, V]{
		keys:   []KeyValuePair[K, int]{},
		values: []*[]V{},
	}
}
func (x *Collector[K, V]) IndexOf(key K) (int, bool) {
	it := sort.Search(len(x.keys), func(i int) bool {
		return x.keys[i].Key.Compare(key) >= 0
	})
	return it, it < len(x.keys) && x.keys[it].Key.Compare(key) == 0
}
func (x *Collector[K, V]) Add(key K, value V) {
	if it, found := x.IndexOf(key); found {
		arr := x.values[x.keys[it].Value]
		*arr = append(*arr, value)
	} else {
		Assert(func() bool {
			for i := 0; i < len(x.keys); i += 1 {
				if x.keys[i].Key.Compare(key) == 0 {
					return false
				}
			}
			return true
		})
		x.keys = InsertAt(x.keys, it, KeyValuePair[K, int]{key, len(x.values)})
		x.values = append(x.values, &[]V{value})
	}
}
func (x *Collector[K, V]) Keys() []K {
	result := make([]K, len(x.keys))
	for i, kv := range x.keys {
		result[i] = kv.Key
	}
	return result
}
func (x *Collector[K, V]) Values(key K) *[]V {
	if it, found := x.IndexOf(key); found {
		return x.values[x.keys[it].Value]
	} else {
		return nil
	}
}
func (x *Collector[K, V]) Each(each func(K, []V)) {
	for _, it := range x.keys {
		each(it.Key, *x.values[it.Value])
	}
}

type StringCollector[T any] struct {
	keys   []string
	values map[string]*[]T
}

func NewStringCollector[T any]() *StringCollector[T] {
	return &StringCollector[T]{
		keys:   []string{},
		values: make(map[string]*[]T),
	}
}
func (x *StringCollector[T]) IndexOf(key string) (int, bool) {
	it := sort.SearchStrings(x.keys, key)
	return it, it < len(x.keys) && x.keys[it] == key
}
func (x *StringCollector[T]) Add(key string, value T) {
	if it, found := x.IndexOf(key); found {
		arr := x.values[x.keys[it]]
		*arr = append(*arr, value)
	} else {
		x.values[key] = &[]T{value}
		x.keys = InsertAt(x.keys, it, key)
	}
}
func (x *StringCollector[T]) Keys() []string {
	return x.keys
}
func (x *StringCollector[T]) Values(key string) *[]T {
	return x.values[key]
}
func (x *StringCollector[T]) Each(each func(string, []T)) {
	for _, key := range x.keys {
		each(key, *x.values[key])
	}
}
