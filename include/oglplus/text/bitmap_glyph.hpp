/**
 *  @file oglplus/text/bitmap_glyph.hpp
 *  @brief Bitmap-font-based text rendering.
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2010-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once
#ifndef OGLPLUS_TEXT_BITMAP_GLYPH_HPP
#define OGLPLUS_TEXT_BITMAP_GLYPH_HPP

#include <oglplus/config.hpp>

#include <oglplus/text/bitmap_glyph/rendering.hpp>
#include <oglplus/text/bitmap_glyph/font.hpp>
#include <oglplus/text/bitmap_glyph/font_essence.hpp>

namespace oglplus {
namespace text {

typedef BitmapGlyphFontTpl<BitmapGlyphFontEssence> BitmapGlyphFont;
typedef BitmapGlyphLayoutTpl<BitmapGlyphFont> BitmapGlyphLayout;
typedef BitmapGlyphRenderingTpl<BitmapGlyphFont> BitmapGlyphRendering;

} // namespace text
} // namespace oglplus

#endif // include guard
