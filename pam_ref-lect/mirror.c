/*
 * Copyright (c) 2012 - Guilhem Bonnefille <guilhem/bonnefille@gmail.com>
 *
 * This file is part of the ref:lect project. ref:lect is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * ref:lect is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "mirror.h"

#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_ext.h>

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>


int wait_token(char *stored_tag, int delay) {
	// TODO
	return PAM_AUTH_ERR;
}

int check_token(char *stored_tag) {
	DBusGConnection *connection;
	GError *error;
	DBusGProxy *proxy;
	gchar **tags = NULL;
	gchar **tag = NULL;
	gboolean found = FALSE;

	g_type_init ();

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	if (connection == NULL) {
		g_error_free (error);
		return PAM_AUTHINFO_UNAVAIL;
	}

	/* Create a proxy object for the "bus driver" (name "org.freedesktop.DBus") */
	proxy = dbus_g_proxy_new_for_name (connection,
	                                   "org.rfid.Mirror",
	                                   "/org/rfid/mirror",
	                                   "org.rfid.Mirror"); 

	/* Call getTags method, wait for reply */
	error = NULL;
	if (!dbus_g_proxy_call_with_timeout (proxy, "GetTags", 1*60*1000, &error,
	                                     G_TYPE_INVALID,
	                                     G_TYPE_STRV, &tags,
	                                     G_TYPE_INVALID))
	{
		/* Just do demonstrate remote exceptions versus regular GError */
		g_error_free (error);
		g_object_unref (proxy);
		return PAM_AUTHINFO_UNAVAIL;
	}

	/* Check the results */
	tag = tags;
	while (!found && tag != NULL && *tag != NULL && **tag != '\0') {
		if (strcmp (*tag, stored_tag) == 0)
			found = TRUE;
		tag++;
	}

	g_object_unref (proxy);
	return found ? PAM_SUCCESS : PAM_AUTH_ERR;
}
