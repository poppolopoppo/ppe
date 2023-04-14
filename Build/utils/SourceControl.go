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

type SourceControlProvider interface {
	GetModifiedFiles() (FileSet, error)
	GetStatus(*SourceControlStatus) error
}

/***************************************
 * Source Control Status
 ***************************************/

type SourceControlStatus struct {
	Branch    string
	Revision  string
	Timestamp time.Time
}

func (x *SourceControlStatus) Alias() BuildAlias {
	return MakeBuildAlias("SourceControl", "Status")
}
func (x *SourceControlStatus) Build(BuildContext) (err error) {
	err = GetSourceControlProvider().GetStatus(x)
	if err == nil {
		LogVerbose("source-control: branch=%s, revision=%s, timestamp=%s", x.Branch, x.Revision, x.Timestamp)
	}
	return
}
func (x *SourceControlStatus) Serialize(ar Archive) {
	ar.String(&x.Branch)
	ar.String(&x.Revision)
	ar.Time(&x.Timestamp)
}

func BuildSourceControlStatus() BuildFactoryTyped[*SourceControlStatus] {
	return func(bi BuildInitializer) (*SourceControlStatus, error) {
		return &SourceControlStatus{}, nil
	}
}

/***************************************
 * Source Control Modified Files
 ***************************************/

type SourceControlModifiedFiles struct {
	OutputFile    Filename
	ModifiedFiles FileSet
}

func (x *SourceControlModifiedFiles) Alias() BuildAlias {
	return MakeBuildAlias("SourceControl", "ModifiedFiles", x.OutputFile.String())
}
func (x *SourceControlModifiedFiles) Build(bc BuildContext) (err error) {
	x.ModifiedFiles, err = GetSourceControlProvider().GetModifiedFiles()
	if err != nil {
		return
	}

	err = UFS.SafeCreate(x.OutputFile, func(w io.Writer) error {
		for _, file := range x.ModifiedFiles {
			fmt.Fprintln(w, filepath.Clean(file.String()))
		}
		return nil
	})

	LogVerbose("source-control: updated %q (%d files)", x.OutputFile, len(x.ModifiedFiles))
	//bc.OutputFile(x.OutputFile)
	return
}
func (x *SourceControlModifiedFiles) Serialize(ar Archive) {
	ar.Serializable(&x.OutputFile)
	ar.Serializable(&x.ModifiedFiles)
}

func BuildSourceControlModifiedFiles() BuildFactoryTyped[*SourceControlModifiedFiles] {
	return func(bi BuildInitializer) (*SourceControlModifiedFiles, error) {
		return &SourceControlModifiedFiles{
			OutputFile: UFS.Saved.File(".modified_files_list.txt"),
		}, nil
	}
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
	LogVeryVerbose("git: %v", MakeStringer(func() string {
		return strings.Join(args, " ")
	}))

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
	for {
		advance, token, err := bufio.ScanLines(status, true)
		if err != nil {
			return FileSet{}, err
		}
		if advance == 0 {
			break
		}
		if advance <= len(status) {
			status = status[advance:]
		}
		if len(token) == 0 {
			continue
		}
		line := string(token)
		if strings.HasPrefix(line, "A ") || strings.HasPrefix(line, " M") || strings.HasPrefix(line, "AM") || strings.HasPrefix(line, "??") {
			file := UFS.Root.AbsoluteFile(strings.TrimSpace(line[3:]))
			LogVeryVerbose("git: %q was modified", file)
			fileset.Append(file)
		}
	}

	if err := reader.Err(); err == nil {
		LogVeryVerbose("git: found %d modified files", len(fileset))
		return fileset, nil
	} else {
		return fileset, err
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
		LogVerbose("source-control: found Git source control in %q", gitDir)
		return &GitSourceControl{gitDir}
	}
	return &DummySourceControl{}
})
