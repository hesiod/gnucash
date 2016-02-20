/*
 * gnc-main-window.h -- GtkWindow which represents the
 *	GnuCash main window.
 *
 * Copyright (C) 2003 Jan Arne Petersen <jpetersen@uni-bonn.de>
 * Copyright (C) 2003,2005 David Hampton <hampton@employees.org>
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

/** @addtogroup Windows
    @{ */
/** @addtogroup GncMainWindow Main Window functions.
    @{ */
/** @file gnc-main-window.h
    @brief Functions for adding content to a window.
    @author Copyright (C) 2003 Jan Arne Petersen <jpetersen@uni-bonn.de>
    @author Copyright (C) 2003,2005 David Hampton <hampton@employees.org>
*/

#ifndef __GNC_MAIN_WINDOW_H
#define __GNC_MAIN_WINDOW_H

#include <gtk/gtk.h>
#include "gnc-plugin-page.h"
#include "gnc-window.h"

#define PLUGIN_PAGE_IMMUTABLE    "page-immutable"

G_BEGIN_DECLS

#define GNC_TYPE_MAIN_WINDOW gnc_main_window_get_type ()
G_DECLARE_DERIVABLE_TYPE (GncMainWindow, gnc_main_window, GNC, MAIN_WINDOW, GtkApplicationWindow)

struct _GncMainWindowClass
{
    GtkApplicationWindowClass parent_class;

    /* callbacks */
    void (*page_added)   (GncMainWindow *window,
                          GncPluginPage *page);
    void (*page_changed) (GncMainWindow *window,
                          GncPluginPage *page);
};

/* typedefs & structures */

typedef struct
{
    GncMainWindow *window;
    gpointer data;
} GncMainWindowActionData;

typedef void (*GncMainWindowFunc) (GncMainWindow *window, GncPluginPage *page);
typedef void (*GncMainWindowPageFunc) (GncPluginPage *page, gpointer user_data);

/* function prototypes */

/** Get the type of a gnc main window.
 *
 *  @return A GType.
 */
//GType gnc_main_window_get_type (void);


/** Create a new gnc main window plugin.
 *
 *  @return A pointer to the new object.
 */
GncMainWindow *gnc_main_window_new (GtkApplication *app);


/** Bring the window containing the specified page to the top of the
 *  window stack, then switch the notebook to show the specified page.
 *
 *  @param page The existing page to be displayed.
 */
void gnc_main_window_display_page (GncPluginPage *page);


/** Display a data plugin page in a window.  If the page already
 *  exists in any window, then that window will be brought to the
 *  front and the notebook switch to display the specified page.  If
 *  the page is new then it will be added to the specified window
 *  (unless the page itself requests otherwise.).  If the window is
 *  NULL, the new page will be added to the first window.
 *
 *  @param window The window to display a new page in.
 *
 *  @param page The new page of data to be displayed, or the existing
 *  page of data the should be brought to the top and displayed.
 */
void gnc_main_window_open_page (GncMainWindow *window,
                                GncPluginPage *page);


/** Remove a data plugin page from a window and display the previous
 *  page.  If the page removed was the last page in the window, and
 *  there is more than one window open, then the entire window will be
 *  destroyed.
 *
 *  @param page The page of data to be removed.
 */
void gnc_main_window_close_page (GncPluginPage *page);


/*  Iterator function to walk all pages in all windows, calling the
 *  specified function for each page.
 *
 *  @param entry A pointer to the function to be called.
 *
 *  @param user_data A data pointer passed to each call of the function.
 */
void gnc_main_window_foreach_page (GtkApplication *app,
                                   GncMainWindowPageFunc fn,
                                   gpointer user_data);

/*  Iterator function to walk all windows, calling the
 *  specified function for each window.
 *
 *  @param entry A pointer to the function to be called.
 *
 *  @param user_data A data pointer passed to each call of the function.
 */
void
gnc_main_window_foreach_window (GtkApplication *app,
                                GFunc           fn,
                                gpointer        user_data);

