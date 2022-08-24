package utils

import (
	"bytes"
	"encoding/binary"
	"flag"
	"fmt"
	"io"
	"sort"
	"strconv"
	"strings"
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
func (v BoolVar) Equals(o BoolVar) bool {
	return (v == o)
}
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

func (v IntVar) Get() int             { return int(v) }
func (v IntVar) Equals(o IntVar) bool { return (v == o) }
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
	Name  string
	Vars  map[string]PersistentVar
	Data  map[string]string
	flags *flag.FlagSet
}

func NewPersistentMap(name string) (result *PersistentMap) {
	result = &PersistentMap{
		Name: name,
		Vars: map[string]PersistentVar{},
		Data: map[string]string{},
	}
	return result
}
func (pmp *PersistentMap) resetFlagSet() {
	LogDebug("reset flag set")
	pmp.flags = flag.NewFlagSet(pmp.Name, flag.ExitOnError)
	pmp.flags.Usage = func() {
		fmt.Printf("Program: %v@%v\n", MAIN_MODULEBASE, MAIN_MODULEVER)
		fmt.Printf("%v%X%v\n", ANSI_FG1_BLACK, Seed.Slice(), ANSI_RESET)
		fmt.Println()
		fmt.Println("Usage:")
		fmt.Println(" ", ANSI_FG0_YELLOW, MAIN_MODULEPATH.Basename, ANSI_FG1_GREEN, "< COMMAND >", ANSI_FG1_BLUE, "[OPTIONS]*", ANSI_FG0_WHITE, "[ARGS]*", ANSI_RESET)
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

		pmp.flags.VisitAll(func(f *flag.Flag) {
			if len(f.Name) > 1 {
				flagsLong = append(flagsLong, f)
			} else {
				flagsShort = append(flagsShort, f)
			}
		})

		printFlags := func(flags ...*flag.Flag) {
			for _, f := range flags {
				_, usage := flag.UnquoteUsage(f)
				name := "value"
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
}
func (pmp *PersistentMap) Usage() {
	pmp.flags.Usage()
}
func (pmp *PersistentMap) Parse(args []string, parsables ...ParsableFlags) []string {
	pmp.resetFlagSet()

	for _, x := range parsables {
		LogDebug("persistent: init flags for <%v>", Inspect(x))
		x.InitFlags(pmp)
	}
	for k, v := range pmp.Vars {
		pmp.LoadData(k, v)
	}

	LogTrace("persistent: parsing flags for %v", Inspect(parsables...))

	var unparsedArgs []string
	if err := pmp.flags.Parse(args); err != nil {
		LogPanic("command: %v", err)
	} else if pmp.flags.NArg() > 0 {
		unparsedArgs = make([]string, pmp.flags.NArg())
		for i, x := range args[len(args)-pmp.flags.NArg():] {
			unparsedArgs[i] = x
		}
		LogTrace("provided arguments: %v", strings.Join(args, ", "))
	}

	for _, x := range parsables {
		LogDebug("persistent: apply vars for %v", Inspect(x))
		x.ApplyVars(pmp)
	}
	for k, v := range pmp.Vars {
		pmp.StoreData(k, v)
	}

	return unparsedArgs
}
func (pmp *PersistentMap) Persistent(value PersistentVar, name, usage string) {
	LogDebug("persistent: new instance <%s> = %v", name, value)
	pmp.Var(value, name, usage)
	pmp.Vars[name] = value
}
func (pmp *PersistentMap) Var(value PersistentVar, name, usage string) {
	pmp.flags.Var(value, name, usage)
}
func (pmp *PersistentMap) BoolVar(value *bool, name, usage string) {
	pmp.flags.BoolVar(value, name, *value, usage)
}
func (pmp *PersistentMap) IntVar(value *int, name, usage string) {
	pmp.flags.IntVar(value, name, *value, usage)
}
func (pmp *PersistentMap) LoadData(key string, dst PersistentVar) error {
	if str, ok := pmp.Data[key]; ok {
		LogDebug("load persistent <%s> = %v", key, str)
		return dst.Set(str)
	} else {
		err := fmt.Errorf("key '%s' not found", key)
		LogWarning("load(%v): %v", key, err)
		return err
	}
}
func (pmp *PersistentMap) StoreData(key string, dst interface{}) {
	LogDebug("persistent: store in <%s> = %v", key, dst)
	pmp.Data[key] = fmt.Sprint(dst)
}
func (pmp *PersistentMap) Serialize(dst io.Writer) error {
	if err := JsonSerialize(&pmp.Data, dst); err == nil {
		LogDebug("persistent: saved %d vars from config to disk", len(pmp.Data))
		return nil
	} else {
		return fmt.Errorf("failed to serialize config: %v", err)
	}
}
func (pmp *PersistentMap) Deserialize(src io.Reader) error {
	if err := JsonDeserialize(&pmp.Data, src); err == nil {
		LogVerbose("persistent: loaded %d vars from disk to config", len(pmp.Data))
		return nil
	} else {
		return fmt.Errorf("failed to deserialize config: %v", err)
	}
}
