/**
 * 
 * Hoel database abstraction library
 * 
 * h-private.h: private structures and functions declarations
 * 
 * Copyright 2018-2020 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef __H_PRIVATE_H_
#define __H_PRIVATE_H_

/** Macro to avoid compiler warning when some parameters are unused and that's ok **/
#define UNUSED(x) (void)(x)

/**
 * Add a new struct _h_data * to an array of struct _h_data *, which already has cols columns
 * return H_OK on success
 */
int h_row_add_data(struct _h_data ** result, struct _h_data * data, int cols);

/**
 * Add a new row of struct _h_data * in a struct _h_result *
 * return H_OK on success
 */
int h_result_add_row(struct _h_result * result, struct _h_data * row, int rows);

/**
 * Allocate memory for a new struct _h_data * containing an int
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_int(const long long int value);

/**
 * Allocate memory for a new struct _h_data * containing a double
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_double(const double value);

/**
 * Allocate memory for a new struct _h_data * containing a text
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_text(const char * value, const size_t length);

/**
 * Allocate memory for a new struct _h_data * containing a blob
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_blob(const void * value, const size_t length);

/**
 * Allocate memory for a new struct _h_data * containing a date time structure
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_datetime(const struct tm * datetime);

/**
 * Allocate memory for a new struct _h_data * containing a null value
 * return pointer to the new structure
 * return NULL on error
 */
struct _h_data * h_new_data_null();

#endif /* __H_PRIVATE_H_ */
