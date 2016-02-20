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

#include "gnc-main-window.h"

#include "gnc-application.h"
#include "gnc-commodity.h"
#include "gnc-component-manager.h"
#include "gnc-engine.h"
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
#include "gnc-session.h"
#include "gnc-ui-util.h"
#include "gnc-ui.h"

#include "top-level.h"
#include "dialog-new-user.h"
#include "dialog-preferences.h"
#include "engine-helpers-guile.h"
#include "swig-runtime.h"

#define ACCEL_MAP_NAME "accelerator-map"

/* This static indicates the debugging module that this .o belongs to.  */
static QofLogModule log_module = GNC_MOD_GUI;

/* GNUCASH_SCM is defined whenever we're building from an svn/svk/git/bzr tree */
#ifdef GNUCASH_SCM
static int is_development_version = TRUE;
#else
static int is_development_version = FALSE;
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

static void gnc_main_window_cmd_edit_preferences (GSimpleAction *action, GVariant *parameter, gpointer window);
static void gnc_main_window_cmd_file_quit (GSimpleAction *action, GVariant *parameter, gpointer window);
static void gnc_main_window_cmd_help_tutorial (GSimpleAction *action, GVariant *parameter, gpointer window);
static void gnc_main_window_cmd_help_contents (GSimpleAction *action, GVariant *parameter, gpointer window);
static void gnc_main_window_cmd_help_about (GSimpleAction *action, GVariant *parameter, gpointer window);

static GActionEntry gnc_app_actions [] =
{
    {
        "quit", gnc_main_window_cmd_file_quit
    },
    /* Help menu */
    {
        "help.tutorial", gnc_main_window_cmd_help_tutorial
    },
    {
        "help.contents", gnc_main_window_cmd_help_contents
    },
    {
        "help.about", gnc_main_window_cmd_help_about
    },
    {
        "edit.preferences", gnc_main_window_cmd_edit_preferences
    },
};
static guint gnc_app_n_actions = G_N_ELEMENTS (gnc_app_actions);

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
    gnc_engine_shutdown();

    G_APPLICATION_CLASS (gnc_application_parent_class)->shutdown(app);
}

static void
gnc_application_init (GncApplication *app)
{
    g_application_add_main_option_entries (G_APPLICATION (app), options);
    {
        g_action_map_add_action_entries (G_ACTION_MAP(app),
                                         gnc_app_actions,
                                         gnc_app_n_actions, app);
    }
}


static void
gnc_main_window_cmd_edit_preferences (GSimpleAction *action,
                                      GVariant      *parameter,
                                      gpointer       window)
{
    gnc_preferences_dialog ();
}

/** This is a helper function to find a data file and suck it into
 *  memory.
 *
 *  @param partial The name of the file relative to the gnucash
 *  specific shared data directory.
 *
 *  @return The text of the file or NULL. The caller is responsible
 *  for freeing this string.
 */
static gchar *
get_file (const gchar *partial)
{
    gchar *filename, *text = NULL;
    gsize length;

    filename = gnc_filepath_locate_doc_file(partial);
    if (filename && g_file_get_contents(filename, &text, &length, NULL))
    {
        if (length)
        {
            g_free(filename);
            return text;
        }
        g_free(text);
    }
    g_free (filename);
    return NULL;
}

/** This is a helper function to find a data file, suck it into
 *  memory, and split it into an array of strings.
 *
 *  @param partial The name of the file relative to the gnucash
 *  specific shared data directory.
 *
 *  @return The text of the file as an array of strings, or NULL. The
 *  caller is responsible for freeing all the strings and the array.
 */
