#include "markdown.h"
#include "html.h"
#include <string.h>

const char* markdown_render(char* src)
{
  struct hoedown_buffer* ob;
  const char* dst;
  struct hoedown_callbacks callbacks;
  struct hoedown_html_renderopt options;
  struct hoedown_markdown *markdown;

  ob = hoedown_buffer_new(64);
  hoedown_html_renderer(&callbacks, &options, 0, 0);
  markdown = hoedown_markdown_new(0, 16, &callbacks, &options);
  hoedown_markdown_render(ob, (uint8_t *)src, strlen(src), markdown);
  hoedown_markdown_free(markdown);

  dst = hoedown_buffer_cstr(ob);
  hoedown_buffer_free(ob);

  return dst;
}
