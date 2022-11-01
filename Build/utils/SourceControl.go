package utils

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

/***************************************
 * Source control
 ***************************************/

type SourceControlStatus struct {
	Branch    string
	Revision  string
	Timestamp time.Time
}

func (x *SourceControlStatus) Alias() BuildAlias {
	return MakeBuildAlias("SourceControl", "Status")
}
func (x *SourceControlStatus) Build(BuildContext) (BuildStamp, error) {
	if err := GetSourceControlProvider().GetStatus(x); err == nil {
		return MakeTimedBuildStamp(x.Timestamp, x)
	} else {
		return BuildStamp{}, err
	}
}
func (x *SourceControlStatus) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.Branch)
	o.WriteString(x.Revision)
	raw, _ := x.Timestamp.MarshalBinary()
	o.Write(raw)
}

type SourceControlProvider interface {
	GetModifiedFiles() (FileSet, error)
	GetStatus(*SourceControlStatus) error
}

/***************************************
 * Dummy source control
 ***************************************/

type DummySourceControl struct{}

func (x DummySourceControl) GetModifiedFiles() (FileSet, error) {
	return NewFileSet(), nil
}
func (x DummySourceControl) GetStatus(status *SourceControlStatus) error {
	status.Branch = "Dummy"
	status.Revision = "Unknown"
	status.Timestamp = time.Now()
	return nil
}

/***************************************
 * Git source control
 ***************************************/

type GitSourceControl struct {
	GitDir Directory
}

func (git GitSourceControl) Command(name string, args ...string) ([]byte, error) {
	args = append([]string{"--no-optional-locks", name}, args...)
	LogVeryVerbose("git: %v", strings.Join(args, " "))

	proc := exec.Command("git", args...)
	proc.Env = os.Environ()
	proc.Dir = UFS.Root.String()

	output, err := proc.Output()
	if err != nil {
		LogError("git: %v -> %v : %v", strings.Join(args, " "), err, output)
	}

	return output, err
}
func (git GitSourceControl) GetModifiedFiles() (FileSet, error) {
	fileset := NewFileSet()
	status, err := git.Command("status", "-s", "--porcelain=v1", UFS.Source.Relative(UFS.Root))
	if err != nil {
		return fileset, err
	}

	reader := bufio.NewScanner(bytes.NewReader(status))
	for reader.Scan() {
		line := reader.Text()
		if len(line) > 0 {
			if line[0:1] == "A" || line[1:2] == "M" || line[0:2] == "AM" || line[0:2] == "??" {
				file := UFS.Root.AbsoluteFile(strings.TrimSpace(line[3:]))
				if file.Ext() == ".cpp" {
					fileset.Append(file)
				}
			}
		}
	}

	if err := reader.Err(); err != nil {
		return fileset, err
	} else {
		return fileset, nil
	}
}
func (git GitSourceControl) GetStatus(status *SourceControlStatus) error {
	if outp, err := git.Command("log", "-1", "--format=\"%H, %ct, %D\""); err == nil {
		line := strings.TrimSpace(string(outp))
		line = strings.TrimPrefix(line, "\"")
		line = strings.TrimSuffix(line, "\"")
		log := strings.SplitN(line, ",", 4)
		status.Revision = strings.TrimSpace(log[0])
		branchInfo := strings.Split(log[len(log)-1], "->")
		status.Branch = strings.TrimSpace(branchInfo[len(branchInfo)-1])
		timestamp := strings.TrimSpace(log[1])
		if unitT, err := strconv.ParseInt(timestamp, 10, 64); err == nil {
			status.Timestamp = time.Unix(unitT, 0)
			return nil
		} else {
			return err
		}
	} else {
		return err
	}
}

var GetSourceControlProvider = Memoize(func() SourceControlProvider {
	if gitDir := UFS.Root.Folder(".git"); gitDir.Exists() {
		return &GitSourceControl{gitDir}
	}
	return &DummySourceControl{}
})

var SourceControlBuilder = MakeBuildable(func(BuildInit) *SourceControlStatus {
	return &SourceControlStatus{}
})

var GenerateSourceControlModifiedFiles = Memoize(func() Future[Filename] {
	return MakeFuture(func() (Filename, error) {
		output := UFS.Saved.File(".modified_files_list.txt")
		err := UFS.Create(output, func(w io.Writer) error {
			if modifiedFiles, err := GetSourceControlProvider().GetModifiedFiles(); err == nil {
				for _, file := range modifiedFiles {
					fmt.Fprintln(w, filepath.Clean(file.String()))
				}
				LogInfo("Found %d modified files with source control", len(modifiedFiles))
				return nil
			} else {
				return err
			}
		})
		return output, err
	})
})
