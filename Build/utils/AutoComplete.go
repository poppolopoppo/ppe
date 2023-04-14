package utils

import (
	"sort"
	"strings"
)

/***************************************
 * AutoComplete
 ***************************************/

func lcs_ex(a, b []rune, rows *[]int) int {
	m, n := len(a), len(b)

	if len(*rows) < (m+1)*(n+1) {
		*rows = make([]int, (m+1)*(n+1))
	} else {
		required := (m + 1) * (n + 1)
		for i := 0; i < required; i += 1 {
			(*rows)[i] = 0
		}
	}

	for i := 0; i <= m; i += 1 {
		for j := 0; j <= n; j += 1 {
			if i == 0 || j == 0 {
				(*rows)[i*n+j] = 0
			} else if a[i-1] == b[j-1] {
				(*rows)[i*n+j] = (*rows)[(i-1)*n+(j-1)] + 1
			} else if (*rows)[(i-1)*n+j] > (*rows)[i*n+(j-1)] {
				(*rows)[i*n+j] = (*rows)[(i-1)*n+j]
			} else {
				(*rows)[i*n+j] = (*rows)[i*n+(j-1)]
			}
		}
	}

	return (*rows)[m*n+n]
}

type AutoComplete interface {
	Any(interface{})
	Append(in AutoCompletable)
	Add(in string) int
	Results(n int) []string
}

type AutoCompletable interface {
	AutoComplete(AutoComplete)
}

type BasicAutoComplete struct {
	input   []rune
	entries []struct {
		Text  string
		Score int
	}
	rows []int
}

func NewAutoComplete(input string) BasicAutoComplete {
	return BasicAutoComplete{
		input: []rune(strings.ToUpper(input)),
	}
}
func (x *BasicAutoComplete) Any(anon interface{}) {
	if autocomplete, ok := anon.(AutoCompletable); ok {
		autocomplete.AutoComplete(x)
	}
}
func (x *BasicAutoComplete) Append(in AutoCompletable) {
	in.AutoComplete(x)
}
func (x *BasicAutoComplete) Add(in string) int {
	AssertNotIn(in, "")
	score := lcs_ex(x.input, []rune(strings.ToUpper(in)), &x.rows)
	if score > 0 {
		x.entries = append(x.entries, struct {
			Text  string
			Score int
		}{
			Text:  in,
			Score: score,
		})
	}
	return score
}
func (x *BasicAutoComplete) Results(n int) []string {
	sort.Slice(x.entries, func(i, j int) bool {
		return x.entries[i].Score > x.entries[j].Score
	})
	if n < 0 || n > len(x.entries) {
		n = len(x.entries)
	}
	return Map(func(it struct {
		Text  string
		Score int
	}) string {
		return it.Text
	}, x.entries[:n]...)
}

type PrefixedAutoComplete struct {
	prefix string
	inner  AutoComplete
}

func NewPrefixedAutoComplete(prefix string, inner AutoComplete) PrefixedAutoComplete {
	Assert(func() bool { return prefix == "" })
	Assert(func() bool { return !IsNil(inner) })
	return PrefixedAutoComplete{
		prefix: prefix,
		inner:  inner,
	}
}
func (x *PrefixedAutoComplete) Any(anon interface{}) {
	if autocomplete, ok := anon.(AutoCompletable); ok {
		autocomplete.AutoComplete(x)
	}
}
func (x *PrefixedAutoComplete) Append(in AutoCompletable) {
	in.AutoComplete(x)
}
func (x *PrefixedAutoComplete) Add(in string) int {
	AssertNotIn(in, "")
	return x.inner.Add(x.prefix + in)
}
func (x *PrefixedAutoComplete) Results(n int) []string {
	return x.inner.Results(n)
}
