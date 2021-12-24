package utils

import (
	"bytes"
	"encoding/binary"
	"flag"
	"fmt"
	"io"
	"sort"
	"strconv"
)

type ParsableFlags interface {
	InitFlags(persistent *PersistentMap)
	ApplyVars(persistent *PersistentMap)
}

type PersistentVar interface {
	fmt.Stringer
	flag.Value
}

type BoolFlag interface {
	IsBoolFlag() bool
	flag.Value
}

type BoolVar bool

func (v BoolVar) IsBoolFlag() bool { return true }
func (v BoolVar) Get() bool        { return bool(v) }
func (v BoolVar) String() string {
	return fmt.Sprint(v.Get())
}
func (v *BoolVar) Set(in string) error {
	if x, err := strconv.ParseBool(in); err == nil {
		*v = BoolVar(x)
		return nil
	} else {
		return err
	}
}
func (v BoolVar) GetDigestable(o *bytes.Buffer) {
	tmp := [1]byte{}
	if v.Get() {
		tmp[0] = 1
	}
	o.Write(tmp[:])
}

type IntVar int

func (v IntVar) Get() int { return int(v) }
func (v IntVar) String() string {
	return fmt.Sprint(v.Get())
}
func (v *IntVar) Set(in string) error {
	if x, err := strconv.Atoi(in); err == nil {
		*v = IntVar(x)
		return nil
	} else {
		return err
	}
}
func (v IntVar) GetDigestable(o *bytes.Buffer) {
	tmp := [binary.MaxVarintLen64]byte{}
	len := binary.PutUvarint(tmp[:], uint64(v.Get()))
	o.Write(tmp[:len])
}

type PersistentMap struct {
	Vars map[string]PersistentVar
	Data map[string]string
	*flag.FlagSet
}

func NewPersistentMap(name string) (result *PersistentMap) {
	result = &PersistentMap{
		Vars:    map[string]PersistentVar{},
		Data:    map[string]string{},
		FlagSet: flag.NewFlagSet(name, flag.ExitOnError),
	}
	result.FlagSet.Usage = func() {
		fmt.Printf("Program: %v@%v\n", MAIN_MODULEBASE, MAIN_MODULEVER)
		fmt.Printf("%v%X%v\n", ANSI_FG1_BLACK, Seed, ANSI_RESET)
		fmt.Println()
		fmt.Println("Usage:")
		fmt.Println(" ", ANSI_FG0_YELLOW, MAIN_MODULEPATH.Basename, ANSI_FG1_GREEN, "< COMMAND >", ANSI_FG1_BLUE, "OPTIONS...", ANSI_RESET)
		fmt.Println()
		fmt.Println("Available commands:")

		commandNames := AllCommands.Keys()
		sort.Strings(commandNames)
		for _, name := range commandNames {
			cmd, _ := AllCommands.Get(name)
			fmt.Println(" ", ANSI_FG1_GREEN, name, ANSI_RESET)
			fmt.Println("\t", cmd().Info().Usage)
		}

		var flagsShort []*flag.Flag
		var flagsLong []*flag.Flag

		result.FlagSet.VisitAll(func(f *flag.Flag) {
			if len(f.Name) > 1 {
				flagsLong = append(flagsLong, f)
			} else {
				flagsShort = append(flagsShort, f)
			}
		})

		printFlags := func(flags ...*flag.Flag) {
			for _, f := range flags {
				name, usage := flag.UnquoteUsage(f)
				name = "value"
				switch f.Value.(type) {
				case BoolFlag:
					name = ""
				case *IntVar:
					name = "int"
				case *Filename:
					name = "file"
				case *Directory:
					name = "dir"
				}
				if len(f.Name) > 1 {
					desc := fmt.Sprintf("%v-%v%v%v "+name+"%v", ANSI_FG0_YELLOW, ANSI_FG1_BLUE, f.Name, ANSI_FG1_BLACK, ANSI_RESET)
					fmt.Printf("  %-60v%v -> %v%v%v\n", desc, ANSI_FG1_BLACK, ANSI_FG0_CYAN, f.Value, ANSI_RESET)
				} else {
					fmt.Printf("  %v-%v%v "+name, ANSI_FG0_YELLOW, f.Name, ANSI_RESET)
				}
				fmt.Println("\t", usage)
			}
		}

		fmt.Println()
		fmt.Println("Command options:")
		printFlags(flagsLong...)

		fmt.Println()
		fmt.Println("Global options:")
		printFlags(flagsShort...)

		fmt.Println()
	}
	return result
}
func (pmp *PersistentMap) Usage() {
	pmp.FlagSet.Usage()
}
func (pmp *PersistentMap) Parse(args []string, parsables ...ParsableFlags) {
	for _, x := range parsables {
		LogTrace("parse <%v>", x)
		x.InitFlags(pmp)
	}
	for k, v := range pmp.Vars {
		pmp.LoadData(k, v)
	}
	if err := pmp.FlagSet.Parse(args); err != nil {
		LogPanic("command: %v", err)
	}
	for _, x := range parsables {
		LogTrace("apply vars <%v>", PrettyPrint(x))
		x.ApplyVars(pmp)
	}
	for k, v := range pmp.Vars {
		pmp.StoreData(k, v)
	}
}
func (pmp *PersistentMap) Persistent(value PersistentVar, name, usage string) {
	LogDebug("new persistent <%s> = %v", name, value)
	pmp.Var(value, name, usage)
	pmp.Vars[name] = value
}
func (pmp *PersistentMap) Var(value PersistentVar, name, usage string) {
	pmp.FlagSet.Var(value, name, usage)
}
func (pmp *PersistentMap) BoolVar(value *bool, name, usage string) {
	pmp.FlagSet.BoolVar(value, name, *value, usage)
}
func (pmp *PersistentMap) IntVar(value *int, name, usage string) {
	pmp.FlagSet.IntVar(value, name, *value, usage)
}
func (pmp *PersistentMap) LoadData(key string, dst PersistentVar) error {
	if str, ok := pmp.Data[key]; ok {
		LogDebug("load persistent <%s> = %v", key, str)
		return dst.Set(str)
	} else {
		err := fmt.Errorf("key '%s' not found", key)
		LogError("load(%v): %v", key, err)
		return err
	}
}
func (pmp *PersistentMap) StoreData(key string, dst interface{}) {
	LogDebug("store persistent <%s> = %v", key, dst)
	pmp.Data[key] = fmt.Sprint(dst)
}
func (pmp *PersistentMap) Serialize(dst io.Writer) error {
	if err := JsonSerialize(&pmp.Data, dst); err == nil {
		LogVerbose("saved %d persistent vars from config to disk", len(pmp.Data))
		return nil
	} else {
		return fmt.Errorf("failed to serialize config: %v", err)
	}
}
func (pmp *PersistentMap) Deserialize(src io.Reader) error {
	if err := JsonDeserialize(&pmp.Data, src); err == nil {
		LogVerbose("loaded %d persistent vars from disk to config", len(pmp.Data))
		return nil
	} else {
		return fmt.Errorf("failed to deserialize config: %v", err)
	}
}
