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
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


void increment(int *countp);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int
main(int argc, char *argv[])
{
	pthread_t **threads = malloc(sizeof(pthread_t*)*JOBS);
	int **counters = malloc(sizeof(int*)*JOBS);
	int ret = 0;
	int counter = 0;
	struct timespec begin, end;

	// Set C locale settings to get niceties like thousands separators
	// for decimal numbers.
	setlocale(LC_NUMERIC, "");
	
	for(int i = 0; i < JOBS; i++){
		counters[i] = malloc(sizeof(int));
		threads[i] = malloc(sizeof(pthread_t));		
		*counters[i] = 0;
	}

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin) != 0)
	{
		err(-1, "Failed to get start time");
	}

	for (int i = 0; i < JOBS; i++)
	{
		ret = pthread_create(threads[i], NULL, increment, counters[i]);
		if(ret != 0){
			err(-1, "Failed to create thread");
		}
	}

	for (int i = 0; i < JOBS; i++){
		pthread_join(*threads[i], NULL);
	}

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end) != 0)
	{
		err(-1, "Failed to get end time");
	}

	long diff = end.tv_nsec - begin.tv_nsec;
	diff += (1000 * 1000 * 1000) * (end.tv_sec - begin.tv_sec);

	printf("Counted to %'d in %'ld ns: %f ns/iter\n",
	       JOBS * WORK_PER_JOB, diff, ((double) diff) / (JOBS * WORK_PER_JOB));
	
	for (int i = 0; i < JOBS; i++)
	{
		free(counters[i]);
		free(threads[i]);
	}
	free(counters);
	free(threads);

	return 0;
}

void increment(int *countp)
{
	for (int i = 0; i < WORK_PER_JOB; i++)
	{
		pthread_mutex_lock(&mutex);
		(*countp)++;
		pthread_mutex_unlock(&mutex);
	}
}