/** Retrieve a pointer to the page that is currently at the front of
 *  the specified window.  Any plugin that needs to manipulate its
 *  menus based upon the currently selected menu page should connect
 *  to the "page_changed" signal on a window.  The callback function
 *  from that signal can then call this function to obtain a pointer
 *  to the current page.
 *
 *  @param window A pointer to the window whose front page should be
 *  returned.
 *
 *  @return A pointer to the GncPluginPage currently at the front of
 *  the specified window.  If the window pointer is invalid or the
 *  window is empty, this function will return NULL.
 */
GncPluginPage *gnc_main_window_get_current_page (GncMainWindow *window);


/** Returns the pointer to the GtkBuilder which is used for the menu
 * item merging. */
EggMenuManager *gnc_main_window_get_uimanager (GncMainWindow *window);


/** Update the name of the page in the main window.
 *
 *  @param page The page to be updated.
 *  @param name_in The new name for the page.
*/
void
main_window_update_page_name (GncPluginPage *page,
                              const gchar *name_in);


/** Update the color on the page tabs in the main window.
 *
 *  @param page The page to be updated.
 *  @param color_in The new color string for the page tab.
*/
void
main_window_update_page_color (GncPluginPage *page,
                               const gchar *color_in);


/** Manually add a set of actions to the specified window.  Plugins
 *  whose user interface is not hard coded (e.g. the menu-additions *
 *  plugin) must create their actions at run time, then use this *
 *  function to install them into the window.
 *
 *  @param window A pointer to the window whose user interface should
 *  be updated.
 *
 *  @param group_name The name for this set of actions.  This name
 *  should be unique among all groups added to the window, and will be
 *  needed to remove the actions from this window.
 *
 *  @param group A pointer to an array of GtkActions.  These are the
 *  actions that will be added to the user interface.
 *
 *  @param merge_id A merge identifier retrieved from a call to
 *  gtk_ui_manager_new_merge_id().
 */
void gnc_main_window_manual_merge_actions (GncMainWindow *window,
        const gchar *group_name,
        guint merge_id);


/** Add a set of actions to the specified window.  This function
 *  should not need to be called directly by plugin implementors.
 *  Correctly assigning values to the GncPluginClass fields during
 *  plugin initialization will cause this routine to be automatically
 *  called.
 *
 *  @param window A pointer to the window whose user interface should
 *  be updated.
 *
 *  @param group_name The name for this set of actions.  This name
 *  should be unique among all groups added to the window, and will be
 *  needed to remove the actions from this window.
 *
 *  @param entries A pointer to an array of GtkActionEntry.  These
 *  are the actions that will be added to the user interface.
 *
 *  @param n_entries The number of actions in the array.
 *
 *  @param toggle_entries A pointer to an array of GtkToggleActionEntry.
 *  These are the toggle actions that will be added to the user interface.
 *
 *  @param n_toggle_entries The number of toggle actions in the array.
 *
 *  @param filename The filename containing the user interface
 *  definition that goes with this set of actions.
 *
 *  @param user_data The data to be provided to all callback
 *  functions.
 */
void gnc_main_window_merge_actions (GncMainWindow *window,
                                    const gchar *group_name,
                                    GActionEntry *entries,
                                    guint n_entries,
                                    const gchar *filename,
                                    gpointer user_data);


/** Remove a set of actions from the specified window.  This function
 *  should not need to be called directly by plugin implementors.  It
 *  will automatically be called when a plugin is removed from a
 *  window.
 *
 *  @param window A pointer to the window whose user interface should
 *  be updated.
 *
 *  @param group_name The name for this set of actions.  This must be
 *  the same name provided when the actions were installed.
 */
void gnc_main_window_unmerge_actions (GncMainWindow *window,
                                      const gchar *group_name);


/** Force a full update of the user interface for the specified
 *  window.  This can be an expensive function, but is needed because
 *  the gtk ui manager doesn't always seem to update properly when
 *  actions are changed.
 *
 *  @param window A pointer to the window whose user interface should
 *  be updated.
 *
 *  @attention Is this function still needed?
 */
void gnc_main_window_actions_updated (GncMainWindow *window);


