#ifndef AWS_COMMON_STRING_H
#define AWS_COMMON_STRING_H
/*
 * Copyright 2010-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#include <aws/common/byte_buf.h>
#include <aws/common/common.h>

/**
 * Represents an immutable string holding either text or binary data. If the
 * string is in constant memory or memory that should otherwise not be freed by
 * this struct, set allocator to NULL and destroy function will be a no-op.
 *
 * This is for use cases where the entire struct and the data bytes themselves
 * need to be held in dynamic memory, such as when held by a struct
 * aws_hash_table. The data bytes themselves are always held in contiguous
 * memory immediately after the end of the struct aws_string, and the memory for
 * both the header and the data bytes is allocated together.
 *
 * Use the aws_string_bytes function to access the data bytes. A null byte is
 * always included immediately after the data but not counted in the length, so
 * that the output of aws_string_bytes can be treated as a C-string in cases
 * where none of the the data bytes are null.
 *
 * Note that the fields of this structure are const; this ensures not only that
 * they cannot be modified, but also that you can't assign the structure using
 * the = operator accidentally.
 */

/* Using a flexible array member is the C99 compliant way to have the bytes of
 * the string immediately follow the header.
 *
 * MSVC doesn't know this for some reason so we need to use a pragma to make
 * it happy.
 */
#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
struct aws_string {
    struct aws_allocator *const allocator;
    const size_t len;
    const uint8_t bytes[];
};
#ifdef _MSC_VER
#    pragma warning(pop)
#endif

AWS_EXTERN_C_BEGIN

/**
 * Returns true if bytes of string are the same, false otherwise.
 */
AWS_COMMON_API
bool aws_string_eq(const struct aws_string *a, const struct aws_string *b);

/**
 * Returns true if bytes of string are equivalent, using a case-insensitive comparison.
 */
AWS_COMMON_API
bool aws_string_eq_ignore_case(const struct aws_string *a, const struct aws_string *b);

/**
 * Returns true if bytes of string and cursor are the same, false otherwise.
 */
AWS_COMMON_API
bool aws_string_eq_byte_cursor(const struct aws_string *str, const struct aws_byte_cursor *cur);

/**
 * Returns true if bytes of string and cursor are equivalent, using a case-insensitive comparison.
 */
AWS_COMMON_API
bool aws_string_eq_byte_cursor_ignore_case(const struct aws_string *str, const struct aws_byte_cursor *cur);

/**
 * Returns true if bytes of string and buffer are the same, false otherwise.
 */
AWS_COMMON_API
bool aws_string_eq_byte_buf(const struct aws_string *str, const struct aws_byte_buf *buf);

/**
 * Returns true if bytes of string and buffer are equivalent, using a case-insensitive comparison.
 */
AWS_COMMON_API
bool aws_string_eq_byte_buf_ignore_case(const struct aws_string *str, const struct aws_byte_buf *buf);

AWS_COMMON_API
bool aws_string_eq_c_str(const struct aws_string *str, const char *c_str);

/**
 * Returns true if bytes of strings are equivalent, using a case-insensitive comparison.
 */
AWS_COMMON_API
bool aws_string_eq_c_str_ignore_case(const struct aws_string *str, const char *c_str);

/**
 * Constructor functions which copy data from null-terminated C-string or array of bytes.
 */
AWS_COMMON_API
struct aws_string *aws_string_new_from_c_str(struct aws_allocator *allocator, const char *c_str);
AWS_COMMON_API
struct aws_string *aws_string_new_from_array(struct aws_allocator *allocator, const uint8_t *bytes, size_t len);

/**
 * Allocate a new string with the same contents as the old.
 */
AWS_COMMON_API
struct aws_string *aws_string_new_from_string(struct aws_allocator *allocator, const struct aws_string *str);

/**
 * Deallocate string.
 */
AWS_COMMON_API
void aws_string_destroy(struct aws_string *str);

/**
 * Zeroes out the data bytes of string and then deallocates the memory.
 * Not safe to run on a string created with AWS_STATIC_STRING_FROM_LITERAL.
 */
AWS_COMMON_API
void aws_string_destroy_secure(struct aws_string *str);

