/*
  Copyright (c) 2006, 2011, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef MYODBC_MYSQL_H
#define MYODBC_MYSQL_H

#define DONT_DEFINE_VOID

#if (MYSQLCLIENT_STATIC_LINKING)
#if (MYSQL8)
#define WIN32_LEAN_AND_MEAN
#include <my_config.h>
#include <my_sys.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <my_alloc.h>
#include <mysql/service_my_snprintf.h>
#include <mysql/service_mysql_alloc.h>
#include <m_ctype.h>
#include <my_io.h>

#define my_bool bool
#define mysys_end my_end

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef _WIN32
typedef DWORD thread_local_key_t;
typedef CRITICAL_SECTION native_mutex_t;
typedef int native_mutexattr_t;

#else
typedef pthread_key_t thread_local_key_t;
//typedef pthread_mutex_t native_mutex_t;
//typedef pthread_mutexattr_t native_mutexattr_t;
#endif


#else // MYSQL8
#include <my_global.h>
#include <mysql.h>
#include <my_sys.h>
#include <my_list.h>
#include <m_string.h>
#include <mysqld_error.h>
#endif // MYSQL8

#else

#include "include/sys/my_global.h"
#include "include/sys/my_thread.h"
#include <mysql.h>
#include "include/sys_main.h"
#include <mysqld_error.h>
#define myodbc_qsort my_qsort

#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define PSI_NOT_INSTRUMENTED 0

#define MIN_MYSQL_VERSION 40100L
#if MYSQL_VERSION_ID < MIN_MYSQL_VERSION
# error "Connector/ODBC requires v4.1 (or later) of the MySQL client library"
#endif

#ifdef MYSQLCLIENT_STATIC_LINKING

#define my_sys_init my_init
#define myodbc_malloc(A,B) my_malloc(PSI_NOT_INSTRUMENTED,A,B)
#ifndef x_free
#define x_free(A) { void *tmp= (A); if (tmp) my_free((char *) tmp); }
#endif

#else

#define myodbc_malloc(A,B) mysys_malloc(A,B)
#ifndef x_free
#define x_free(A) { void *tmp= (A); if (tmp) mysys_free((char *) tmp); }
#endif

#endif

#define myodbc_mutex_t native_mutex_t
#define myodbc_key_t thread_local_key_t
#define myodbc_realloc(A,B,C) my_realloc(PSI_NOT_INSTRUMENTED,A,B,C)
#define myodbc_memdup(A,B,C) my_memdup(PSI_NOT_INSTRUMENTED,A,B,C)
#define myodbc_strdup(A,B) my_strdup(PSI_NOT_INSTRUMENTED,A,B)
#define myodbc_init_dynamic_array(A,B,C,D) my_init_dynamic_array(A,PSI_NOT_INSTRUMENTED,B,NULL,C,D)
#define myodbc_mutex_lock native_mutex_lock
#define myodbc_mutex_unlock native_mutex_unlock
#define myodbc_mutex_trylock native_mutex_trylock
#define myodbc_mutex_init native_mutex_init
#define myodbc_mutex_destroy native_mutex_destroy
#define sort_dynamic(A,cmp) myodbc_qsort((A)->buffer, (A)->elements, (A)->size_of_element, (cmp))
#define push_dynamic(A,B) insert_dynamic((A),(B))

#ifdef _WIN32
#define myodbc_snprintf snprintf
#else
#define myodbc_snprintf my_snprintf
#endif

  static my_bool inline myodbc_allocate_dynamic(DYNAMIC_ARRAY *array, uint max_elements)
  {
    if (max_elements >= array->max_element)
    {
      uint size;
      uchar *new_ptr;
      size = (max_elements + array->alloc_increment) / array->alloc_increment;
      size *= array->alloc_increment;
      if (array->buffer == (uchar *)(array + 1))
      {
        /*
        In this senerio, the buffer is statically preallocated,
        so we have to create an all-new malloc since we overflowed
        */
        if (!(new_ptr = (uchar *)myodbc_malloc(size *
          array->size_of_element,
          MYF(MY_WME))))
          return 0;
        memcpy(new_ptr, array->buffer,
          array->elements * array->size_of_element);
      }
      else


      if (!(new_ptr = (uchar*)myodbc_realloc(array->buffer, size*
        array->size_of_element,
        MYF(MY_WME | MY_ALLOW_ZERO_PTR))))
        return 1;
      array->buffer = new_ptr;
      array->max_element = size;
    }
    return 0;
  }

  static void inline delete_dynamic_element(DYNAMIC_ARRAY *array, uint idx)
  {
    char *ptr = (char*)array->buffer + array->size_of_element*idx;
    array->elements--;
    memmove(ptr, ptr + array->size_of_element,
      (array->elements - idx)*array->size_of_element);
  }


/* Get rid of defines from my_config.h that conflict with our myconf.h */
#ifdef VERSION
# undef VERSION
#endif
#ifdef PACKAGE
# undef PACKAGE
#endif

/*
  It doesn't matter to us what SIZEOF_LONG means to MySQL's headers, but its
  value matters a great deal to unixODBC, which calculates it differently.

  This causes problems where an application linked against unixODBC thinks
  SIZEOF_LONG == 4, and the driver was compiled thinking SIZEOF_LONG == 8,
  such as on Solaris x86_64 using Sun C 5.8.

  This stems from unixODBC's use of silly platform macros to guess
  SIZEOF_LONG instead of just using sizeof(long).
*/
#ifdef SIZEOF_LONG
# undef SIZEOF_LONG
#endif

#ifdef __cplusplus
}
#endif

#endif

