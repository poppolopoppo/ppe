package utils

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"strings"
)

type JsonMap map[string]interface{}

/***************************************
 * JSON
 ***************************************/

func MarshalJSON[T fmt.Stringer](x T) ([]byte, error) {
	return json.Marshal(x.String())
}
func UnmarshalJSON[T flag.Value](x T, data []byte) error {
	var str string
	if err := json.Unmarshal(data, &str); err != nil {
		return err
	}
	return x.Set(str)
}

func JsonSerialize(x interface{}, dst io.Writer) error {
	str := strings.Builder{}
	encoder := json.NewEncoder(&str)
	var err error
	if err = encoder.Encode(x); err == nil {
		var pretty bytes.Buffer
		if err = json.Indent(&pretty, []byte(str.String()), "", "\t"); err == nil {
			dst.Write(pretty.Bytes())
			return nil
		}
	}
	return err
}
func JsonDeserialize(x interface{}, src io.Reader) error {
	decoder := json.NewDecoder(src)
	if err := decoder.Decode(x); err == nil {
		return nil
	} else {
		return err
	}
}

func PrettyPrint(x interface{}) string {
	str := strings.Builder{}
	encoder := json.NewEncoder(&str)
	var err error
	if err = encoder.Encode(x); err == nil {
		var pretty bytes.Buffer
		if err = json.Indent(&pretty, []byte(str.String()), "", "\t"); err == nil {
			return pretty.String()
		}
	}
	return fmt.Sprint(err)
}