/**
 * Compares lexicographical ordering of two strings. This is a binary
 * byte-by-byte comparison, treating bytes as unsigned integers. It is suitable
 * for either textual or binary data and is unaware of unicode or any other byte
 * encoding. If both strings are identical in the bytes of the shorter string,
 * then the longer string is lexicographically after the shorter.
 *
 * Returns a positive number if string a > string b. (i.e., string a is
 * lexicographically after string b.) Returns zero if string a = string b.
 * Returns negative number if string a < string b.
 */
AWS_COMMON_API
int aws_string_compare(const struct aws_string *a, const struct aws_string *b);

/**
 * A convenience function for sorting lists of (const struct aws_string *) elements. This can be used as a
 * comparator for aws_array_list_sort. It is just a simple wrapper around aws_string_compare.
 */
AWS_COMMON_API
int aws_array_list_comparator_string(const void *a, const void *b);

/**
 * Defines a (static const struct aws_string *) with name specified in first
 * argument that points to constant memory and has data bytes containing the
 * string literal in the second argument.
 *
 * GCC allows direct initilization of structs with variable length final fields
 * However, this might not be portable, so we can do this instead
 * This will have to be updated whenever the aws_string structure changes
 */
#define AWS_STATIC_STRING_FROM_LITERAL(name, literal)                                                                  \
    static const struct {                                                                                              \
        struct aws_allocator *const allocator;                                                                         \
        const size_t len;                                                                                              \
        const uint8_t bytes[sizeof(literal)];                                                                          \
    } name##_s = {NULL, sizeof(literal) - 1, literal};                                                                 \
    static const struct aws_string *(name) = (struct aws_string *)(&name##_s)

/*
 * A related macro that declares the string pointer without static, allowing it to be externed as a global constant
 */
#define AWS_STRING_FROM_LITERAL(name, literal)                                                                         \
    static const struct {                                                                                              \
        struct aws_allocator *const allocator;                                                                         \
        const size_t len;                                                                                              \
        const uint8_t bytes[sizeof(literal)];                                                                          \
    } name##_s = {NULL, sizeof(literal) - 1, literal};                                                                 \
    const struct aws_string *(name) = (struct aws_string *)(&name##_s)

/**
 * Copies all bytes from string to buf.
 *
 * On success, returns true and updates the buf pointer/length
 * accordingly. If there is insufficient space in the buf, returns
 * false, leaving the buf unchanged.
 */
AWS_COMMON_API
bool aws_byte_buf_write_from_whole_string(
    struct aws_byte_buf *AWS_RESTRICT buf,
    const struct aws_string *AWS_RESTRICT src);

/**
 * Creates an aws_byte_cursor from an existing string.
 */
AWS_COMMON_API
struct aws_byte_cursor aws_byte_cursor_from_string(const struct aws_string *src);

/**
 * If the string was dynamically allocated, clones it. If the string was statically allocated (i.e. has no allocator),
 * returns the original string.
 */
AWS_COMMON_API
struct aws_string *aws_string_clone_or_reuse(struct aws_allocator *allocator, const struct aws_string *str);

/* Computes the length of a c string in bytes assuming the character set is either ASCII or UTF-8. If no NULL character
 * is found within max_read_len of str, AWS_ERROR_C_STRING_BUFFER_NOT_NULL_TERMINATED is raised. Otherwise, str_len
 * will contain the string length minus the NULL character, and AWS_OP_SUCCESS will be returned. */
AWS_COMMON_API
int aws_secure_strlen(const char *str, size_t max_read_len, size_t *str_len);

/**
 * Equivalent to str->bytes.
 */
AWS_STATIC_IMPL
const uint8_t *aws_string_bytes(const struct aws_string *str);

/**
 * Evaluates the set of properties that define the shape of all valid aws_string structures.
 * It is also a cheap check, in the sense it run in constant time (i.e., no loops or recursion).
 */
AWS_STATIC_IMPL
bool aws_string_is_valid(const struct aws_string *str);

/**
 * Best-effort checks aws_string invariants, when the str->len is unknown
 */
AWS_STATIC_IMPL
bool aws_c_string_is_valid(const char *str);

#ifndef AWS_NO_STATIC_IMPL
#    include <aws/common/string.inl>
#endif /* AWS_NO_STATIC_IMPL */

AWS_EXTERN_C_END

#endif /* AWS_COMMON_STRING_H */
