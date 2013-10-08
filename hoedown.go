package hoedown

// #cgo CFLAGS: -c -g -O3 -Wall -Wextra -Wno-unused-parameter
// #include "markdown.h"
// #include "html.h"
// #include <stdio.h>
import "C"
import "io"
import "unsafe"

const (
	output_uint = 64
)

type Hoedown struct {
}

func NewHoedown() *Hoedown {
	return &Hoedown{}
}

func (self *Hoedown) Write(writer io.Writer, src []byte) (n int, err error) {
	var callbacks C.struct_hoedown_callbacks
	var options C.struct_hoedown_html_renderopt

	ob := C.hoedown_buffer_new(output_uint)
    C.hoedown_html_renderer(&callbacks, &options, 0, 0);
	markdown := C.hoedown_markdown_new(0, 16, &callbacks, unsafe.Pointer(&options))

	C.hoedown_markdown_render(ob, (*C.uint8_t)(unsafe.Pointer(&src[0])), C.size_t(len(src)), markdown)
	C.hoedown_markdown_free(markdown)

	n, err = writer.Write(C.GoBytes(unsafe.Pointer(ob.data), C.int(ob.size)))
	C.hoedown_buffer_free(ob)

	return n, err
}
