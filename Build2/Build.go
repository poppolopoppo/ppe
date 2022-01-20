package build

import (
	"build/cmd"
	"build/compile"
	"build/hal"
	"build/utils"
	"math/rand"
	"time"
)

func initInternals() {
	hal.InitHAL()
	utils.InitUtils()
	compile.InitCompile()
	cmd.InitCmd()
}

func test(id int) <-chan *int {
	split := 200 + rand.Intn(800)
	channel := make(chan *int)
	go func() {
		defer close(channel)

		var off int
		if id != 3 {
			off = 1
		}

		pin := utils.LogProgress(0, split*off, "writer [%d]", id)
		defer pin.Close()

		for i := 0; i < split; i += 1 {
			pin.Inc()
			time.Sleep((time.Second * time.Duration(rand.Intn(30))) / time.Duration(2000))
		}

		x := 32
		channel <- &x
	}()
	return channel
}

func LaunchCommand(prefix string, rootFile utils.Filename, args []string) {
	defer utils.StartProfiling()()
	defer utils.FlushLog()

	env := utils.InitCommandEnv(prefix, rootFile)
	initInternals()

	// channel := make(chan *int)
	// go func() {
	// 	defer close(channel)

	// 	var deferred []<-chan *int
	// 	for i := 0; i < 10; i += 1 {
	// 		deferred = append(deferred, test(i))
	// 	}

	// 	for {
	// 		found := false
	// 		for _, x := range deferred {
	// 			if it := <-x; it != nil {
	// 				found = true
	// 				channel <- it
	// 			}
	// 		}
	// 		if !found {
	// 			break
	// 		}
	// 	}
	// }()

	env.Load()
	defer env.Save()

	env.Init(args)

	env.Run()

	// for {
	// 	if x := <-channel; x != nil {
	// 		utils.LogInfo("consume[%v] %v", *x, x)
	// 	} else {
	// 		break
	// 	}
	// }
}
