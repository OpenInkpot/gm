#include <stdio.h>
#include <string.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_X_Atoms.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_atom.h>
#include "raise.h"

#define LONG_MAX 0x7fffffff

static
Ecore_X_Window
gm_root_get()
{
    Ecore_X_Screen *s = ecore_x_default_screen_get();
    return  ((xcb_screen_t*)s)->root;
}

static inline
xcb_connection_t *
gm_connection_get()
{
    return (xcb_connection_t *) ecore_x_connection_get();
}

static
xcb_atom_t
gm_get_atom(const char *name)
{
    xcb_intern_atom_cookie_t cookie;
    xcb_generic_error_t* err;
    xcb_intern_atom_reply_t* reply;
    xcb_connection_t *xcb_conn = gm_connection_get();
    cookie = xcb_intern_atom(xcb_conn, 0, strlen(name), name);

    reply = xcb_intern_atom_reply(xcb_conn, cookie, &err);
    if(err)
    {
        free(err);
        return XCB_NONE;
    }
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

Ecore_X_Window
gm_get_active_document_window()
{
    xcb_get_property_cookie_t cookie;
    xcb_atom_t active_doc_window_id = gm_get_atom("ACTIVE_DOC_WINDOW_ID");
    if(!active_doc_window_id)
    {
        printf("No atom\n");
        return 0;
    }
    //printf("Atom: %d\n", active_doc_window_id);
    Ecore_X_Window root = gm_root_get();
    Ecore_X_Window value = 0;
    xcb_connection_t *conn = gm_connection_get();
    cookie =  xcb_get_property(conn, 0,
                root,
                active_doc_window_id,
                WINDOW,
                0L,
                LONG_MAX);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
    if (!reply)
    {
        //printf("Not reply\n");
        return 0;
    }
    if (reply->type == XCB_NONE)
    {
        //printf("XCB_NONE\n");
        goto bad;
    }
    if (reply->format != 32)
    {
        printf("not 32\n");
        goto bad;
    }
    if(reply->value_len != 1)
    {
        printf("value_len=%d\n", reply->value_len);
        goto bad;
    }

    value = *(Ecore_X_Window *)xcb_get_property_value(reply);
bad:
    free(reply);
    //printf("Got: %x", value);
    return value;
}

void
open_current_book(Evas * e __attribute__((unused)))
{
    Ecore_X_Window fbreader = gm_get_active_document_window();
    if (fbreader)
    {
        printf("Raising\n");
        ecore_x_window_raise(fbreader);
        ecore_x_window_focus(fbreader);
    }
}


/*
    If save_size given -- will be string not terminated with zero, and
    len will be stored in *save_size
*/
static char *
gm_get_fb_string_internal(Ecore_X_Window win, xcb_connection_t *conn,
                char *prop, xcb_atom_t atom_type,
                int *save_size)
{
    char *result = NULL;
    xcb_atom_t atom = gm_get_atom(prop);
    if(!atom)
    {
        printf("Can't get atom %s\n", prop);
        return NULL;
    }
    xcb_get_property_cookie_t cookie;
    cookie =  xcb_get_property(conn, 0,
                win,
                atom,
                atom_type,
                0L,
                LONG_MAX);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);

    if (!reply)
    {
//        printf("Not reply\n");
        return 0;
    }
    if (reply->type == XCB_NONE)
    {
//       printf("XCB_NONE\n");
        goto bad;
    }
    if (reply->type != atom_type)
    {
        printf("BAD TYPE\n");
        goto bad;
    }
    int len = reply->value_len; // xcb_get_property_value_length(reply);
    result = malloc(len+1);
    if(result)
    {
        memcpy(result, xcb_get_property_value(reply), len);
        if(save_size)
            *save_size = len;
        else
            result[len]='\0';
        free(reply);
//        printf("Got: %s = %s (%d)\n", prop, result, len);
        return result;
    }
bad:
    if(result)
        free(result);
    free(reply);
    return NULL;
}

char *
gm_get_fb_string(Ecore_X_Window win, xcb_connection_t *conn, char *prop)
{
    xcb_atom_t utf8_string = gm_get_atom("UTF8_STRING");
    if(!utf8_string)
    {
        printf("Can't get atom UTF8_STRING\n");
        return NULL;
    }
    return gm_get_fb_string_internal(win, conn, prop, utf8_string, NULL);
}

char *
gm_get_fb_string_blob(Ecore_X_Window win, xcb_connection_t *conn, char *prop,
                int *save_size)
{
    xcb_atom_t atom_type = gm_get_atom("STRING");
    if(!atom_type)
    {
        printf("Can't get atom STRING\n");
        return NULL;
    }
    return gm_get_fb_string_internal(win, conn, prop, atom_type, save_size);
}

int
gm_get_fb_int(Ecore_X_Window win, xcb_connection_t *conn, char *prop)
{
    int result = 0;
    xcb_atom_t atom = gm_get_atom(prop);
    if(!atom)
    {
//        printf("Can't get atom %s\n", prop);
        return 0;
    }
    xcb_atom_t integer_atom = gm_get_atom("INTEGER");
    if(!atom)
    {
        printf("Can't get atom INTEGER\n");
        return 0;
    }
    xcb_get_property_cookie_t cookie;
    cookie =  xcb_get_property(conn, 0,
                win,
                atom,
                integer_atom,
                0L,
                LONG_MAX);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
    if (!reply)
    {
//        printf("Not reply: %s\n", prop);
        return 0;
    }
    if (reply->type == XCB_NONE)
    {
//        printf("XCB_NONE\n");
        goto bad;
    }
//    printf("reply->type = %d, reply->format = %d\n", reply->type, reply->format);
    //int len = xcb_get_property_value_length(reply);
    result = *(int *) xcb_get_property_value(reply);
//    printf("Got: %s = %d %d\n", prop, result, len);
bad:
    free(reply);
    return result;
}

struct bookinfo_t *
gm_get_titles()
{
    Ecore_X_Window fbreader = gm_get_active_document_window();
    xcb_connection_t *conn = gm_connection_get();
    struct bookinfo_t *titles = calloc(sizeof(struct bookinfo_t), 1);
    if(!titles)
        return NULL;
    titles->author = gm_get_fb_string(fbreader, conn, "ACTIVE_DOC_AUTHOR");
    titles->filename = gm_get_fb_string(fbreader, conn, "ACTIVE_DOC_FILENAME");
    titles->filepath = gm_get_fb_string(fbreader, conn, "ACTIVE_DOC_FILEPATH");
    titles->series = gm_get_fb_string(fbreader, conn, "ACTIVE_DOC_SERIES");
    titles->title = gm_get_fb_string(fbreader, conn, "ACTIVE_DOC_TITLE");
    titles->series_number = gm_get_fb_int(fbreader, conn,
                                        "ACTIVE_DOC_SERIES_NUMBER");
    titles->cover_image = gm_get_fb_string_blob(fbreader, conn,
                            "ACTIVE_DOC_COVER_IMAGE", &titles->cover_size);
    titles->pages_count = gm_get_fb_int(fbreader, conn, "ACTIVE_DOC_PAGES_COUNT");
    titles->current_page = gm_get_fb_int(fbreader, conn, "ACTIVE_DOC_CURRENT_PAGE");
    return titles;
}

void
gm_free_titles(struct bookinfo_t *titles)
{
    if(!titles)
        return;
    free(titles->author);
    free(titles->filename);
    free(titles->filepath);
    free(titles->series);
    free(titles->title);
    free(titles->cover_image);
    free(titles);
}
