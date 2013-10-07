package hoedown

// #cgo LDFLAGS: -L/Users/kentaro/dev/github/hoedown/hoedown -lhoedown
// #include "hoedown.h"
import "C"
import "unsafe"

func Markdown(src string) string {
	c_str := C.CString(src)
	defer C.free(unsafe.Pointer(c_str))
	c_dst := C.markdown_render(c_str)
	defer C.free(unsafe.Pointer(c_dst))
	return C.GoStringN(c_dst, 1024) // XXX
}
