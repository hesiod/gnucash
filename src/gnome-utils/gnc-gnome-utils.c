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
#include "gfec.h"

#include "gnc-component-manager.h"
#include "gnc-prefs-utils.h"
#include "gnc-prefs.h"
#include "gnc-gnome-utils.h"
#include "gnc-engine.h"
#include "gnc-environment.h"
#include "gnc-path.h"
#include "gnc-ui.h"
#include "gnc-file.h"
#include "gnc-hooks.h"
#include "gnc-filepath-utils.h"
#include "gnc-menu-extensions.h"
#include "gnc-module.h"
#include "gnc-report.h"
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

static void
update_message(const gchar *msg)
{
    gnc_update_splash_screen(msg, GNC_SPLASH_PERCENTAGE_UNKNOWN);
    g_message("%s", msg);
}

void
gnc_log_init(const gchar *log_to_filename, const gchar **log_flags)
{
    if (log_to_filename != NULL)
    {
        qof_log_init_filename_special(log_to_filename);
    }
    else
    {
        /* initialize logging to our file. */
        gchar *tracefilename;
        tracefilename = g_build_filename(g_get_tmp_dir(), "gnucash.trace",
                                         (gchar *)NULL);
        qof_log_init_filename(tracefilename);
        g_free(tracefilename);
    }

    // set a reasonable default.
    qof_log_set_default(QOF_LOG_WARNING);

    gnc_log_default();

    if (gnc_prefs_is_debugging_enabled())
    {
        qof_log_set_level("", QOF_LOG_INFO);
        qof_log_set_level("qof", QOF_LOG_INFO);
        qof_log_set_level("gnc", QOF_LOG_INFO);
    }

#if 0
    {
        gchar *log_config_filename;
        log_config_filename = gnc_build_dotgnucash_path("log.conf");
        if (g_file_test(log_config_filename, G_FILE_TEST_EXISTS))
            qof_log_parse_log_config(log_config_filename);
        g_free(log_config_filename);
    }
#endif

    if (log_flags != NULL)
    {
        int i = 0;
        for (; log_flags[i] != NULL; i++)
        {
            QofLogLevel level;
            gchar **parts = NULL;

            const gchar *log_opt = log_flags[i];
            parts = g_strsplit(log_opt, "=", 2);
            if (parts == NULL || parts[0] == NULL || parts[1] == NULL)
            {
                g_warning("string [%s] not parseable", log_opt);
                continue;
            }

            level = qof_log_level_from_string(parts[1]);
            qof_log_set_level(parts[0], level);
            g_strfreev(parts);
        }
    }
}

static gboolean
try_load_config_array(const gchar *fns[])
{
    gchar *filename;
    int i;

    for (i = 0; fns[i]; i++)
    {
        filename = gnc_build_dotgnucash_path(fns[i]);
        if (gfec_try_load(filename))
        {
            g_free(filename);
            return TRUE;
        }
        g_free(filename);
    }
    return FALSE;
}

void
load_system_config(void)
{
    static int is_system_config_loaded = FALSE;
    gchar *system_config_dir;
    gchar *system_config;

    if (is_system_config_loaded) return;

    update_message("loading system configuration");
    system_config_dir = gnc_path_get_pkgsysconfdir();
    system_config = g_build_filename(system_config_dir, "config", NULL);
    is_system_config_loaded = gfec_try_load(system_config);
    g_free(system_config_dir);
    g_free(system_config);
}

void
load_user_config(void)
{
    /* Don't continue adding to this list. When 2.0 rolls around bump
       the 1.4 (unnumbered) files off the list. */
    static const gchar *user_config_files[] =
    {
        "config-2.0.user", "config-1.8.user", "config-1.6.user",
        "config.user", NULL
    };
    static const gchar *auto_config_files[] =
    {
        "config-2.0.auto", "config-1.8.auto", "config-1.6.auto",
        "config.auto", NULL
    };
    static const gchar *saved_report_files[] =
    {
        SAVED_REPORTS_FILE, SAVED_REPORTS_FILE_OLD_REV, NULL
    };
    static const gchar *stylesheet_files[] = { "stylesheets-2.0", NULL};
    static int is_user_config_loaded = FALSE;

    if (is_user_config_loaded)
        return;
    else is_user_config_loaded = TRUE;

    update_message("loading user configuration");
    try_load_config_array(user_config_files);
    update_message("loading auto configuration");
    try_load_config_array(auto_config_files);
    update_message("loading saved reports");
    try_load_config_array(saved_report_files);
    update_message("loading stylesheets");
    try_load_config_array(stylesheet_files);
}