/** Set the window where all progressbar updates should occur.  This
 *  is a wrapper around the gnc_window_set_progressbar_window()
 *  function.
 *
 *  @param window The window to use for all progressbar updates.
 */
void gnc_main_window_set_progressbar_window( GncMainWindow *window );


/** Callback function invoked when the user clicks in the content of
 *  any Gnucash window.  If this was a "right-click" then Gnucash will
 *  popup the contextual menu.
 *
 *  @param whatever Whatever widget had focus when the user issued the
 *  keyboard context-menu request.
 *
 *  @param event The event parameter describing where on the screen
 *  the mouse was pointing when clicked, type of click, modifiers,
 *  etc.
 *
 *  @param page This is the GncPluginPage corresponding to the visible
 *  page.
 *
 *  @return Returns TRUE if this was a right-click, meaning Gnucash
 *  handled the click.
 */
gboolean gnc_main_window_button_press_cb (GtkWidget *whatever,
        GdkEventButton *event,
        GncPluginPage *page);

/** Restore the persistent state of all windows.
 *
 *  @param keyfile The GKeyFile containing persistent window state.
 */
void gnc_main_window_restore_all_windows(const GKeyFile *keyfile);

/** Save the persistent state of all windows.
 *
 *  @param keyfile The GKeyFile to contain persistent window state.
 */
void gnc_main_window_save_all_windows(GKeyFile *keyfile);

/** Restore the persistent state of one window to a sane default.
 */
void gnc_main_window_restore_default_state(GncMainWindow *window);


/** Tell a window to finish any outstanding activities.  This function
 *  will call gnc_plugin_page_finish_pending for each installed page.
 *  If any page returns a failure indication, then the function stops
 *  walking pages and immediately returns a failure.
 *
 *  @param window Whe window whose pages should be checked.
 *
 *  @return FALSE if any page could not or would not comply, which
 *  should cancel the pending operation.  TRUE otherwise */
gboolean gnc_main_window_finish_pending (GncMainWindow *window);


/** Tell all pages in all windows to finish any outstanding
 *  activities.  This function will call
 *  gnc_plugin_page_finish_pending for each installed page.  If any
 *  page returns a failure indication, then the function stops walking
 *  pages and immediately returns a failure.
 *
 *  @return FALSE if any page could not or would not comply, which
 *  should cancel the pending operation.  TRUE otherwise */
gboolean gnc_main_window_all_finish_pending (void);

/** Change the sensitivity of a command in all windows.  This can be
 *  used to serialize access to a command so that in cannot be
 *  reinvoked until the current invocation is finished.
 *
 *  @param action_name The name of the command to modity.
 *
 *  @param sensitive Whether or not the user should be able to invoke
 *  this action. */
void gnc_main_window_all_action_set_sensitive (const gchar *action_name, gboolean sensitive);

/** Find action in main window.
 *
 *  @param window Whe window which should be checked for the action.
 *
 *  @param name The name of the command to be retrieved.
 *
 *  @return A pointer to a GtkAction that was added with the
 *  specified name. If the name cannot be found, then NULL will be
 *  returned.
 */
GAction *gnc_main_window_find_action (GncMainWindow *window, const gchar *name);

/**
 * Shows all main windows.
 **/
void gnc_main_window_show_all_windows(void);

/**
 * Opens the Book Options dialog.
 *
 *  @param modal True to open in modal mode, false otherwise.
 *
 *  @param title Title of the dialog; "Book Options" if NULL.
 *
 *  @return A pointer to the GtkWidget for the dialog that can be used
 *  when started in modal mode.
 **/
GtkWidget *gnc_book_options_dialog_cb (gboolean modal, gchar *title);

/**
 * Processes selected options in the Book Options dialog: checks book_currency
 * and use_split_action_for_num to see if features kvp shuold be set. To be used
 * where ever a new book situation requires book option selection (e.g., not
 * just in Book Options dialog opened from main window but also in new-file
 * assistant).
 *
 *  @param GNCOptionDB * options.
 *
 **/
void gnc_book_options_dialog_apply_helper(GNCOptionDB * options);

G_END_DECLS

#endif /* __GNC_MAIN_WINDOW_H */

/** @} */
/** @} */