static gchar **
get_file_strsplit (const gchar *partial)
{
    gchar *text, **lines;

    text = get_file(partial);
    g_return_val_if_fail(text, NULL);

    lines = g_strsplit(text, " ", 0);
    g_free(text);
    g_return_val_if_fail(lines, NULL);
    {
        const gsize num_elems = g_strv_length (lines);
        guint i;
        for (i = 0; i < num_elems; ++i) {
            gchar *word = lines[i];
            if (!word || word[0] == '\0' || strlen(word) > 1)
                continue;
            if (word[0] == '&') {
                g_free(word);
                word = g_strdup("&amp;");
            }
        };
    }
    text = g_strjoinv(" ", lines);
    g_strfreev(lines);
    g_return_val_if_fail(text, NULL);
    lines = g_strsplit_set(text, "\r\n", 0);
    g_free(text);
    return lines;
}
/** URL activation callback.
 *  Use our own function to activate the URL in the users browser
 *  instead of gtk_show_uri(), which requires gvfs.
 *  Signature described in gtk docs at GtkAboutDialog activate-link signal.
 */

static gboolean
url_signal_cb (GtkAboutDialog *dialog, gchar *uri, gpointer data)
{
    gnc_launch_assoc (uri);
    return TRUE;
}

/** Create and display the "about" dialog for gnucash.
 *
 *  @param action The GAction for the "about" menu item.
 *
 *  @param window The main window whose menu item was activated.
 */
static void
gnc_main_window_cmd_help_about (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       app)
{
    static GtkWidget *about_dialog = NULL;

    if (about_dialog == NULL)
    {
        const gchar *fixed_message = _("The GnuCash personal finance manager. "
                                   "The GNU way to manage your money!");
	    const gchar *copyright = _("© 1997-2016 Contributors");
	    gchar **authors = get_file_strsplit("AUTHORS");
	    gchar **documenters = get_file_strsplit("DOCUMENTERS");
	    gchar *license = get_file("LICENSE");
	    gchar *message;

#ifdef GNUCASH_SCM
        /* Development version */
        /* Translators: 1st %s is a fixed message, which is translated independently;
                        2nd %s is the scm type (svn/svk/git/bzr);
                        3rd %s is the scm revision number;
                        4th %s is the build date */
            message = g_strdup_printf(_("%s\nThis copy was built from %s rev %s on %s."),
                                      fixed_message, GNUCASH_SCM, GNUCASH_SCM_REV,
                                      GNUCASH_BUILD_DATE);
#else
        /* Translators: 1st %s is a fixed message, which is translated independently;
                        2nd %s is the scm (svn/svk/git/bzr) revision number;
                        3rd %s is the build date */
        message = g_strdup_printf(_("%s\nThis copy was built from rev %s on %s."),
                                  fixed_message, GNUCASH_SCM_REV,
                                  GNUCASH_BUILD_DATE);
#endif
        about_dialog = gtk_about_dialog_new ();
        g_object_set (about_dialog,
                  "authors", authors,
                  "documenters", documenters,
                  "comments", message,
                  "copyright", copyright,
                  "license", license,
                  "name", "GnuCash",
         /* Translators: the following string will be shown in Help->About->Credits
          * Enter your name or that of your team and an email contact for feedback.
          * The string can have multiple rows, so you can also add a list of
          * contributors. */
                          "translator-credits", _("translator_credits"),
                          "version", VERSION,
                          "website", "http://www.gnucash.org",
                          NULL);

        g_free(message);
        if (license)     g_free(license);
        if (documenters) g_strfreev(documenters);
        if (authors)     g_strfreev(authors);
        g_signal_connect (about_dialog, "activate-link",
                  G_CALLBACK(url_signal_cb), NULL);
        g_signal_connect (about_dialog, "response",
                  G_CALLBACK(gtk_widget_hide), NULL);
        gtk_window_set_transient_for (GTK_WINDOW (about_dialog),
                          GTK_WINDOW (gtk_application_get_active_window(app)));
    }
    gtk_dialog_run (GTK_DIALOG (about_dialog));
}


static void
gnc_main_window_cmd_help_tutorial (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       window)
{
    gnc_gnome_help (HF_GUIDE, NULL);
}

static void
gnc_main_window_cmd_help_contents (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       window)
{
    gnc_gnome_help (HF_HELP, NULL);
}

static void
gnc_main_window_cmd_file_quit (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       app)
{
    if (!gnc_main_window_all_finish_pending())
        return;

    g_application_quit(G_APPLICATION(app));
}

