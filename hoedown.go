package hoedown

// #cgo CFLAGS: -c -g -O3 -Wall -Wextra -Wno-unused-parameter
// #include "markdown.h"
// #include "html.h"
import "C"

import (
	"errors"
	"io"
	"unsafe"
)

// Hoedown version
const (
	VERSION = C.HOEDOWN_VERSION
)

// extensions
const (
	EXT_NO_INTRA_EMPHASIS     = C.HOEDOWN_EXT_NO_INTRA_EMPHASIS
	EXT_TABLES                = C.HOEDOWN_EXT_TABLES
	EXT_FENCED_CODE           = C.HOEDOWN_EXT_FENCED_CODE
	EXT_AUTOLINK              = C.HOEDOWN_EXT_AUTOLINK
	EXT_STRIKETHROUGH         = C.HOEDOWN_EXT_STRIKETHROUGH
	EXT_UNDERLINE             = C.HOEDOWN_EXT_UNDERLINE
	EXT_SPACE_HEADERS         = C.HOEDOWN_EXT_SPACE_HEADERS
	EXT_SUPERSCRIPT           = C.HOEDOWN_EXT_SUPERSCRIPT
	EXT_LAX_SPACING           = C.HOEDOWN_EXT_LAX_SPACING
	EXT_DISABLE_INDENTED_CODE = C.HOEDOWN_EXT_DISABLE_INDENTED_CODE
	EXT_HIGHLIGHT             = C.HOEDOWN_EXT_HIGHLIGHT
	EXT_FOOTNOTES             = C.HOEDOWN_EXT_FOOTNOTES
	EXT_QUOTE                 = C.HOEDOWN_EXT_QUOTE
)

// HTML render modes
const (
	HTML_SKIP_HTML   = C.HOEDOWN_HTML_SKIP_HTML
	HTML_SKIP_STYLE  = C.HOEDOWN_HTML_SKIP_STYLE
	HTML_SKIP_IMAGES = C.HOEDOWN_HTML_SKIP_IMAGES
	HTML_SKIP_LINKS  = C.HOEDOWN_HTML_SKIP_LINKS
	HTML_EXPAND_TABS = C.HOEDOWN_HTML_EXPAND_TABS
	HTML_SAFELINK    = C.HOEDOWN_HTML_SAFELINK
	HTML_TOC         = C.HOEDOWN_HTML_TOC
	HTML_HARD_WRAP   = C.HOEDOWN_HTML_HARD_WRAP
	HTML_USE_XHTML   = C.HOEDOWN_HTML_USE_XHTML
	HTML_ESCAPE      = C.HOEDOWN_HTML_ESCAPE
	HTML_PRETTIFY    = C.HOEDOWN_HTML_PRETTIFY
)

const (
	htmlRenderer = iota
	htmlTocRenderer
)

const (
	buffSize = 64
)

type Hoedown struct {
	extensions      uint
	renderModes     uint
	maxNesting      uint
	tocNestingLevel uint
	callbacks       C.struct_hoedown_callbacks
	options         C.struct_hoedown_html_renderopt
}

func NewHoedown(options map[string]uint) *Hoedown {
	if options["maxNesting"] == 0 {
		options["maxNesting"] = 16
	}
	if options["tocNestingLevel"] == 0 {
		options["tocNestingLevel"] = 99
	}

	return &Hoedown{
		extensions:      options["extensions"],
		renderModes:     options["renderModes"],
		maxNesting:      options["maxNesting"],
		tocNestingLevel: options["tocNestingLevel"],
	}
}

func (self *Hoedown) Markdown(writer io.Writer, src []byte) (n int, err error) {
	return self.render(htmlRenderer, writer, src)
}

func (self *Hoedown) MarkdownTOC(writer io.Writer, src []byte) (n int, err error) {
	return self.render(htmlTocRenderer, writer, src)
}

func (self *Hoedown) render(renderer int, writer io.Writer, src []byte) (n int, err error) {
	ob := C.hoedown_buffer_new(buffSize)

	switch renderer {
	case htmlRenderer:
		C.hoedown_html_renderer(&self.callbacks, &self.options, C.uint(self.renderModes), C.int(self.tocNestingLevel))
	case htmlTocRenderer:
		C.hoedown_html_toc_renderer(&self.callbacks, &self.options, C.int(self.tocNestingLevel))
	default:
		return 0, errors.New("Unsupported renderer")
	}

	markdown := C.hoedown_markdown_new(C.uint(self.extensions), C.size_t(self.maxNesting), &self.callbacks, unsafe.Pointer(&self.options))

	C.hoedown_markdown_render(ob, (*C.uint8_t)(unsafe.Pointer(&src[0])), C.size_t(len(src)), markdown)
	C.hoedown_markdown_free(markdown)

	n, err = writer.Write(C.GoBytes(unsafe.Pointer(ob.data), C.int(ob.size)))
	C.hoedown_buffer_free(ob)

	return n, err
}
