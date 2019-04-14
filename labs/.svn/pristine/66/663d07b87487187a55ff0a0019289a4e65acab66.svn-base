/*
 * Copyright 2018 Jonathan Anderson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <string.h>

//! Fill up the number of bytes of memory specified.
static int	balloon(size_t bytes);


int main(int argc, char *argv[])
{
	// Prompt user for memory size to fill
	size_t Gigs;

	while (true)
	{
		printf("Enter number of GiB to fill: ");

		char *line;
		size_t n = 0;
		if (getline(&line, &n, stdin) < 0) {
			err(-1, "failed to read number from stdin");
		}

		char *end;
		Gigs = strtol(line, &end, 10);

		free(line);

		if (end != line) {
			break;
		}

		fprintf(stderr, "Invalid number: '%s'\n", line);
	}

	// Create some balloons!
	for (size_t i = 0; i < Gigs; i++) {
		int ret = balloon(1024);
		if(ret == 1){
			break;
		}
	}

	printf("Ballooning complete. Press Enter to quit.");
	getchar();

	return 0;
}

static int	balloon(size_t bytes){
    // long int availablePages = get_avphys_pages();
	// int pageSize = getpagesize();

	// printf("Available memory before balloon: %ld\n", (availablePages*pageSize));
	void *ptr = malloc(bytes);
	if(ptr==NULL){
		printf("No more memory available");
		return 1;
	}
	memset(ptr,0, bytes);

	// availablePages = get_avphys_pages();
	// printf("Available memory after balloon: %ld\n", (availablePages*pageSize));
	return 0;
}
