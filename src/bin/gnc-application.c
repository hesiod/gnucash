/*
 * gnc-application.c -- The program entry point for GnuCash
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

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#if !defined(G_THREADS_ENABLED) || defined(G_THREADS_IMPL_NONE)
#    error "No GLib thread implementation available!"
#endif

#include <libguile.h>

#include "core-utils/gnc-version.h"
#include "binreloc.h"
#include "gfec.h"

#include "gnc-main-window.h"

#include "gnc-application.h"
#include "gnc-commodity.h"
#include "gnc-component-manager.h"
#include "gnc-engine.h"
#include "gnc-environment.h"
#include "gnc-file.h"
#include "gnc-filepath-utils.h"
#include "gnc-gnome-utils.h"
#include "gnc-gsettings.h"
#include "gnc-hooks.h"
#include "gnc-locale-utils.h"
#include "gnc-module.h"
#include "gnc-menu-extensions.h"
#include "gnc-path.h"
#include "gnc-prefs.h"
#include "gnc-prefs-utils.h"
#include "gnc-report.h"
#include "gnc-session.h"
#include "gnc-splash.h"
#include "gnc-ui-util.h"

#include "top-level.h"
#include "dialog-new-user.h"
#include "engine-helpers-guile.h"
#include "swig-runtime.h"

/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "glib.h"
*/

#define ACCEL_MAP_NAME "accelerator-map"

/* This static indicates the debugging module that this .o belongs to.  */
static QofLogModule log_module = GNC_MOD_GUI;

/* GNUCASH_SCM is defined whenever we're building from an svn/svk/git/bzr tree */
#ifdef GNUCASH_SCM
static int is_development_version = TRUE;
#else
static int is_development_version = FALSE;
#define GNUCASH_SCM ""
#endif

struct _GncApplication {
    GtkApplication parent_instance;
};

G_DEFINE_TYPE(GncApplication, gnc_application, GTK_TYPE_APPLICATION);

static guint id;

/* Command-line option variables */
static int          gnucash_show_version = 0;
static int          debugging        = 0;
static int          extra            = 0;
static gchar      **log_flags        = NULL;
static gchar       *log_to_filename  = NULL;
static int          nofile           = 0;
static const gchar *gsettings_prefix = NULL;
static const char  *add_quotes_file  = NULL;
static char        *namespace_regexp = NULL;
static const char  *file_to_load     = NULL;
static gchar      **args_remaining   = NULL;

static GOptionEntry options[] =
{
    {
        "version", 'v', 0, G_OPTION_ARG_NONE, &gnucash_show_version,
        N_("Show GnuCash version"), NULL
    },

    {
        "debug", '\0', 0, G_OPTION_ARG_NONE, &debugging,
        N_("Enable debugging mode: provide deep detail in the logs.\nThis is equivalent to: --log \"=info\" --log \"qof=info\" --log \"gnc=info\""), NULL
    },

    {
        "extra", '\0', 0, G_OPTION_ARG_NONE, &extra,
        N_("Enable extra/development/debugging features."), NULL
    },

    {
        "log", '\0', 0, G_OPTION_ARG_STRING_ARRAY, &log_flags,
        N_("Log level overrides, of the form \"modulename={debug,info,warn,crit,error}\"\nExamples: \"--log qof=debug\" or \"--log gnc.backend.file.sx=info\"\nThis can be invoked multiple times."),
        NULL
    },

    {
        "logto", '\0', 0, G_OPTION_ARG_STRING, &log_to_filename,
        N_("File to log into; defaults to \"/tmp/gnucash.trace\"; can be \"stderr\" or \"stdout\"."),
        NULL
    },

    {
        "nofile", '\0', 0, G_OPTION_ARG_NONE, &nofile,
        N_("Do not load the last file opened"), NULL
    },
    {
        "gsettings-prefix", '\0', 0, G_OPTION_ARG_STRING, &gsettings_prefix,
        N_("Set the prefix for gsettings schemas for gsettings queries. This can be useful to have a different settings tree while debugging."),
        /* Translators: Argument description for autohelp; see
           http://developer.gnome.org/doc/API/2.0/glib/glib-Commandline-option-parser.html */
        N_("GSETTINGSPREFIX")
    },
    {
        "namespace", '\0', 0, G_OPTION_ARG_STRING, &namespace_regexp,
        N_("Regular expression determining which namespace commodities will be retrieved"),
        /* Translators: Argument description for autohelp; see
           http://developer.gnome.org/doc/API/2.0/glib/glib-Commandline-option-parser.html */
        N_("REGEXP")
    },
    {
        G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &args_remaining, NULL, N_("[datafile]") },
    { NULL }
};


