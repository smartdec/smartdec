/* This file was copied as-is from ArXLib with permission of the copyright holder. 
 * See http://code.google.com/p/arxlib/. */

/* This file is part of ArXLib, a C++ ArX Primitives Library.
 *
 * Copyright (C) 2008-2010 Alexander Fokin <apfokin@gmail.com>
 *
 * ArXLib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * ArXLib is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ArXLib. If not, see <http://www.gnu.org/licenses/>. 
 * 
 * $Id: Foreach.h 257 2011-08-09 11:23:04Z ru.elric $ */
#ifndef ARX_FOREACH_H
#define ARX_FOREACH_H

//#include <arx/config.h>
#include <boost/config.hpp> /* For BOOST_NO_AUTO_DECLARATIONS. */
#include <boost/foreach.hpp>
#include <boost/preprocessor/cat.hpp>

/* Use BOOST_TYPEOF when auto is not available. */
#ifdef BOOST_NO_AUTO_DECLARATIONS
#  include <boost/typeof/typeof.hpp>
#  define ARX_FOREACH_AUTO_ELEMENT(NAME, CONTAINER) boost::range_value<BOOST_TYPEOF(CONTAINER)>::type &NAME
#else
#  define ARX_FOREACH_AUTO_ELEMENT(NAME, CONTAINER) auto NAME
#endif

/* Undefine qt foreach. */
#ifdef foreach
#  undef foreach
#endif

#ifdef reverse_foreach
#  undef reverse_foreach
#endif

#define foreach BOOST_FOREACH

#define reverse_foreach BOOST_REVERSE_FOREACH

#define ARX_FOREACH_VAR(NAME) BOOST_PP_CAT(arx_foreach_, BOOST_PP_CAT(NAME, __LINE__))

#define map_foreach(KEY, VAL, MAP)                                              \
  if(bool ARX_FOREACH_VAR(_stop) = false) {} else                               \
  foreach(ARX_FOREACH_AUTO_ELEMENT(pair, MAP), MAP)                             \
  if(ARX_FOREACH_VAR(_stop)) { break; } else                                    \
  if(bool ARX_FOREACH_VAR(_end) = false) {} else                                \
  if((ARX_FOREACH_VAR(_stop) = true), false) {} else                            \
  for(KEY = pair.first; !ARX_FOREACH_VAR(_end); ARX_FOREACH_VAR(_end) = true)   \
  for(VAL = pair.second; !ARX_FOREACH_VAR(_end); ARX_FOREACH_VAR(_stop) = false, ARX_FOREACH_VAR(_end) = true)

#endif // ARX_FOREACH_H
