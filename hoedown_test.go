package hoedown

import (
	. "github.com/r7kamura/gospel"
	"testing"
)

func TestHowdown(t *testing.T) {
	Describe(t, "Markdown", func() {
		It("evaluates actual == expected", func() {
			Expect(Markdown("# Hoedown")).To(Equal, "<h1>Hoedown</h1>\n")
		})
	})
}
