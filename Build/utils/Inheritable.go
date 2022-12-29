package utils

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strconv"
	"strings"
)

/***************************************
 * Inheritable interface
 ***************************************/

type InheritableBase interface {
	IsInheritable() bool
}

type Inheritable[T any] interface {
	Inherit(*T)
	Overwrite(*T)
}

type TInheritableScalar[T InheritableBase] struct {
	Value T
}

func (x *TInheritableScalar[T]) Inherit(other T) {
	if x.Value.IsInheritable() {
		x.Value = other
	}
}
func (x *TInheritableScalar[T]) Overwrite(other T) {
	if !other.IsInheritable() {
		x.Value = other
	}
}

func MakeInheritable[T InheritableBase](value T) TInheritableScalar[T] {
	return TInheritableScalar[T]{value}
}

func Inherit[T InheritableBase](result *T, values ...T) {
	wrapper := MakeInheritable(*result)
	for _, it := range values {
		wrapper.Inherit(it)
	}
	*result = wrapper.Value
}
func Overwrite[T InheritableBase](result *T, values ...T) {
	wrapper := MakeInheritable(*result)
	for _, it := range values {
		wrapper.Inherit(it)
	}
	*result = wrapper.Value
}

/***************************************
 * InheritableInt
 ***************************************/

type InheritableInt int

const (
	INHERIT_VALUE InheritableInt = 0
)

func (x InheritableInt) Get() int { return int(x) }

func (x InheritableInt) Equals(o InheritableInt) bool {
	return x == o
}
func (x InheritableInt) GetDigestable(o *bytes.Buffer) {
	tmp := [binary.MaxVarintLen64]byte{}
	len := binary.PutUvarint(tmp[:], uint64(x.Get()))
	o.Write(tmp[:len])
}
func (x InheritableInt) IsInheritable() bool {
	return x == INHERIT_VALUE
}

func (x InheritableInt) String() string {
	if x.IsInheritable() {
		return "INHERIT"
	}
	return fmt.Sprint(x.Get())
}
func (x *InheritableInt) Set(in string) error {
	switch strings.ToUpper(in) {
	case "INHERIT":
		*x = INHERIT_VALUE
		return nil
	default:
		if v, err := strconv.Atoi(in); err == nil {
			*x = InheritableInt(v)
			return nil
		} else {
			return err
		}
	}
}
func (x InheritableInt) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *InheritableInt) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

/***************************************
 * InheritableBool
 ***************************************/

type InheritableBool InheritableInt

const INHERITABLE_INHERIT InheritableBool = 0
const INHERITABLE_FALSE InheritableBool = 1
const INHERITABLE_TRUE InheritableBool = 2

func (x *InheritableBool) AsInt() *InheritableInt {
	type InheritableIntPtr = *InheritableInt
	return InheritableIntPtr(x)
}

func (x InheritableBool) IsBoolFlag() bool { return true } // BoolFlag interface
func (x InheritableBool) Get() bool        { return x == INHERITABLE_TRUE }

func (x InheritableBool) Equals(o InheritableBool) bool {
	return x == o
}
func (x InheritableBool) GetDigestable(o *bytes.Buffer) {
	x.AsInt().GetDigestable(o)
}
func (x InheritableBool) IsInheritable() bool {
	return x == INHERITABLE_INHERIT
}

func (x *InheritableBool) Enable() {
	*x = INHERITABLE_TRUE
}
func (x *InheritableBool) Disable() {
	*x = INHERITABLE_FALSE
}
func (x *InheritableBool) Toggle() {
	if x.Get() {
		x.Disable()
	} else {
		x.Enable()
	}
}

func (x InheritableBool) String() string {
	if x.Get() {
		return "TRUE"
	} else if !x.IsInheritable() {
		return "FALSE"
	} else {
		return "INHERIT"
	}
}
func (x *InheritableBool) Set(in string) error {
	switch strings.ToUpper(in) {
	case "TRUE":
		x.Enable()
		return nil
	case "FALSE":
		x.Disable()
		return nil
	default:
		return x.AsInt().Set(in)
	}
}
func (x InheritableBool) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *InheritableBool) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}