static void
gnc_print_unstable_message(void)
{
    if (!is_development_version) return;

    g_print("\n\n%s\n%s\n%s\n%s\n",
            _("This is a development version. It may or may not work."),
            _("Report bugs and other problems to gnucash-devel@gnucash.org"),
            _("You can also lookup and file bug reports at http://bugzilla.gnome.org"),
            _("To find the last stable version, please refer to http://www.gnucash.org"));
}


static void
gnc_log_init()
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

            gchar *log_opt = log_flags[i];
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

static void
update_message(const gchar *msg)
{
    gnc_update_splash_screen(msg, GNC_SPLASH_PERCENTAGE_UNKNOWN);
    g_message("%s", msg);
}

static void
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

static void
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

static void
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

static void
load_gnucash_modules()
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

static gboolean
gnc_ui_check_events (gpointer not_used)
{
    QofSession *session;
    gboolean force;

    if (gtk_main_level() != 1)
        return TRUE;

    if (!gnc_current_session_exist())
        return TRUE;
    session = gnc_get_current_session ();

    if (gnc_gui_refresh_suspended ())
        return TRUE;

    if (!qof_session_events_pending (session))
        return TRUE;

    gnc_suspend_gui_refresh ();

    force = qof_session_process_events (session);

    gnc_resume_gui_refresh ();

    if (force)
        gnc_gui_refresh_all ();

    return TRUE;
}

static void
inner_main (void *closure, int argc, char **argv)
{
    scm_c_eval_string("(debug-set! stack 200000)");

    {
        SCM main_mod;

        main_mod = scm_c_resolve_module("gnucash main");
        scm_set_current_module(main_mod);
    }

    /* GnuCash switched to gsettings to store its preferences in version 2.5.6
     * Migrate the user's preferences from gconf if needed */
    gnc_gsettings_migrate_from_gconf();

    load_gnucash_modules();

    /* Load the config before starting up the gui. This insures that
     * custom reports have been read into memory before the Reports
     * menu is created. */
    load_system_config();
    load_user_config();

    /* Setting-up the report menu must come after the module
       loading but before the gui initialization. */
    scm_c_use_module("gnucash report report-gnome");
    scm_c_eval_string("(gnc:report-menu-setup)");

    /* TODO: After some more guile-extraction, this should happen even
       before booting guile.  */
    gnc_main_gui_init();

    gnc_hook_add_dangler(HOOK_UI_SHUTDOWN, (GFunc)gnc_file_quit, NULL);

    /* Install Price Quote Sources */
    gnc_update_splash_screen(_("Checking Finance::Quote..."), GNC_SPLASH_PERCENTAGE_UNKNOWN);
    scm_c_use_module("gnucash price-quotes");
    scm_c_eval_string("(gnc:price-quotes-install-sources)");

    gnc_hook_run(HOOK_STARTUP, NULL);

    if (!nofile && file_to_load)
    {
        char* fn = g_strdup(file_to_load);
        gnc_update_splash_screen(_("Loading data..."), GNC_SPLASH_PERCENTAGE_UNKNOWN);
        gnc_file_open_file(fn, /*open_readonly*/ FALSE);
        g_free(fn);
    }
    else if (gnc_prefs_get_bool(GNC_PREFS_GROUP_NEW_USER, GNC_PREF_FIRST_STARTUP))
    {
        gnc_destroy_splash_screen();
        gnc_ui_new_user_dialog();
    }
    /* Ensure temporary preferences are temporary */
    gnc_prefs_reset_group (GNC_PREFS_GROUP_WARNINGS_TEMP);

    gnc_destroy_splash_screen();
    gnc_main_window_show_all_windows();

    gnc_hook_run(HOOK_UI_POST_STARTUP, NULL);
}

static void
gnc_application_shutdown (GApplication *app)
{
    if (gnc_file_query_save(FALSE))
    {
        gchar *map;

        map = gnc_build_dotgnucash_path(ACCEL_MAP_NAME);
        gtk_accel_map_save(map);
        g_free(map);
    }

    g_source_remove (id);

    gnc_hook_run(HOOK_UI_SHUTDOWN, NULL);
    gnc_hook_remove_dangler(HOOK_UI_SHUTDOWN, (GFunc)gnc_file_quit);

    gnc_extensions_shutdown ();

    gnc_hook_run(HOOK_SHUTDOWN, NULL);
    //gnc_engine_shutdown();

    G_APPLICATION_CLASS (gnc_application_parent_class)->shutdown(app);
}

