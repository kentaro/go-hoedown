#include "html.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "escape.h"

#define USE_XHTML(opt) (opt->flags & HOEDOWN_HTML_USE_XHTML)

int
hoedown_html_is_tag(const uint8_t *tag_data, size_t tag_size, const char *tagname)
{
	size_t i;
	int closed = 0;

	if (tag_size < 3 || tag_data[0] != '<')
		return HOEDOWN_HTML_TAG_NONE;

	i = 1;

	if (tag_data[i] == '/') {
		closed = 1;
		i++;
	}

	for (; i < tag_size; ++i, ++tagname) {
		if (*tagname == 0)
			break;

		if (tag_data[i] != *tagname)
			return HOEDOWN_HTML_TAG_NONE;
	}

	if (i == tag_size)
		return HOEDOWN_HTML_TAG_NONE;

	if (isspace(tag_data[i]) || tag_data[i] == '>')
		return closed ? HOEDOWN_HTML_TAG_CLOSE : HOEDOWN_HTML_TAG_OPEN;

	return HOEDOWN_HTML_TAG_NONE;
}

static inline void escape_html(struct hoedown_buffer *ob, const uint8_t *source, size_t length)
{
	hoedown_escape_html(ob, source, length, 0);
}

static inline void escape_href(struct hoedown_buffer *ob, const uint8_t *source, size_t length)
{
	hoedown_escape_href(ob, source, length);
}

/********************
 * GENERIC RENDERER *
 ********************/
static int
rndr_autolink(struct hoedown_buffer *ob, const struct hoedown_buffer *link, enum hoedown_autolink type, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	if (!link || !link->size)
		return 0;

	if ((options->flags & HOEDOWN_HTML_SAFELINK) != 0 &&
		!hoedown_autolink_is_safe(link->data, link->size) &&
		type != HOEDOWN_AUTOLINK_EMAIL)
		return 0;

	BUFPUTSL(ob, "<a href=\"");
	if (type == HOEDOWN_AUTOLINK_EMAIL)
		BUFPUTSL(ob, "mailto:");
	escape_href(ob, link->data, link->size);

	if (options->link_attributes) {
		hoedown_buffer_putc(ob, '\"');
		options->link_attributes(ob, link, opaque);
		hoedown_buffer_putc(ob, '>');
	} else {
		BUFPUTSL(ob, "\">");
	}

	/*
	 * Pretty printing: if we get an email address as
	 * an actual URI, e.g. `mailto:foo@bar.com`, we don't
	 * want to print the `mailto:` prefix
	 */
	if (hoedown_buffer_prefix(link, "mailto:") == 0) {
		escape_html(ob, link->data + 7, link->size - 7);
	} else {
		escape_html(ob, link->data, link->size);
	}

	BUFPUTSL(ob, "</a>");

	return 1;
}

static void
rndr_blockcode(struct hoedown_buffer *ob, const struct hoedown_buffer *text, const struct hoedown_buffer *lang, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	if (ob->size) hoedown_buffer_putc(ob, '\n');

	if (lang && lang->size) {
		size_t i, cls = 0;
		if (options->flags & HOEDOWN_HTML_PRETTIFY) {
			BUFPUTSL(ob, "<pre><code class=\"prettyprint");
			cls++;
		} else {
			BUFPUTSL(ob, "<pre><code class=\"");
		}

		for (i = 0; i < lang->size; ++i, ++cls) {
			while (i < lang->size && isspace(lang->data[i]))
				i++;

			if (i < lang->size) {
				size_t org = i;
				while (i < lang->size && !isspace(lang->data[i]))
					i++;

				if (lang->data[org] == '.')
					org++;

				if (cls) hoedown_buffer_putc(ob, ' ');
				escape_html(ob, lang->data + org, i - org);
			}
		}

		BUFPUTSL(ob, "\">");
	} else if (options->flags & HOEDOWN_HTML_PRETTIFY) {
		BUFPUTSL(ob, "<pre><code class=\"prettyprint\">");
	} else {
		BUFPUTSL(ob, "<pre><code>");
	}

	if (text)
		escape_html(ob, text->data, text->size);

	BUFPUTSL(ob, "</code></pre>\n");
}

