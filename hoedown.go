package hoedown

// #cgo CFLAGS: -c -g -O3 -Wall -Wextra -Wno-unused-parameter
// #include "hoedown.h"
import "C"
import "unsafe"

type Hoedown struct {

}

func NewHoedown() *Hoedown {
    return &Hoedown{}
}

func (self *Hoedown)Render(src string) string {
	c_str := C.CString(src)
	defer C.free(unsafe.Pointer(c_str))
	c_dst := C.markdown_render(c_str)
	return C.GoStringN(c_dst, C.int(C.strlen(c_dst)))
}
