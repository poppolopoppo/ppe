package cluster

import (
	. "build/utils"

	"runtime"

	"github.com/shirou/gopsutil/cpu"
	"github.com/shirou/gopsutil/mem"
)

/***************************************
 * Peer Hardware
 ***************************************/

type PeerHardware struct {
	Arch          string
	Family        string
	Vendor        string
	Cores         int32
	Threads       int32
	MaxClock      int32
	CacheSize     int32
	VirtualMemory uint64
}

func CurrentPeerHardware() (hw PeerHardware, err error) {
	hw.Arch = runtime.GOARCH

	var cpuInfos []cpu.InfoStat
	if cpuInfos, err = cpu.Info(); err != nil {
		return
	}

	mainCpu := cpuInfos[0]

	hw.Vendor = mainCpu.VendorID
	hw.Family = mainCpu.Family
	hw.Cores = mainCpu.Cores
	hw.MaxClock = int32(mainCpu.Mhz)
	hw.CacheSize = mainCpu.CacheSize

	var numberOfLogicalProcessors int
	if numberOfLogicalProcessors, err = cpu.Counts(true); err != nil {
		return
	}

	hw.Threads = int32(numberOfLogicalProcessors)

	var vm *mem.VirtualMemoryStat
	if vm, err = mem.VirtualMemory(); err != nil {
		return
	}

	hw.VirtualMemory = vm.Total
	return
}
func (x *PeerHardware) String() string {
	return PrettyPrint(x)
}
