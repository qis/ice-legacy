/*
 * Written by Solar Designer <solar at openwall.com> in 2000-2011.
 * No copyright is claimed, and the software is hereby placed in the public
 * domain.  In case this attempt to disclaim copyright and place the software
 * in the public domain is deemed null and void, then the software is
 * Copyright (c) 2000-2011 Solar Designer and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See crypt_blowfish.c for more information.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int crypt_magic(const char* setting, char* output, int size);
char* crypt_hash(const char* key, const char* setting, char* output, int size);
char* crypt_salt(const char* prefix, unsigned long count, const char* input, int size, char* output, int output_size);

#ifdef __cplusplus
}  // extern "C"
#endif
