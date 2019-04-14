/**
 * Copyright 2018 Jonathan Anderson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if !defined(ASSIGN5_H)
#define ASSIGN5_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Read data from a set of files into a corresponding set of files.
 *
 * For each file in @b inputs, read the entire file's contents into memory
 * and then write them into the corresponding file in @b outputs.
 * Also record the time required to perform the I/O (not including system calls
 * such as `fstat(2)` that deal with metadata rather than data or time required
 * to allocate any needed buffers).
 *
 * If the @b async parameter is `true`, perform the I/O using POSIX asynchronous
 * I/O calls (e.g., `aio_read(2)` instead of `read(2)`).
 *
 * @param      inputs       file descriptors of input files
 * @param      outputs      file descriptors of input files
 * @param      length       the length of both @b inputs and @b outputs
 * @param      async        perform I/O using POSIX asynchronous I/O API
 * @param[out] time         microseconds required to read and write the data
 *
 * @returns    total number of bytes copied
 */
size_t copyFiles(const int inputs[], const int outputs[],
	size_t length, size_t bufferSize, bool async, size_t *time);

#endif /* !defined(ASSIGN5_H) */
