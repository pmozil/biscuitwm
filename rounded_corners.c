#include <xcb/xcb.h>
#include <xcb/shape.h>
#include <stdlib.h>

extern xcb_connection_t *d;

void window_rounded_border(xcb_window_t win, unsigned int radius)
{
    // Check for compatibility
 //   const xcb_query_extension_reply_t *shape_query;

//    shape_query = xcb_get_extension_data (d, &win);

    // get geometry
	xcb_get_geometry_reply_t *geo = xcb_get_geometry_reply(d, xcb_get_geometry(d, win), NULL);
    if (geo == NULL) return;

//    uint16_t x  = geo->x;
//    uint16_t y  = geo->y;
	uint16_t w  = geo->width;
    uint16_t h  = geo->height;
    uint16_t bw = geo->border_width;
	uint16_t ow  = w+2*bw;
	uint16_t oh  = h+2*bw;

    xcb_pixmap_t bpid = xcb_generate_id(d);
    xcb_pixmap_t cpid = xcb_generate_id(d);

    xcb_create_pixmap(d, 1, bpid, win, ow, oh);
    xcb_create_pixmap(d, 1, cpid, win, w, h);

    xcb_gcontext_t black = xcb_generate_id(d);
    xcb_gcontext_t white = xcb_generate_id(d);

    xcb_create_gc(d, black, bpid, XCB_GC_FOREGROUND, (uint32_t[]){0, 0});
    xcb_create_gc(d, white, bpid, XCB_GC_FOREGROUND, (uint32_t[]){1, 0});

    int32_t rad, dia;
    rad = radius;

    rad += bw; dia = rad*2-1;

    xcb_arc_t barcs[] = {
        { -1,     -1,     dia, dia, 0, 360 << 6 },
        { -1,     oh-dia, dia, dia, 0, 360 << 6 },
        { ow-dia, -1,     dia, dia, 0, 360 << 6 },
        { ow-dia, oh-dia, dia, dia, 0, 360 << 6 },
    };
    xcb_rectangle_t brects[] = {
        { rad, 0, ow-dia, oh },
        { 0, rad, ow, oh-dia },
    };

    rad -= bw; dia = rad*2-1;

    xcb_arc_t carcs[] = {
        { -1,    -1,    dia, dia, 0, 360 << 6 },
        { -1,    h-dia, dia, dia, 0, 360 << 6 },
        { w-dia, -1,    dia, dia, 0, 360 << 6 },
        { w-dia, h-dia, dia, dia, 0, 360 << 6 },
    };
    xcb_rectangle_t crects[] = {
        { rad, 0, w-dia, h },
        { 0, rad, w, h-dia },
    };

    xcb_rectangle_t bounding = {0, 0, w+2*bw, h+2*bw};
    xcb_poly_fill_rectangle(d, bpid, black, 1, &bounding);
    xcb_poly_fill_rectangle(d, bpid, white, 2, brects);
    xcb_poly_fill_arc(d, bpid, white, 4, barcs);

    xcb_rectangle_t clipping = {0, 0, w, h};
    xcb_poly_fill_rectangle(d, cpid, black, 1, &clipping);
    xcb_poly_fill_rectangle(d, cpid, white, 2, crects);
    xcb_poly_fill_arc(d, cpid, white, 4, carcs);

    xcb_shape_mask(d, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING,  win, -bw, -bw, bpid);
    xcb_shape_mask(d, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_CLIP, win, 0, 0, cpid);

/*    if (n->presel != NULL && n->presel != XCB_NONE) {
        xcb_window_t fb = n->presel->feedback;
        xcb_get_geometry_reply_t *fb_geo = xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, fb), NULL);

        if (fb_geo != NULL) {
            xcb_shape_mask(dpy, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, fb, x-fb_geo->x, y-fb_geo->y, bpid);
            free(fb_geo);
        }
    }
*/

//    xcb_shape_mask(d, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, geo, x-16, y-16, bpid);

    free(geo);
    xcb_free_pixmap(d, bpid);
    xcb_free_pixmap(d, cpid);
}
