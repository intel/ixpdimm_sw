/*
 * Copyright (c) 2015, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains the definition of common string tokenizer utilities.
 */

#ifndef	_X_STR_H_
#define	_X_STR_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * Tokenize a string such that no token is omitted.
 * @remarks
 * 		Unlike the function @c strtok, which skips the tokens which result from adjacent
 * 		delimiters, this implementation will return every token between every delimiter. @n
 * 		Unlike @c strtok, this function is memoryless and thread-safe, meaning it must be
 * 		passed @c input to receive each token. @n
 * 		Like @c strtok, this function will alter @c input. @n
 * 		This is a platform independent function equivalent to the GCC implementation of @c strsep
 * @param[in,out] input
 * 		A pointer to the @b char array to be tokenized. @n
 * 		@c *input must be null terminated. @n
 * 		After the call, @c *input will point to the next token, while @c **input may be NULL,
 * 		if at the end.
 * @param[in] delims
 * 		A null terminated @b char array containing the delimiters to be used.
 * @return
 * 		A pointer to the first token found in @c *input, otherwise @n
 * 		@b NULL, if @c input == @b NULL, @c *input == @b NULL, @c delims == NULL @n
 */
char *x_strtok(char **input, const char *delims);


#ifdef __cplusplus
}
#endif

#endif /* _X_STR_H_ */
