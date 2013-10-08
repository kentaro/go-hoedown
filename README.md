# go-hoedown

This is Go-bindings for [Hoedown](https://github.com/hoedown/hoedown) Markdown parser.

## USAGE

```go
package main

import (
        "github.com/kentaro/go-hoedown"
        "os"
)

hoedown := NewHoedown(map[string]uint{
        "extensions":  hoedown.EXT_NO_INTRA_EMPHASIS | hoedown.EXT_AUTOLINK,
        "renderModes": hoedown.HTML_USE_XHTML | HTML_ESCAPE,
        "maxNesting":  16,
})
hoedown.RenderHTML(os.Stdout, []byte("# Hoedown"))
```

### Extensions

  * EXT_NO_INTRA_EMPHASIS
  * EXT_TABLES
  * EXT_FENCED_CODE
  * EXT_AUTOLINK
  * EXT_STRIKETHROUGH
  * EXT_UNDERLINE
  * EXT_SPACE_HEADERS
  * EXT_SUPERSCRIPT
  * EXT_LAX_SPACING
  * EXT_DISABLE_INDENTED_CODE
  * EXT_HIGHLIGHT
  * EXT_FOOTNOTES
  * EXT_QUOTE

### Render Modes

  * HTML_SKIP_HTML
  * HTML_SKIP_STYLE
  * HTML_SKIP_IMAGES
  * HTML_SKIP_LINKS
  * HTML_EXPAND_TABS
  * HTML_SAFELINK
  * HTML_TOC
  * HTML_HARD_WRAP
  * HTML_USE_XHTML
  * HTML_ESCAPE
  * HTML_PRETTIFY

## See Also

  * [Hoedown](https://github.com/hoedown/hoedown)
  * [Goskirt](https://github.com/madari/goskirt)
    * Go-bindings for Sundown Markdown parser

## Author

  * [Kentaro Kuribayashi](http://kentarok.org/)

## License

  * MIT http://kentaro.mit-license.org/

