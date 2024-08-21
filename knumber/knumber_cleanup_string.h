/*
    SPDX-FileCopyrightText: 2024 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

template<void strDeleter(char *)>
class KNumberCleanupString
{
public:
    static inline void cleanup(char *ptr)
    {
        strDeleter(ptr);
    }
};