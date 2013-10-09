package hoedown

import (
	"bytes"
	. "github.com/r7kamura/gospel"
	"testing"
)

func TestHowdown(t *testing.T) {
	Describe(t, "hoedown.Markdown", func() {
		Context("when a headline passed", func() {
			hoedown := NewHoedown(map[string]uint{
				"extensions":  0,
				"renderModes": 0,
			})
			buffer := bytes.NewBuffer([]byte{})
			hoedown.Markdown(buffer, []byte("# Hoedown"))

			It("should render HTML from markdown string", func() {
				Expect(buffer.String()).To(Equal, "<h1>Hoedown</h1>\n")
			})
		})

		Context("when UTF-8 strings are passed", func() {
			hoedown := NewHoedown(map[string]uint{
				"extensions":  0,
				"renderModes": 0,
			})
			buffer := bytes.NewBuffer([]byte{})
			hoedown.Markdown(buffer, []byte("# あいう"))

			It("should render HTML from markdown string", func() {
				Expect(buffer.String()).To(Equal, "<h1>あいう</h1>\n")
			})
		})

		Context("when a extension is passed", func() {
			hoedown := NewHoedown(map[string]uint{
				"extensions":  EXT_AUTOLINK,
				"renderModes": 0,
			})
			buffer := bytes.NewBuffer([]byte{})
			hoedown.Markdown(buffer, []byte("http://example.com/"))

			It("should render HTML from markdown string", func() {
				Expect(buffer.String()).To(Equal, "<p><a href=\"http://example.com/\">http://example.com/</a></p>\n")
			})
		})

		Context("when a render mode is passed", func() {
			hoedown := NewHoedown(map[string]uint{
				"extensions":  0,
				"renderModes": HTML_USE_XHTML,
			})
			buffer := bytes.NewBuffer([]byte{})
			hoedown.Markdown(buffer, []byte("![test](http://example.com/test.jpg)"))

			It("should render HTML from markdown string", func() {
				Expect(buffer.String()).To(Equal, "<p><img src=\"http://example.com/test.jpg\" alt=\"test\"/></p>\n")
			})
		})
	})
}
