/*
 * gnc-plugin_page-account-tree.h --
 *
 * Copyright (C) 2003 Jan Arne Petersen <jpetersen@uni-bonn.de>
 * Copyright (C) 2003 David Hampton <hampton@employees.org>
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

/** @addtogroup ContentPlugins
    @{ */
/** @addtogroup GncPluginPageAccountTree An Account Tree Plugin
    @{ */
/** @file gnc-plugin-page-account-tree.h
    @brief Functions providing a chart of account page.
    @author Copyright (C) 2003 Jan Arne Petersen <jpetersen@uni-bonn.de>
    @author Copyright (C) 2003,2005,2006 David Hampton <hampton@employees.org>
*/

#ifndef __GNC_PLUGIN_PAGE_ACCOUNT_TREE_H
#define __GNC_PLUGIN_PAGE_ACCOUNT_TREE_H

#include <gtk/gtk.h>
#include <glib-object.h>

#include "gnc-plugin-page.h"
#include "Account.h"

G_BEGIN_DECLS

/* type macros */
#define GNC_TYPE_PLUGIN_PAGE_ACCOUNT_TREE            (gnc_plugin_page_account_tree_get_type ())
G_DECLARE_DERIVABLE_TYPE(GncPluginPageAccountTree, gnc_plugin_page_account_tree, GNC, PLUGIN_PAGE_ACCOUNT_TREE, GncPluginPage)

#define GNC_PLUGIN_PAGE_ACCOUNT_TREE_NAME "GncPluginPageAccountTree"

/* typedefs & structures */
struct _GncPluginPageAccountTreeClass
{
    GncPluginPageClass gnc_plugin_page;

    /* callbacks */
    void (*account_selected) (GncPluginPage	 *page,
                              Account        *account);
};

/* function prototypes */

/** Create a new "account tree" plugin page.
 *
 *  @return The newly created plugin page.
 */
GncPluginPage *gnc_plugin_page_account_tree_new  (void);


/** Given a pointer to an account tree plugin page, return the
 *  selected account (if any).
 *
 *  @param page The "account tree" page.
 *
 *  @return The currently selected account.  NULL if no account is
 *  selected.
 */
Account * gnc_plugin_page_account_tree_get_current_account (GncPluginPageAccountTree *page);

G_END_DECLS

#endif /* __GNC_PLUGIN_PAGE_ACCOUNT_TREE_H */
/** @} */
/** @} */
