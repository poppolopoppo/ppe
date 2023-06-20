package build

import (
	"build/cluster"
	"build/hal"
	"build/utils"
	"os"
	"time"
)

func WithBuild2(prefix string, scope func(*utils.CommandEnvT) error) error {
	startedAt := time.Now()

	defer utils.StartTrace()()
	defer utils.PurgePinnedLogs()

	env := utils.InitCommandEnv(prefix, os.Args[1:], startedAt)

	defer utils.StartProfiling()()

	hal.InitHAL()
	utils.InitUtils()

	err := scope(env)
	if err != nil {
		utils.LogError(utils.LogCommand, "%v", err)
	}
	return err
}

var CommandWork = utils.NewCommand(
	"Worker", "work",
	"listen for incomming requests and execute distributed tasks",
	utils.OptionCommandParsableAccessor("PeerFlags", "peer connection settings", cluster.GetPeerFlags),
	utils.OptionCommandParsableAccessor("WorkerFlags", "local worker settings", cluster.GetWorkerFlags),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		peers := cluster.NewCluster()
		worker, _, err := peers.StartWorker()
		if err == nil {
			err = worker.Close()
		}
		return err
	}))

func main() {
	WithBuild2("worker", func(env *utils.CommandEnvT) error {
		env.LoadConfig()

		err := env.Run()
		if err != nil {
			utils.LogError(utils.LogCommand, "%v", err)
		}

		env.SaveConfig()
		return err
	})
}
