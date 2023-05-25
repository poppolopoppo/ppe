package utils

import (
	"flag"
	"fmt"
	"io"
)

var LogPersistent = NewLogCategory("Persistent")

type PersistentVar interface {
	fmt.Stringer
	flag.Value
	Serializable
}

type BoolVar = InheritableBool
type IntVar = InheritableInt
type StringVar = InheritableString

type BoolSliceVar = InheritableSlice[BoolVar, *BoolVar]
type IntSliceVar = InheritableSlice[IntVar, *IntVar]
type StringSliceVar = InheritableSlice[StringVar, *StringVar]

type PersistentData interface {
	PinData() map[string]string
	LoadData(object string, property string, value PersistentVar) error
	StoreData(object string, property string, value PersistentVar)
}

type persistentData struct {
	Data map[string]map[string]string
}

func NewPersistentMap(name string) *persistentData {
	return &persistentData{
		Data: make(map[string]map[string]string),
	}
}
func (pmp *persistentData) Len() (result int) {
	for _, vars := range pmp.Data {
		result += len(vars)
	}
	return
}
func (pmp *persistentData) PinData() (result map[string]string) {
	result = make(map[string]string, len(pmp.Data))
	for object, it := range pmp.Data {
		for property, value := range it {
			result[fmt.Sprint(object, `.`, property)] = value
		}
	}
	return
}
func (pmp *persistentData) LoadData(name string, property string, dst PersistentVar) error {
	if object, ok := pmp.Data[name]; ok {
		if value, ok := object[property]; ok {
			LogDebug(LogPersistent, "load object property %s.%s = %v", name, property, value)
			return dst.Set(value)
		} else {
			err := fmt.Errorf("object %q has no property %q", name, property)
			LogWarning(LogPersistent, "load(%s.%s): %v", name, property, err)
			return err
		}

	} else {
		err := fmt.Errorf("object '%s' not found", name)
		LogWarning(LogPersistent, "load(%s.%s): %v", name, property, err)
		return err
	}
}
func (pmp *persistentData) StoreData(name string, property string, dst PersistentVar) {
	LogDebug(LogPersistent, "store in %s.%s = %v", name, property, dst)
	object, ok := pmp.Data[name]
	if !ok {
		object = make(map[string]string)
		pmp.Data[name] = object
	}
	object[property] = dst.String()
}
func (pmp *persistentData) Serialize(dst io.Writer) error {
	if err := JsonSerialize(&pmp.Data, dst, OptionJsonPrettyPrint(true)); err == nil {
		LogDebug(LogPersistent, "saved %d vars from config to disk", pmp.Len())
		return nil
	} else {
		return fmt.Errorf("failed to serialize config: %v", err)
	}
}
func (pmp *persistentData) Deserialize(src io.Reader) error {
	if err := JsonDeserialize(&pmp.Data, src); err == nil {
		LogVerbose(LogPersistent, "loaded %d vars from disk to config", pmp.Len())
		return nil
	} else {
		return fmt.Errorf("failed to deserialize config: %v", err)
	}
}
