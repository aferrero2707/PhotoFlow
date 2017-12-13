/*
    Windows-specific stuff
    Copyright (C) 2008 by Andrew Zabolotny
*/

#include "lensfun/config.h"
#include <windows.h>
#include <glib/gstdio.h>

char *_lf_get_database_dir ()
{
    static gchar *dir = NULL;

    HMODULE h = GetModuleHandle ("liblensfun.dll");
    if (!h)
        return g_strdup ("");

    char buff [FILENAME_MAX];
    if (GetModuleFileName (h, buff, sizeof (buff)) <= 0)
        return g_strdup ("");

    char *eos = strchr (buff, 0);
    while (eos > buff && eos [-1] != '/' && eos [-1] != ':' && eos [-1] != '\\')
        eos--;
    *eos = 0;

    dir = g_build_filename (buff, SYSTEM_DB_PATH, DATABASE_SUBDIR, NULL);

    return dir;
}