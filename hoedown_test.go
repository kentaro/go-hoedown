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
				Expect(buffer.String()).To(Equal, "<h1 id=\"toc_0\">Hoedown</h1>\n")
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
				Expect(buffer.String()).To(Equal, "<h1 id=\"toc_0\">あいう</h1>\n")
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

	Describe(t, "hoedown.MarkdownTOC", func() {
		src := []byte(`
# 1
## 1.1
### 1.1.1
## 1.2
### 1.2.1
# 2
## 2.2`)

		Context("when some a toc passed", func() {
			hoedown := NewHoedown(map[string]uint{})
			buffer := bytes.NewBuffer([]byte{})
			hoedown.MarkdownTOC(buffer, src)

			It("should render the toc as a list", func() {
				Expect(buffer.String()).To(Equal, `<ul>
<li>
<a href="#toc_0">1</a>
<ul>
<li>
<a href="#toc_1">1.1</a>
<ul>
<li>
<a href="#toc_2">1.1.1</a>
</li>
</ul>
</li>
<li>
<a href="#toc_3">1.2</a>
<ul>
<li>
<a href="#toc_4">1.2.1</a>
</li>
</ul>
</li>
</ul>
</li>
<li>
<a href="#toc_5">2</a>
<ul>
<li>
<a href="#toc_6">2.2</a>
</li>
</ul>
</li>
</ul>
`)
			})
		})
	})
}
