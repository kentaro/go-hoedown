package hoedown

import (
	"bytes"
	. "github.com/r7kamura/gospel"
	"testing"
)

func TestHowdown(t *testing.T) {
	Describe(t, "hoedown.Render", func() {
		hoedown := NewHoedown(0, 0, 0)
		buffer  := bytes.NewBuffer([]byte{})
		hoedown.Write(buffer, []byte("# Hoedown"))

		It("should render HTML from markdown string", func() {
			Expect(buffer.String()).To(Equal, "<h1>Hoedown</h1>\n")
		})
	})
}
