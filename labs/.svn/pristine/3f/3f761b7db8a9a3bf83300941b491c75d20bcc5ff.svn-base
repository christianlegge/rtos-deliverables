/*
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

#include <stdbool.h>

/**
 * Run a command, wait for it to finish execution and then report on its
 * resource usage.
 *
 * @param   argc         length of the command line to run @pre > 0
 * @param   argv         command line to run with @b argc elements
 *                       (including command name) followed by a NULL pointer
 * @param   buffer       a string buffer of at least 1 B
 * @param   buffer_len   the length of @b buffer
 *
 * @returns true on success
 */
void get_command_rusage(int argc, char * const argv[], char *buffer,
                        size_t buffer_len);

/**
 * Emit current resource usage information into a string buffer.
 *
 * @param   buffer       a string buffer of at least 1 B
 * @param   buffer_len   the length of @b buffer
 */
void get_rusage_string(char *buffer, size_t buffer_len);
