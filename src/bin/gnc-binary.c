/*
 * gnc-binary.c -- The program entry point for GnuCash
 *
 * Copyright (C) 2006 Chris Shoemaker <c.shoemaker@cox.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, contact:
 *
 * Free Software Foundation           Voice:  +1-617-542-5942
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652
 * Boston, MA  02110-1301,  USA       gnu@gnu.org
 */
#include "config.h"

#include "gnc-application.h"
#include "gnc-engine.h"
#include "gnc-filepath-utils.h"

/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libguile.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "glib.h"
#include "gnc-module.h"
#include "gnc-path.h"
#include "binreloc.h"
#include "gnc-locale-utils.h"
#include "core-utils/gnc-version.h"
#include "gnc-environment.h"
#include "gnc-filepath-utils.h"
#include "gnc-ui-util.h"
#include "gnc-file.h"
#include "gnc-hooks.h"
#include "top-level.h"
#include "gfec.h"
#include "gnc-commodity.h"
#include "gnc-prefs.h"
#include "gnc-prefs-utils.h"
#include "gnc-gsettings.h"
#include "gnc-report.h"
#include "gnc-main-window.h"
#include "gnc-splash.h"
#include "gnc-gnome-utils.h"
#include "dialog-new-user.h"
#include "gnc-session.h"
#include "engine-helpers-guile.h"
#include "swig-runtime.h"
*/

/* This static indicates the debugging module that this .o belongs to.  */
static QofLogModule log_module = GNC_MOD_GUI;

#ifdef HAVE_GETTEXT
#  include <libintl.h>
#  include <locale.h>
#endif
#if 0
static void
inner_main_add_price_quotes(void *closure, int argc, char **argv)
{
    SCM mod, add_quotes, scm_book, scm_result = SCM_BOOL_F;
    QofSession *session = NULL;

    scm_c_eval_string("(debug-set! stack 200000)");

    mod = scm_c_resolve_module("gnucash price-quotes");
    scm_set_current_module(mod);

    /* Don't load the modules since the stylesheet module crashes if the
       GUI is not initialized */
#ifdef PRICE_QUOTES_NEED_MODULES
    load_gnucash_modules();
#endif
    gnc_prefs_init ();
    qof_event_suspend();
    scm_c_eval_string("(gnc:price-quotes-install-sources)");

    if (!gnc_quote_source_fq_installed())
    {
        g_print("%s", _("No quotes retrieved. Finance::Quote isn't "
                        "installed properly.\n"));
        goto fail;
    }

    add_quotes = scm_c_eval_string("gnc:book-add-quotes");
    session = gnc_get_current_session();
    if (!session) goto fail;

    qof_session_begin(session, add_quotes_file, FALSE, FALSE, FALSE);
    if (qof_session_get_error(session) != ERR_BACKEND_NO_ERR) goto fail;

    qof_session_load(session, NULL);
    if (qof_session_get_error(session) != ERR_BACKEND_NO_ERR) goto fail;

    scm_book = gnc_book_to_scm(qof_session_get_book(session));
    scm_result = scm_call_2(add_quotes, SCM_BOOL_F, scm_book);

    qof_session_save(session, NULL);
    if (qof_session_get_error(session) != ERR_BACKEND_NO_ERR) goto fail;

    qof_session_destroy(session);
    if (!scm_is_true(scm_result))
    {
        g_warning("Failed to add quotes to %s.", add_quotes_file);
        goto fail;
    }

    qof_event_resume();
    gnc_shutdown(0);
    return;
fail:
    if (session && qof_session_get_error(session) != ERR_BACKEND_NO_ERR)
        g_warning("Session Error: %s", qof_session_get_error_message(session));
    qof_event_resume();
    gnc_shutdown(1);
}
#endif

int
main (int argc, char *argv[])
{
    g_set_application_name(PACKAGE_NAME);
    return g_application_run (G_APPLICATION (gnc_application_new ()), argc, argv);
}