static void
rndr_blockquote(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	BUFPUTSL(ob, "<blockquote>\n");
	if (text) hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</blockquote>\n");
}

static int
rndr_codespan(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;
	if (options->flags & HOEDOWN_HTML_PRETTIFY)
		BUFPUTSL(ob, "<code class=\"prettyprint\">");
	else
		BUFPUTSL(ob, "<code>");
	if (text) escape_html(ob, text->data, text->size);
	BUFPUTSL(ob, "</code>");
	return 1;
}

static int
rndr_strikethrough(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size)
		return 0;

	BUFPUTSL(ob, "<del>");
	hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</del>");
	return 1;
}

static int
rndr_double_emphasis(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size)
		return 0;

	BUFPUTSL(ob, "<strong>");
	hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</strong>");

	return 1;
}

static int
rndr_emphasis(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size) return 0;
	BUFPUTSL(ob, "<em>");
	if (text) hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</em>");
	return 1;
}

static int
rndr_underline(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size)
		return 0;

	BUFPUTSL(ob, "<u>");
	hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</u>");

	return 1;
}

static int
rndr_highlight(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size)
		return 0;

	BUFPUTSL(ob, "<mark>");
	hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</mark>");

	return 1;
}

static int
rndr_quote(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size)
		return 0;

	BUFPUTSL(ob, "<q>");
	hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</q>");

	return 1;
}

static int
rndr_linebreak(struct hoedown_buffer *ob, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;
	hoedown_buffer_puts(ob, USE_XHTML(options) ? "<br/>\n" : "<br>\n");
	return 1;
}

static void
rndr_header(struct hoedown_buffer *ob, const struct hoedown_buffer *text, int level, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	if (ob->size)
		hoedown_buffer_putc(ob, '\n');

	if ((options->flags & HOEDOWN_HTML_TOC) && (level <= options->toc_data.nesting_level))
		hoedown_buffer_printf(ob, "<h%d id=\"toc_%d\">", level, options->toc_data.header_count++);
	else
		hoedown_buffer_printf(ob, "<h%d>", level);

	if (text) hoedown_buffer_put(ob, text->data, text->size);
	hoedown_buffer_printf(ob, "</h%d>\n", level);
}

static int
rndr_link(struct hoedown_buffer *ob, const struct hoedown_buffer *link, const struct hoedown_buffer *title, const struct hoedown_buffer *content, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	if (link != NULL && (options->flags & HOEDOWN_HTML_SAFELINK) != 0 && !hoedown_autolink_is_safe(link->data, link->size))
		return 0;

	BUFPUTSL(ob, "<a href=\"");

	if (link && link->size)
		escape_href(ob, link->data, link->size);

	if (title && title->size) {
		BUFPUTSL(ob, "\" title=\"");
		escape_html(ob, title->data, title->size);
	}

	if (options->link_attributes) {
		hoedown_buffer_putc(ob, '\"');
		options->link_attributes(ob, link, opaque);
		hoedown_buffer_putc(ob, '>');
	} else {
		BUFPUTSL(ob, "\">");
	}

	if (content && content->size) hoedown_buffer_put(ob, content->data, content->size);
	BUFPUTSL(ob, "</a>");
	return 1;
}

static void
rndr_list(struct hoedown_buffer *ob, const struct hoedown_buffer *text, int flags, void *opaque)
{
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	hoedown_buffer_put(ob, flags & HOEDOWN_LIST_ORDERED ? "<ol>\n" : "<ul>\n", 5);
	if (text) hoedown_buffer_put(ob, text->data, text->size);
	hoedown_buffer_put(ob, flags & HOEDOWN_LIST_ORDERED ? "</ol>\n" : "</ul>\n", 6);
}

static void
rndr_listitem(struct hoedown_buffer *ob, const struct hoedown_buffer *text, int flags, void *opaque)
{
	BUFPUTSL(ob, "<li>");
	if (text) {
		size_t size = text->size;
		while (size && text->data[size - 1] == '\n')
			size--;

		hoedown_buffer_put(ob, text->data, size);
	}
	BUFPUTSL(ob, "</li>\n");
}