void
gnc_setup_locale (void)
{
    gchar *sys_locale = NULL;
    gnc_environment_setup();
    sys_locale = g_strdup (setlocale (LC_ALL, ""));
    if (!sys_locale)
    {
        g_print ("The locale defined in the environment isn't supported. "
                 "Falling back to the 'C' (US English) locale\n");
        g_setenv ("LC_ALL", "C", TRUE);
        setlocale (LC_ALL, "C");
    }
#ifdef HAVE_GETTEXT
    {
        gchar *localedir = gnc_path_get_localedir();
        bindtextdomain(GETTEXT_PACKAGE, localedir);
        textdomain(GETTEXT_PACKAGE);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
        g_free(localedir);
    }
#endif

    /* Write some locale details to the log to simplify debugging */
    PINFO ("System locale returned %s", sys_locale ? sys_locale : "(null)");
    PINFO ("Effective locale set to %s.", setlocale (LC_ALL, ""));
    g_free (sys_locale);
}

void
load_gnucash_modules(void)
{
    int i, len;
    struct
    {
        gchar * name;
        int version;
        gboolean optional;
    } modules[] =
    {
        { "gnucash/app-utils", 0, FALSE },
        { "gnucash/engine", 0, FALSE },
        { "gnucash/register/ledger-core", 0, FALSE },
        { "gnucash/register/register-core", 0, FALSE },
        { "gnucash/register/register-gnome", 0, FALSE },
        { "gnucash/import-export/qif-import", 0, FALSE },
        { "gnucash/import-export/ofx", 0, TRUE },
        { "gnucash/import-export/csv-import", 0, TRUE },
        { "gnucash/import-export/csv-export", 0, TRUE },
        { "gnucash/import-export/log-replay", 0, TRUE },
        { "gnucash/import-export/aqbanking", 0, TRUE },
        { "gnucash/report/report-system", 0, FALSE },
        { "gnucash/report/stylesheets", 0, FALSE },
        { "gnucash/report/standard-reports", 0, FALSE },
        { "gnucash/report/utility-reports", 0, FALSE },
        { "gnucash/report/locale-specific/us", 0, FALSE },
        { "gnucash/report/report-gnome", 0, FALSE },
        { "gnucash/business-gnome", 0, TRUE },
        { "gnucash/gtkmm", 0, TRUE },
        { "gnucash/python", 0, TRUE },
        { "gnucash/plugins/bi_import", 0, TRUE},
        { "gnucash/plugins/customer_import", 0, TRUE},
    };

    /* module initializations go here */
    len = sizeof(modules) / sizeof(*modules);
    for (i = 0; i < len; i++)
    {
        DEBUG("Loading module %s started", modules[i].name);
        gnc_update_splash_screen(modules[i].name, GNC_SPLASH_PERCENTAGE_UNKNOWN);
        if (modules[i].optional)
            gnc_module_load_optional(modules[i].name, modules[i].version);
        else
            gnc_module_load(modules[i].name, modules[i].version);
        DEBUG("Loading module %s finished", modules[i].name);
    }
    if (!gnc_engine_is_initialized())
    {
        /* On Windows this check used to fail anyway, see
         * https://lists.gnucash.org/pipermail/gnucash-devel/2006-September/018529.html
         * but more recently it seems to work as expected
         * again. 2006-12-20, cstim. */
        g_warning("GnuCash engine failed to initialize.  Exiting.\n");
        exit(1);
    }
}

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

