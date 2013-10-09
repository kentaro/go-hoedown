package hoedown

import (
	"bytes"
	. "github.com/r7kamura/gospel"
	"testing"
)

func TestHowdown(t *testing.T) {
	Describe(t, "hoedown.Render", func() {
		hoedown := NewHoedown(map[string]uint{
			"extensions":  0,
			"renderModes": 0,
			"maxNesting":  0,
		})
		buffer := bytes.NewBuffer([]byte{})
		hoedown.Markdown(buffer, []byte("# Hoedown"))

		It("should render HTML from markdown string", func() {
			Expect(buffer.String()).To(Equal, "<h1>Hoedown</h1>\n")
		})
	})
}