static void
rndr_paragraph(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;
	size_t i = 0;

	if (ob->size) hoedown_buffer_putc(ob, '\n');

	if (!text || !text->size)
		return;

	while (i < text->size && isspace(text->data[i])) i++;

	if (i == text->size)
		return;

	BUFPUTSL(ob, "<p>");
	if (options->flags & HOEDOWN_HTML_HARD_WRAP) {
		size_t org;
		while (i < text->size) {
			org = i;
			while (i < text->size && text->data[i] != '\n')
				i++;

			if (i > org)
				hoedown_buffer_put(ob, text->data + org, i - org);

			/*
			 * do not insert a line break if this newline
			 * is the last character on the paragraph
			 */
			if (i >= text->size - 1)
				break;

			rndr_linebreak(ob, opaque);
			i++;
		}
	} else {
		hoedown_buffer_put(ob, &text->data[i], text->size - i);
	}
	BUFPUTSL(ob, "</p>\n");
}

static void
rndr_raw_block(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	size_t org, sz;
	if (!text) return;
	sz = text->size;
	while (sz > 0 && text->data[sz - 1] == '\n') sz--;
	org = 0;
	while (org < sz && text->data[org] == '\n') org++;
	if (org >= sz) return;
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	hoedown_buffer_put(ob, text->data + org, sz - org);
	hoedown_buffer_putc(ob, '\n');
}

static int
rndr_triple_emphasis(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size) return 0;
	BUFPUTSL(ob, "<strong><em>");
	hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</em></strong>");
	return 1;
}

static void
rndr_hrule(struct hoedown_buffer *ob, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	hoedown_buffer_puts(ob, USE_XHTML(options) ? "<hr/>\n" : "<hr>\n");
}

static int
rndr_image(struct hoedown_buffer *ob, const struct hoedown_buffer *link, const struct hoedown_buffer *title, const struct hoedown_buffer *alt, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;
	if (!link || !link->size) return 0;

	BUFPUTSL(ob, "<img src=\"");
	escape_href(ob, link->data, link->size);
	BUFPUTSL(ob, "\" alt=\"");

	if (alt && alt->size)
		escape_html(ob, alt->data, alt->size);

	if (title && title->size) {
		BUFPUTSL(ob, "\" title=\"");
		escape_html(ob, title->data, title->size); }

	hoedown_buffer_puts(ob, USE_XHTML(options) ? "\"/>" : "\">");
	return 1;
}

static int
rndr_raw_html(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	/* HTML_ESCAPE overrides SKIP_HTML, SKIP_STYLE, SKIP_LINKS and SKIP_IMAGES
	* It doens't see if there are any valid tags, just escape all of them. */
	if((options->flags & HOEDOWN_HTML_ESCAPE) != 0) {
		escape_html(ob, text->data, text->size);
		return 1;
	}

	if ((options->flags & HOEDOWN_HTML_SKIP_HTML) != 0)
		return 1;

	if ((options->flags & HOEDOWN_HTML_SKIP_STYLE) != 0 &&
		hoedown_html_is_tag(text->data, text->size, "style"))
		return 1;

	if ((options->flags & HOEDOWN_HTML_SKIP_LINKS) != 0 &&
		hoedown_html_is_tag(text->data, text->size, "a"))
		return 1;

	if ((options->flags & HOEDOWN_HTML_SKIP_IMAGES) != 0 &&
		hoedown_html_is_tag(text->data, text->size, "img"))
		return 1;

	hoedown_buffer_put(ob, text->data, text->size);
	return 1;
}

static void
rndr_table(struct hoedown_buffer *ob, const struct hoedown_buffer *header, const struct hoedown_buffer *body, void *opaque)
{
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	BUFPUTSL(ob, "<table><thead>\n");
	if (header)
		hoedown_buffer_put(ob, header->data, header->size);
	BUFPUTSL(ob, "</thead><tbody>\n");
	if (body)
		hoedown_buffer_put(ob, body->data, body->size);
	BUFPUTSL(ob, "</tbody></table>\n");
}

