/**
 *  @file oalplus/alfunc.hpp
 *  @brief Helper macro expanding into OpenAL function name
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once
#ifndef OALPLUS_ALFUNC_1303201702_HPP
#define OALPLUS_ALFUNC_1303201702_HPP

#include <oalplus/config.hpp>

namespace oglplus {

#define OALPLUS_ALFUNC(PREFIX, FUNCNAME) \
	::PREFIX##FUNCNAME

} // namespace oglplus

#endif // include guard
