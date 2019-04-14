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
#include <time.h>
#include <stdio.h>


extern void do_stuff(void);


int main(int argc, char *argv[])
{
	struct timespec begin, end;

	printf("Calling do_stuff()!\n");

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin) != 0)
	{
		err(-1, "Failed to get start time");
	}

	do_stuff();

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end) != 0)
	{
		err(-1, "Failed to get end time");
	}
	printf("%ld",end.tv_nsec - begin.tv_nsec);
	printf("\nBack in main() again.\n");

	return 0;
}