static void
gnc_application_init (GncApplication *app)
{
    g_application_add_main_option_entries (G_APPLICATION (app), options);
}

static gint
gnc_application_command_line (GApplication            *application,
                              GApplicationCommandLine *command_line)
{
    gnc_prefs_set_debugging(debugging);
    gnc_prefs_set_extra(extra);

    if (gsettings_prefix)
        gnc_gsettings_set_prefix(g_strdup(gsettings_prefix));

    if (namespace_regexp)
        gnc_prefs_set_namespace_regexp(namespace_regexp);

    if (args_remaining)
        file_to_load = args_remaining[0];

    return 0;
}

static gint
gnc_application_handle_local_options (GApplication *app,
                                      GVariantDict *dict)
{
    if (gnucash_show_version)
    {
        gchar *fixed_message;

        if (is_development_version)
        {
            fixed_message = g_strdup_printf(_("GnuCash %s development version"), VERSION);

            /* Translators: 1st %s is a fixed message, which is translated independently;
                            2nd %s is the scm type (svn/svk/git/bzr);
                            3rd %s is the scm revision number;
                            4th %s is the build date */
            g_print ( _("%s\nThis copy was built from %s rev %s on %s."),
                      fixed_message, GNUCASH_SCM, GNUCASH_SCM_REV,
                      GNUCASH_BUILD_DATE );
        }
        else
        {
            fixed_message = g_strdup_printf(_("GnuCash %s"), VERSION);

            /* Translators: 1st %s is a fixed message, which is translated independently;
                            2nd %s is the scm (svn/svk/git/bzr) revision number;
                            3rd %s is the build date */
            g_print ( _("%s\nThis copy was built from rev %s on %s."),
                      fixed_message, GNUCASH_SCM_REV, GNUCASH_BUILD_DATE );
        }
        g_print("\n");
        g_free (fixed_message);
        return 0; // Exit, Success
    }

    return -1; // Continue
}

static void
gnc_application_activate (GApplication *app)
{
    GncMainWindow *win;

    win = gnc_main_window_new ();
    gnc_gui_init(win);
    gtk_window_present (GTK_WINDOW (win));
}

static void
gnc_application_startup (GApplication *app)
{
    G_APPLICATION_CLASS (gnc_application_parent_class)->startup(app);

#ifdef ENABLE_BINRELOC
    {
        GError *binreloc_error = NULL;
        if (!gnc_gbr_init(&binreloc_error))
        {
            g_print("main: Error on gnc_gbr_init: %s\n", binreloc_error->message);
            g_error_free(binreloc_error);
        }
    }
#endif

    gnc_setup_locale();

    gnc_print_unstable_message();

    gnc_log_init();

    /* Now the module files are looked up, which might cause some library
    initialization to be run, hence gtk must be initialized beforehand. */
    gnc_module_system_init();

    //gnc_file_set_shutdown_callback (gnc_shutdown);
    id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 10000, /* 10 secs */
                             gnc_ui_check_events, NULL, NULL);

    //scm_boot_guile(argc, argv, inner_main, 0);
}

static void
gnc_application_open (GApplication  *app,
                      GFile        **files,
                      gint           n_files,
                      const gchar   *hint)
{
    GList *windows;
    GncMainWindow *win;
    int i;

    windows = gtk_application_get_windows (GTK_APPLICATION (app));
    if (windows)
        win = GNC_MAIN_WINDOW (windows->data);
    else
        win = gnc_main_window_new ();

#if 0
    for (i = 0; i < n_files; i++)
        gnc_main_window_open (win, files[i]);
#endif

    gnc_gui_init(win);
    gtk_window_present (GTK_WINDOW (win));
}

static void
gnc_application_class_init (GncApplicationClass *class)
{
    G_APPLICATION_CLASS (class)->activate = gnc_application_activate;
    G_APPLICATION_CLASS (class)->open = gnc_application_open;
    G_APPLICATION_CLASS (class)->command_line = gnc_application_command_line;
    G_APPLICATION_CLASS (class)->handle_local_options = gnc_application_handle_local_options;
    G_APPLICATION_CLASS (class)->startup = gnc_application_startup;
    G_APPLICATION_CLASS (class)->shutdown = gnc_application_shutdown;
}

GncApplication *
gnc_application_new (void)
{
     return g_object_new (GNC_TYPE_APPLICATION,
                          "application-id", "org.gnucash",
                          "flags", G_APPLICATION_HANDLES_OPEN
                                   | G_APPLICATION_HANDLES_COMMAND_LINE,
                          NULL);
}
