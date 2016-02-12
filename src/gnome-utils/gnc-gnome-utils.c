/********************************************************************\
 * gnc-gnome-utils.c -- utility functions for gnome for GnuCash     *
 * Copyright (C) 2001 Linux Developers Group                        *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

#include "config.h"

#include <glib/gi18n.h>
#include <libxml/xmlIO.h>

#include "gnc-component-manager.h"
#include "gnc-prefs-utils.h"
#include "gnc-prefs.h"
#include "gnc-gnome-utils.h"
//#include "gnc-html.h"
#include "gnc-engine.h"
#include "gnc-path.h"
#include "gnc-ui.h"
#include "gnc-file.h"
#include "gnc-hooks.h"
#include "gnc-filepath-utils.h"
#include "gnc-menu-extensions.h"
#include "gnc-splash.h"
#include "gnc-window.h"
#include "dialog-options.h"
#include "dialog-commodity.h"
#include "dialog-totd.h"
#include "gnc-ui-util.h"
#include "gnc-session.h"
#include "qofbookslots.h"
#ifdef G_OS_WIN32
#include <windows.h>
#include "gnc-help-utils.h"
#endif

static QofLogModule log_module = GNC_MOD_GUI;
static int gnome_is_running = FALSE;
static int gnome_is_terminating = FALSE;
static int gnome_is_initialized = FALSE;

#define ACCEL_MAP_NAME "accelerator-map"

static void gnc_book_options_help_cb (GNCOptionWin *win, gpointer dat);

static void
gnc_global_options_help_cb (GNCOptionWin *win, gpointer dat)
{
    gnc_gnome_help (HF_HELP, HL_GLOBPREFS);
}

static void
gnc_book_options_help_cb (GNCOptionWin *win, gpointer dat)
{
    gnc_gnome_help (HF_HELP, HL_BOOK_OPTIONS);
}

void
gnc_options_dialog_set_book_options_help_cb (GNCOptionWin *win)
{
    gnc_options_dialog_set_help_cb(win,
                                (GNCOptionWinCallback)gnc_book_options_help_cb,
                                NULL);
}

void
gnc_options_dialog_set_new_book_option_values (GNCOptionDB *odb)
{
    GNCOption *num_source_option;
    GtkWidget *num_source_is_split_action_button;
    gboolean num_source_is_split_action;

    if (!odb) return;
    num_source_is_split_action = gnc_prefs_get_bool(GNC_PREFS_GROUP_GENERAL,
                                                    GNC_PREF_NUM_SOURCE);
    if (num_source_is_split_action)
    {
        num_source_option = gnc_option_db_get_option_by_name(odb,
                                                 OPTION_SECTION_ACCOUNTS,
                                                 OPTION_NAME_NUM_FIELD_SOURCE);
        num_source_is_split_action_button =
                                gnc_option_get_gtk_widget (num_source_option);
        gtk_toggle_button_set_active
                    (GTK_TOGGLE_BUTTON (num_source_is_split_action_button),
                        num_source_is_split_action);
    }
}

static void
gnc_commodity_help_cb (void)
{
    gnc_gnome_help (HF_HELP, HL_COMMODITY);
}

/* gnc_configure_date_format
 *    sets dateFormat to the current value on the scheme side
 *
 * Args: Nothing
 * Returns: Nothing
 */
static void
gnc_configure_date_format (void)
{
    QofDateFormat df = gnc_prefs_get_int(GNC_PREFS_GROUP_GENERAL,
                                         GNC_PREF_DATE_FORMAT);

    /* Only a subset of the qof date formats is currently
     * supported for date entry.
     */
    if ((df > QOF_DATE_FORMAT_LOCALE)
            || (df > QOF_DATE_FORMAT_LOCALE))
    {
        PERR("Incorrect date format");
        return;
    }

    qof_date_format_set(df);
}

/* gnc_configure_date_completion
 *    sets dateCompletion to the current value on the scheme side.
 *    QOF_DATE_COMPLETION_THISYEAR: use current year
 *    QOF_DATE_COMPLETION_SLIDING: use a sliding 12-month window
 *    backmonths 0-11: windows starts this many months before current month
 *
 * Args: Nothing
 * Returns: Nothing
 */
static void
gnc_configure_date_completion (void)
{
    QofDateCompletion dc = QOF_DATE_COMPLETION_THISYEAR;
    int backmonths = gnc_prefs_get_float(GNC_PREFS_GROUP_GENERAL,
                                         GNC_PREF_DATE_BACKMONTHS);

    if (backmonths < 0)
        backmonths = 0;
    else if (backmonths > 11)
        backmonths = 11;

    if (gnc_prefs_get_bool (GNC_PREFS_GROUP_GENERAL, GNC_PREF_DATE_COMPL_SLIDING))
        dc = QOF_DATE_COMPLETION_SLIDING;

    qof_date_completion_set(dc, backmonths);
}

