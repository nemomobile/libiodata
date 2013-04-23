/*-----------------------------------------------------------------------+
|                                                                        |
|   Copyright (C) 2013 Jolla Ltd.                                        |
|   Contact: Petri M. Gerdt <petri.gerdt@jollamobile.com>                |
|                                                                        |
|   This file is part of Iodata                                          |
|                                                                        |
|   Iodata is free software; you can redistribute it and/or modify       |
|   it under the terms of the GNU Lesser General Public License          |
|   version 2.1 as published by the Free Software Foundation.            |
|                                                                        |
|   Iodata is distributed in the hope that it will be useful, but        |
|   WITHOUT ANY WARRANTY;  without even the implied warranty  of         |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                 |
|   See the GNU Lesser General Public License  for more details.         |
|                                                                        |
|   You should have received a copy of the GNU  Lesser General Public    |
|   License along with Iodata. If not, see http://www.gnu.org/licenses/  |
|                                                                        |
+-----------------------------------------------------------------------*/

#ifndef LOG_H
#define LOG_H

#include <QtGlobal>
#include <cassert>
#include <cstdio>

#define LOG_NONE     0
#define LOG_ASSERT   1
#define LOG_CRITICAL 2
#define LOG_ERROR    3
#define LOG_WARNING  4
#define LOG_NOTICE   5
#define LOG_INFO     6
#define LOG_DEBUG    7
#define LOG_FULL     LOG_DEBUG

#define LOG_LEVEL LOG_WARNING

#if LOG_LEVEL >= LOG_DEBUG
# define log_debug(FMT,ARGS...) \
    do { \
        printf("DEBUG: %s:%d: %s", __FILE__, __LINE__, Q_FUNC_INFO); \
        if (strlen(""FMT) > 0) \
            printf(":\n- "FMT"\n", ## ARGS); \
        else \
            printf("\n"); \
    } while(0)
#else
# define log_debug(FMT,ARGS...) do { } while(0)
#endif

#if LOG_LEVEL >= LOG_INFO
# define log_info(FMT,ARGS...) do { fprintf(stderr, "INFO: "FMT"\n", ## ARGS); } while(0)
#else
# define log_info(FMT,ARGS...) do { } while(0)
#endif

#if LOG_LEVEL >= LOG_INFO
# define log_notice(FMT,ARGS...) do { fprintf(stderr, "NOTICE: "FMT"\n", ## ARGS); } while(0)
#else
# define log_notice(FMT,ARGS...) do { } while(0)
#endif

#if LOG_LEVEL >= LOG_WARNING
# define log_warning(FMT,ARGS...) do { fprintf(stderr, "WARNING: "FMT"\n", ## ARGS); } while(0)
#else
# define log_warning(FMT,ARGS...) do { } while(0)
#endif

#if LOG_LEVEL >= LOG_ERROR
# define log_error(FMT,ARGS...) do { fprintf(stderr, "ERROR: "FMT"\n", ## ARGS); } while(0)
#else
# define log_error(FMT,ARGS...) do { } while(0)
#endif

#if LOG_LEVEL >= LOG_CRITICAL
# define log_critical(FMT,ARGS...) do { fprintf(stderr, "CRITICAL: "FMT"\n", ## ARGS); } while(0)
#else
# define log_critical(FMT,ARGS...) do { } while(0)
#endif

#if LOG_LEVEL >= LOG_ASSERT
# define log_assert(COND, ARGS...) \
    do { \
        if (!(COND)) \
            fprintf(stderr, "ASSERT: "ARGS); \
        assert(COND); \
    } while(0)
# define log_abort(FMT,ARGS...) do { fprintf(stderr, "ABORT: "FMT"\n", ## ARGS); assert(0); } while(0)
#else
# define log_assert(COND, ARGS...) do { assert(COND); } while(0)
# define log_abort(FMT,ARGS...) do { assert(0); } while(0)
#endif

#endif // LOG_H
