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
buffer:
	.fill 4096, 1, 0

message:
	.ascii "/etc/fstab"

	# The .text section contains instructions. The name is "text" for...
	# historical reasons.
	.text

	# Here's how we define a function in assembly! This function doesn't
	# take any parameters, so we just start executing instructions.
	.global	do_stuff


do_stuff:
	#part a
	#mov	$4, %eax
	#mov 	$1, %ebx
	#mov	$message, %ecx
	#mov	$20, %edx
	#int 	$0x80

	#part b
	#mov	$1, %rax
	#mov 	$1, %rdi
	#mov	$message, %rsi
	#mov	$20, %r10
	#syscall

	#part d
	#open	
	mov	$2, %rax
	mov 	$message, %rdi
	mov	$0, %rsi
	syscall
	mov	%rax, %r11

	#read
	mov	$0, %rax
	mov 	%r11, %rdi
	mov	$buffer, %rsi
	mov	$4096, %r10
	syscall

	#write	
	mov	$1, %rax
	mov 	$1, %rdi
	mov	$buffer, %rsi
	mov	$4096, %r10
	syscall

	#close	
	mov	$3, %rax
	mov 	%r11, %rdi
	syscall
	ret

