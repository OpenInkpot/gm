#include <stdio.h>
#include <string.h>
#include <Ecore.h>
#include <Evas.h>
#include "apps.h"
#include "setup.h"
#include "run.h"

static
void run_subshell(void * arg) {
    Ecore_Exe *exe;
    exe = ecore_exe_run((const char *) arg, NULL);
    if(exe)
        ecore_exe_free(exe);
};


void
gm_run_madshelf_books(Evas * e __attribute__((unused))) {
    run_subshell("/usr/bin/madshelf --filter=books");
};


void
gm_run_madshelf_images(Evas * e __attribute__((unused))) {
    run_subshell("/usr/bin/madshelf --filter=image");
};

void
gm_run_etimetool(Evas *e __attribute__((unused))) {
    run_subshell("/usr/bin/etimetool");
};

void
gm_run_applications(Evas *e) {
    run_applications(e, "Applications");
};

void
gm_run_games(Evas *e) {
    run_applications(e, "Games");
};


