# Copyright 2018 Jonathan Anderson
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


	# The .data section of an object file holds, well, data! Names defined
	# in this section will appear in the symbol table as global variables.
	.data

message:
	.ascii "Hello, world!\n"

	# The .text section contains instructions. The name is "text" for...
	# historical reasons.
	.text

	# Here's how we define a function in assembly! This function doesn't
	# take any parameters, so we just start executing instructions.
	.global	do_stuff

do_stuff:
	mov	$message, %eax
	int	$0x80

	ret

