package hoedown

import (
	. "github.com/r7kamura/gospel"
	"testing"
)

func TestHowdown(t *testing.T) {
	Describe(t, "NewHoedown", func() {
		hoedown := NewHoedown()

		It("should return a new instance of Hoedown", func() {
			Expect(hoedown).To(Exist)
		})
	})

	Describe(t, "hoedown.Markdown", func() {
		hoedown := NewHoedown()

		It("should render HTML from markdown string", func() {
			Expect(hoedown.Markdown("# Hoedown")).To(Equal, "<h1>Hoedown</h1>\n")
		})
	})
}