static gint
gnc_application_handle_local_options (GApplication *app,
                                      GVariantDict *dict)
{
    gnc_prefs_set_debugging(debugging);
    gnc_prefs_set_extra(extra);

    if (gsettings_prefix)
        gnc_gsettings_set_prefix(g_strdup(gsettings_prefix));

    if (namespace_regexp)
        gnc_prefs_set_namespace_regexp(namespace_regexp);

    if (gnucash_show_version)
    {
        gchar *fixed_message;
        gchar *rev;

        if (is_development_version)
        {
            fixed_message = g_strdup_printf(_("GnuCash %s development version"), VERSION);
            rev = g_strdup_printf("%s ", GNUCASH_SCM);
        }
        else
        {
            fixed_message = g_strdup_printf(_("GnuCash %s"), VERSION);
            rev = g_strdup("");
        }
        /* Translators: 1st %s is a fixed message, which is translated independently;
                        2nd %s is the scm type (svn/svk/git/bzr) or nothing if not a development version;
                        3rd %s is the scm revision number;
                        4th %s is the build date */
        g_print ( _("%s\nThis copy was built from %srev %s on %s.\n"),
                  fixed_message, rev, GNUCASH_SCM_REV,
                  GNUCASH_BUILD_DATE );
        g_free (fixed_message);
        g_free (rev);
        return 0; // Exit, Success
    }

    return -1; // Continue
}

static void
gnc_application_startup (GApplication *app)
{
    G_APPLICATION_CLASS (gnc_application_parent_class)->startup(app);
    g_application_mark_busy (g_application_get_default());

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

    gnc_log_init(log_to_filename, (const gchar**)log_flags);

    /* Now the module files are looked up, which might cause some library
    initialization to be run, hence gtk must be initialized beforehand. */
    gnc_module_system_init();

    //gnc_file_set_shutdown_callback (gnc_shutdown);
    id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 10000, /* 10 secs */
                             gnc_ui_check_events, NULL, NULL);

    {
        SCM main_mod;

        scm_init_guile();
        //scm_c_eval_string("(debug-set! stack 200000)");
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
    scm_c_use_module("gnucash price-quotes");
    scm_c_eval_string("(gnc:price-quotes-install-sources)");

    gnc_hook_run(HOOK_STARTUP, NULL);

    g_application_unmark_busy (g_application_get_default());

    if (gnc_prefs_get_bool(GNC_PREFS_GROUP_NEW_USER, GNC_PREF_FIRST_STARTUP))
    {
        gnc_ui_new_user_dialog();
    }
    /* Ensure temporary preferences are temporary */
    gnc_prefs_reset_group (GNC_PREFS_GROUP_WARNINGS_TEMP);

    gnc_hook_run(HOOK_UI_POST_STARTUP, NULL);
}

static void
gnc_application_activate (GApplication *app)
{
    GncMainWindow *win;

    win = gnc_main_window_new (GTK_APPLICATION(app));
    gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(win));
    gnc_gui_init(win);
    gtk_window_present (GTK_WINDOW (win));
    gtk_widget_show_all (GTK_WIDGET (win));
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

    // FIXME this
    {
        char* fn = g_file_get_path(files[0]);
        gnc_file_open_file(fn, /*open_readonly*/ FALSE);
        g_free(fn);
    }

    windows = gtk_application_get_windows (GTK_APPLICATION (app));
    if (windows)
        win = GNC_MAIN_WINDOW (windows->data);
    else
        win = gnc_main_window_new (GTK_APPLICATION(app));

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
    G_APPLICATION_CLASS (class)->handle_local_options = gnc_application_handle_local_options;
    G_APPLICATION_CLASS (class)->startup = gnc_application_startup;
    G_APPLICATION_CLASS (class)->shutdown = gnc_application_shutdown;
}

GncApplication *
gnc_application_new (void)
{
     return g_object_new (GNC_TYPE_APPLICATION,
                          "application-id", "org.gnucash",
                          "flags", G_APPLICATION_HANDLES_OPEN,
                          NULL);
}
