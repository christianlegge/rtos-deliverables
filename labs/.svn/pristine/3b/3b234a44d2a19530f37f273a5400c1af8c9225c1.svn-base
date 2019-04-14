/*-
 * Copyright 2018 Jonathan Anderson
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <pthread.h>
#include <stdlib.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ui.h"
#include "rusage.h"


static void	print_rusage(const char *context, bool use_ui);


int
main(int argc, char *argv[])
{
	print_rusage("beginning of main()", false);

	ui_init(&argc, &argv);
	print_rusage("after ui_init()", false);

	/* TODO: start running `ui_run()` in a separate thread */
	pthread_t *thread = malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, ui_run, NULL);

	sleep(1);

	print_rusage("after starting UI thread", true);

	/* TODO: wait for the UI thread to finish */
	pthread_join(*thread, NULL);
	free(thread);

	print_rusage("after UI thread complete", false);

	return 0;
}


static void
print_rusage(const char *context, bool use_ui)
{
	char buffer[4096];
	get_rusage_string(buffer, sizeof(buffer));

	if (use_ui)
	{
		ui_show(context, "rusage information", buffer);
	} else {
		printf("----\n%s\n----\n%s\n%s\n", context,
		       "rusage:", buffer);
	}
}