static void
rndr_tablerow(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	BUFPUTSL(ob, "<tr>\n");
	if (text)
		hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</tr>\n");
}

static void
rndr_tablecell(struct hoedown_buffer *ob, const struct hoedown_buffer *text, int flags, void *opaque)
{
	if (flags & HOEDOWN_TABLE_HEADER) {
		BUFPUTSL(ob, "<th");
	} else {
		BUFPUTSL(ob, "<td");
	}

	switch (flags & HOEDOWN_TABLE_ALIGNMASK) {
	case HOEDOWN_TABLE_ALIGN_CENTER:
		BUFPUTSL(ob, " style=\"text-align: center\">");
		break;

	case HOEDOWN_TABLE_ALIGN_L:
		BUFPUTSL(ob, " style=\"text-align: left\">");
		break;

	case HOEDOWN_TABLE_ALIGN_R:
		BUFPUTSL(ob, " style=\"text-align: right\">");
		break;

	default:
		BUFPUTSL(ob, ">");
	}

	if (text)
		hoedown_buffer_put(ob, text->data, text->size);

	if (flags & HOEDOWN_TABLE_HEADER) {
		BUFPUTSL(ob, "</th>\n");
	} else {
		BUFPUTSL(ob, "</td>\n");
	}
}

static int
rndr_superscript(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (!text || !text->size) return 0;
	BUFPUTSL(ob, "<sup>");
	hoedown_buffer_put(ob, text->data, text->size);
	BUFPUTSL(ob, "</sup>");
	return 1;
}

static void
rndr_normal_text(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	if (text)
		escape_html(ob, text->data, text->size);
}

static void
rndr_footnotes(struct hoedown_buffer *ob, const struct hoedown_buffer *text, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	if (ob->size) hoedown_buffer_putc(ob, '\n');
	BUFPUTSL(ob, "<div class=\"footnotes\">\n");
	hoedown_buffer_puts(ob, USE_XHTML(options) ? "<hr/>\n" : "<hr>\n");
	BUFPUTSL(ob, "<ol>\n");
	
	if (text)
		hoedown_buffer_put(ob, text->data, text->size);
	
	BUFPUTSL(ob, "\n</ol>\n</div>\n");
}

static void
rndr_footnote_def(struct hoedown_buffer *ob, const struct hoedown_buffer *text, unsigned int num, void *opaque)
{
	size_t i = 0;
	int pfound = 0;
	
	/* insert anchor at the end of first paragraph block */
	if (text) {
		while ((i+3) < text->size) {
			if (text->data[i++] != '<') continue;
			if (text->data[i++] != '/') continue;
			if (text->data[i++] != 'p' && text->data[i] != 'P') continue;
			if (text->data[i] != '>') continue;
			i -= 3;
			pfound = 1;
			break;
		}
	}
	
	hoedown_buffer_printf(ob, "\n<li id=\"fn%d\">\n", num);
	if (pfound) {
		hoedown_buffer_put(ob, text->data, i);
		hoedown_buffer_printf(ob, "&nbsp;<a href=\"#fnref%d\" rev=\"footnote\">&#8617;</a>", num);
		hoedown_buffer_put(ob, text->data + i, text->size - i);
	} else if (text) {
		hoedown_buffer_put(ob, text->data, text->size);
	}
	BUFPUTSL(ob, "</li>\n");
}

static int
rndr_footnote_ref(struct hoedown_buffer *ob, unsigned int num, void *opaque)
{
	hoedown_buffer_printf(ob, "<sup id=\"fnref%d\"><a href=\"#fn%d\" rel=\"footnote\">%d</a></sup>", num, num, num);
	return 1;
}