#ifdef G_OS_WIN32 /* G_OS_WIN32 */
void
gnc_gnome_help (const char *file_name, const char *anchor)
{
    const gchar * const *lang;
    gchar *pkgdatadir, *fullpath, *found = NULL;

    pkgdatadir = gnc_path_get_pkgdatadir ();
    for (lang = g_get_language_names (); *lang; lang++)
    {
        fullpath = g_build_filename (pkgdatadir, "help", *lang, file_name,
                                     (gchar*) NULL);
        if (g_file_test (fullpath, G_FILE_TEST_IS_REGULAR))
        {
            found = g_strdup (fullpath);
            g_free (fullpath);
            break;
        }
        g_free (fullpath);
    }
    g_free (pkgdatadir);

    if (!found)
    {
        const gchar *message =
            _("GnuCash could not find the files for the help documentation.");
        gnc_error_dialog (NULL, message);
    }
    else
    {
        gnc_show_htmlhelp (found, anchor);
    }
    g_free (found);
}
#else
void
gnc_gnome_help (const char *file_name, const char *anchor)
{
    GError *error = NULL;
    gchar *uri = NULL;
    gboolean success;

    if (anchor)
        uri = g_strconcat ("ghelp:", file_name, "?", anchor, NULL);
    else
        uri = g_strconcat ("ghelp:", file_name, NULL);

    DEBUG ("Attempting to opening help uri %s", uri);
    success = gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);
    g_free (uri);
    if (success)
        return;

    g_assert(error != NULL);
    {
        const gchar *message =
            _("GnuCash could not find the files for the help documentation. "
              "This is likely because the 'gnucash-docs' package is not installed.");
        gnc_error_dialog(NULL, "%s", message);
    }
    PERR ("%s", error->message);
    g_error_free(error);
}
#endif

#ifdef G_OS_WIN32 /* G_OS_WIN32 */
void
gnc_launch_assoc (const char *uri)
{
    wchar_t *winuri = NULL;
    /* ShellExecuteW open doesn't decode http escapes if it's passed a
     * file URI so we have to do it. */
    if (strcmp (g_uri_parse_scheme(uri), "file") == 0)
    {
    gchar *filename = g_filename_from_uri (uri, NULL, NULL);
    winuri = (wchar_t *)g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
    }
    else
    winuri = (wchar_t *)g_utf8_to_utf16(uri, -1, NULL, NULL, NULL);

    if (winuri)
    {
    wchar_t *wincmd = (wchar_t *)g_utf8_to_utf16("open", -1,
                             NULL, NULL, NULL);
    if ((INT_PTR)ShellExecuteW(NULL, wincmd, winuri,
                   NULL, NULL, SW_SHOWNORMAL) <= 32)
    {
        const gchar *message =
        _("GnuCash could not find the associated file");
        gnc_error_dialog(NULL, "%s: %s", message, uri);
    }
    g_free (wincmd);
    g_free (winuri);
    }
}
#else
void
gnc_launch_assoc (const char *uri)
{
    GError *error = NULL;
    gboolean success;

    if (!uri)
        return;

    DEBUG ("Attempting to open uri %s", uri);
    success = gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);
    if (success)
        return;

    g_assert(error != NULL);
    {
        const gchar *message =
            _("GnuCash could not open the associated URI:");
        gnc_error_dialog(NULL, "%s\n%s", message, uri);
    }
    PERR ("%s", error->message);
    g_error_free(error);
}
#endif

void
gnc_gui_init(GncMainWindow *main_window)
{
    gchar *map;

    ENTER ("");

    gnc_prefs_init();
    gnc_show_splash_screen();

    gnome_is_initialized = TRUE;

    gnc_ui_util_init();
    gnc_configure_date_format();
    gnc_configure_date_completion();

    gnc_prefs_register_cb (GNC_PREFS_GROUP_GENERAL,
                           GNC_PREF_DATE_FORMAT,
                           gnc_configure_date_format,
                           NULL);
    gnc_prefs_register_cb (GNC_PREFS_GROUP_GENERAL,
                           GNC_PREF_DATE_COMPL_THISYEAR,
                           gnc_configure_date_completion,
                           NULL);
    gnc_prefs_register_cb (GNC_PREFS_GROUP_GENERAL,
                           GNC_PREF_DATE_COMPL_SLIDING,
                           gnc_configure_date_completion,
                           NULL);
    gnc_prefs_register_cb (GNC_PREFS_GROUP_GENERAL,
                           GNC_PREF_DATE_BACKMONTHS,
                           gnc_configure_date_completion,
                           NULL);
    gnc_prefs_register_group_cb (GNC_PREFS_GROUP_GENERAL,
                                gnc_gui_refresh_all,
                                NULL);

    gnc_ui_commodity_set_help_callback (gnc_commodity_help_cb);

    gnc_options_dialog_set_global_help_cb (gnc_global_options_help_cb, NULL);

    // Bug#350993:
    gnc_window_set_progressbar_window (GNC_WINDOW(main_window));

    map = gnc_build_dotgnucash_path(ACCEL_MAP_NAME);
    gtk_accel_map_load(map);
    g_free(map);

    gnc_totd_dialog(GTK_WINDOW(main_window), TRUE);

    LEAVE ("");
}

void
gnc_shutdown(int i)
{
    g_application_quit(g_application_get_default());
}