static void
toc_header(struct hoedown_buffer *ob, const struct hoedown_buffer *text, int level, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	if (level <= options->toc_data.nesting_level) {
		/* set the level offset if this is the first header
		 * we're parsing for the document */
		if (options->toc_data.current_level == 0)
			options->toc_data.level_offset = level - 1;

		level -= options->toc_data.level_offset;

		if (level > options->toc_data.current_level) {
			while (level > options->toc_data.current_level) {
				BUFPUTSL(ob, "<ul>\n<li>\n");
				options->toc_data.current_level++;
			}
		} else if (level < options->toc_data.current_level) {
			BUFPUTSL(ob, "</li>\n");
			while (level < options->toc_data.current_level) {
				BUFPUTSL(ob, "</ul>\n</li>\n");
				options->toc_data.current_level--;
			}
			BUFPUTSL(ob,"<li>\n");
		} else {
			BUFPUTSL(ob,"</li>\n<li>\n");
		}

		hoedown_buffer_printf(ob, "<a href=\"#toc_%d\">", options->toc_data.header_count++);
		if (text) escape_html(ob, text->data, text->size);
		BUFPUTSL(ob, "</a>\n");
	}
}

static int
toc_link(struct hoedown_buffer *ob, const struct hoedown_buffer *link, const struct hoedown_buffer *title, const struct hoedown_buffer *content, void *opaque)
{
	if (content && content->size)
		hoedown_buffer_put(ob, content->data, content->size);
	return 1;
}

static void
toc_finalize(struct hoedown_buffer *ob, void *opaque)
{
	struct hoedown_html_renderopt *options = opaque;

	while (options->toc_data.current_level > 0) {
		BUFPUTSL(ob, "</li>\n</ul>\n");
		options->toc_data.current_level--;
	}
}

void
hoedown_html_toc_renderer(struct hoedown_callbacks *callbacks, struct hoedown_html_renderopt *options, int nesting_level)
{
	static const struct hoedown_callbacks cb_default = {
		NULL,
		NULL,
		NULL,
		toc_header,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,

		NULL,
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		rndr_underline,
		rndr_highlight,
		rndr_quote,
		NULL,
		NULL,
		toc_link,
		NULL,
		rndr_triple_emphasis,
		rndr_strikethrough,
		rndr_superscript,
		NULL,

		NULL,
		NULL,

		NULL,
		toc_finalize,
	};

	memset(options, 0x0, sizeof(struct hoedown_html_renderopt));
	options->flags = HOEDOWN_HTML_TOC;
	options->toc_data.nesting_level = nesting_level;

	memcpy(callbacks, &cb_default, sizeof(struct hoedown_callbacks));
}

void
hoedown_html_renderer(struct hoedown_callbacks *callbacks, struct hoedown_html_renderopt *options, unsigned int render_flags, int toc_nesting_lvl)
{
	static const struct hoedown_callbacks cb_default = {
		rndr_blockcode,
		rndr_blockquote,
		rndr_raw_block,
		rndr_header,
		rndr_hrule,
		rndr_list,
		rndr_listitem,
		rndr_paragraph,
		rndr_table,
		rndr_tablerow,
		rndr_tablecell,
		rndr_footnotes,
		rndr_footnote_def,

		rndr_autolink,
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		rndr_underline,
		rndr_highlight,
		rndr_quote,
		rndr_image,
		rndr_linebreak,
		rndr_link,
		rndr_raw_html,
		rndr_triple_emphasis,
		rndr_strikethrough,
		rndr_superscript,
		rndr_footnote_ref,

		NULL,
		rndr_normal_text,

		NULL,
		NULL,
	};

	/* Prepare the options pointer */
	memset(options, 0x0, sizeof(struct hoedown_html_renderopt));
	options->flags = render_flags;

	if (toc_nesting_lvl > 0) {
		options->flags |= HOEDOWN_HTML_TOC;
		options->toc_data.nesting_level = toc_nesting_lvl;
	}

	/* Prepare the callbacks */
	memcpy(callbacks, &cb_default, sizeof(struct hoedown_callbacks));

	if (render_flags & HOEDOWN_HTML_SKIP_IMAGES)
		callbacks->image = NULL;

	if (render_flags & HOEDOWN_HTML_SKIP_LINKS) {
		callbacks->link = NULL;
		callbacks->autolink = NULL;
	}

	if (render_flags & HOEDOWN_HTML_SKIP_HTML || render_flags & HOEDOWN_HTML_ESCAPE)
		callbacks->blockhtml = NULL;
}
