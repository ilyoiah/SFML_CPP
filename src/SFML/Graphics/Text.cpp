////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <utility>

#include <cmath>


namespace
{
// Add an underline or strikethrough line to the vertex array
void addLine(sf::VertexArray& vertices,
             float            lineLeft,
             float            lineRight,
             float            lineTop,
             const sf::Color& color,
             float            offset,
             float            thickness,
             float            outlineThickness = 0)
{
    const float top    = std::floor(lineTop + offset - (thickness / 2) + 0.5f);
    const float bottom = top + std::floor(thickness + 0.5f);

    vertices.append(
        sf::Vertex(sf::Vector2f(lineLeft - outlineThickness, top - outlineThickness), color, sf::Vector2f(1, 1)));
    vertices.append(
        sf::Vertex(sf::Vector2f(lineRight + outlineThickness, top - outlineThickness), color, sf::Vector2f(1, 1)));
    vertices.append(
        sf::Vertex(sf::Vector2f(lineLeft - outlineThickness, bottom + outlineThickness), color, sf::Vector2f(1, 1)));
    vertices.append(
        sf::Vertex(sf::Vector2f(lineLeft - outlineThickness, bottom + outlineThickness), color, sf::Vector2f(1, 1)));
    vertices.append(
        sf::Vertex(sf::Vector2f(lineRight + outlineThickness, top - outlineThickness), color, sf::Vector2f(1, 1)));
    vertices.append(
        sf::Vertex(sf::Vector2f(lineRight + outlineThickness, bottom + outlineThickness), color, sf::Vector2f(1, 1)));
}

// Add a glyph quad to the vertex array
void addGlyphQuad(sf::VertexArray& vertices, sf::Vector2f position, const sf::Color& color, const sf::Glyph& glyph, float italicShear)
{
    const float padding = 1.0;

    const float left   = glyph.bounds.left - padding;
    const float top    = glyph.bounds.top - padding;
    const float right  = glyph.bounds.left + glyph.bounds.width + padding;
    const float bottom = glyph.bounds.top + glyph.bounds.height + padding;

    const float u1 = static_cast<float>(glyph.textureRect.left) - padding;
    const float v1 = static_cast<float>(glyph.textureRect.top) - padding;
    const float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding;
    const float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height) + padding;

    vertices.append(
        sf::Vertex(sf::Vector2f(position.x + left - italicShear * top, position.y + top), color, sf::Vector2f(u1, v1)));
    vertices.append(
        sf::Vertex(sf::Vector2f(position.x + right - italicShear * top, position.y + top), color, sf::Vector2f(u2, v1)));
    vertices.append(
        sf::Vertex(sf::Vector2f(position.x + left - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u1, v2)));
    vertices.append(
        sf::Vertex(sf::Vector2f(position.x + left - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u1, v2)));
    vertices.append(
        sf::Vertex(sf::Vector2f(position.x + right - italicShear * top, position.y + top), color, sf::Vector2f(u2, v1)));
    vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * bottom, position.y + bottom),
                               color,
                               sf::Vector2f(u2, v2)));
}
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
Text::Text(const Font& font, String string, unsigned int characterSize) :
m_string(std::move(string)),
m_font(&font),
m_characterSize(characterSize)
{
}


////////////////////////////////////////////////////////////
void Text::setString(const String& string)
{
    if (m_string != string)
    {
        m_string             = string;
        m_geometryNeedUpdate = true;
    }
}


////////////////////////////////////////////////////////////
void Text::setFont(const Font& font)
{
    if (m_font != &font)
    {
        m_font               = &font;
        m_geometryNeedUpdate = true;
    }
}


////////////////////////////////////////////////////////////
void Text::setCharacterSize(unsigned int size)
{
    if (m_characterSize != size)
    {
        m_characterSize      = size;
        m_geometryNeedUpdate = true;
    }
}


////////////////////////////////////////////////////////////
void Text::setLetterSpacing(float spacingFactor)
{
    if (m_letterSpacingFactor != spacingFactor)
    {
        m_letterSpacingFactor = spacingFactor;
        m_geometryNeedUpdate  = true;
    }
}


////////////////////////////////////////////////////////////
void Text::setLineSpacing(float spacingFactor)
{
    if (m_lineSpacingFactor != spacingFactor)
    {
        m_lineSpacingFactor  = spacingFactor;
        m_geometryNeedUpdate = true;
    }
}


////////////////////////////////////////////////////////////
void Text::setStyle(std::uint32_t style)
{
    if (m_style != style)
    {
        m_style              = style;
        m_geometryNeedUpdate = true;
    }
}


////////////////////////////////////////////////////////////
void Text::setFillColor(const Color& color)
{
    if (color != m_fillColor)
    {
        m_fillColor = color;

        // Change vertex colors directly, no need to update whole geometry
        // (if geometry is updated anyway, we can skip this step)
        if (!m_geometryNeedUpdate)
        {
            for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i)
                m_vertices[i].color = m_fillColor;
        }
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineColor(const Color& color)
{
    if (color != m_outlineColor)
    {
        m_outlineColor = color;

        // Change vertex colors directly, no need to update whole geometry
        // (if geometry is updated anyway, we can skip this step)
        if (!m_geometryNeedUpdate)
        {
            for (std::size_t i = 0; i < m_outlineVertices.getVertexCount(); ++i)
                m_outlineVertices[i].color = m_outlineColor;
        }
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineThickness(float thickness)
{
    if (thickness != m_outlineThickness)
    {
        m_outlineThickness   = thickness;
        m_geometryNeedUpdate = true;
    }
}


////////////////////////////////////////////////////////////
void Text::setLineAlignment(LineAlignment lineAlignment)
{
    if (m_lineAlignment != lineAlignment)
        m_geometryNeedUpdate = true;
    m_lineAlignment = lineAlignment;
}


////////////////////////////////////////////////////////////
const String& Text::getString() const
{
    return m_string;
}


////////////////////////////////////////////////////////////
const Font& Text::getFont() const
{
    return *m_font;
}


////////////////////////////////////////////////////////////
unsigned int Text::getCharacterSize() const
{
    return m_characterSize;
}


////////////////////////////////////////////////////////////
float Text::getLetterSpacing() const
{
    return m_letterSpacingFactor;
}


////////////////////////////////////////////////////////////
float Text::getLineSpacing() const
{
    return m_lineSpacingFactor;
}


////////////////////////////////////////////////////////////
std::uint32_t Text::getStyle() const
{
    return m_style;
}


////////////////////////////////////////////////////////////
const Color& Text::getFillColor() const
{
    return m_fillColor;
}


////////////////////////////////////////////////////////////
const Color& Text::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
float Text::getOutlineThickness() const
{
    return m_outlineThickness;
}


////////////////////////////////////////////////////////////
Text::LineAlignment Text::getLineAlignment() const
{
    return m_lineAlignment;
}


////////////////////////////////////////////////////////////
Vector2f Text::findCharacterPos(std::size_t index) const
{
    // Adjust the index if it's out of range
    if (index > m_string.getSize())
        index = m_string.getSize();

    // Calculate and update the line offsets
    updateLineOffsets();

    // Precompute the variables needed by the algorithm
    const bool isBold                                        = m_style & Bold;
    const auto [whitespaceWidth, letterSpacing, lineSpacing] = getSpacing();

    // Compute the position
    Vector2f      position(m_lineOffsets[0], 0.f); // There will always be at least one line
    std::uint32_t prevChar = 0;
    for (std::size_t i = 0; i < index; ++i)
    {
        const std::uint32_t curChar = m_string[i];

        // Apply the kerning offset
        position.x += m_font->getKerning(prevChar, curChar, m_characterSize, isBold);
        prevChar = curChar;

        // Handle special characters
        switch (curChar)
        {
            case U' ':
                position.x += whitespaceWidth;
                continue;
            case U'\t':
                position.x += whitespaceWidth * 4;
                continue;
            case U'\n':
                position.y += lineSpacing;
                position.x = 0;
                continue;
        }

        // For regular characters, add the advance offset of the glyph
        position.x += m_font->getGlyph(curChar, m_characterSize, isBold).advance + letterSpacing;
    }

    // Transform the position to global coordinates
    position = getTransform().transformPoint(position);

    return position;
}


////////////////////////////////////////////////////////////
FloatRect Text::getLocalBounds() const
{
    ensureGeometryUpdate();

    return m_bounds;
}


////////////////////////////////////////////////////////////
FloatRect Text::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
void Text::draw(RenderTarget& target, const RenderStates& states) const
{
    ensureGeometryUpdate();

    RenderStates statesCopy(states);

    statesCopy.transform *= getTransform();
    statesCopy.texture = &m_font->getTexture(m_characterSize);

    // Only draw the outline if there is something to draw
    if (m_outlineThickness != 0)
        target.draw(m_outlineVertices, statesCopy);

    target.draw(m_vertices, statesCopy);
}


////////////////////////////////////////////////////////////
void Text::ensureGeometryUpdate() const
{
    // Do nothing, if geometry has not changed and the font texture has not changed
    if (!m_geometryNeedUpdate && m_font->getTexture(m_characterSize).m_cacheId == m_fontTextureId)
        return;

    // Save the current fonts texture id
    m_fontTextureId = m_font->getTexture(m_characterSize).m_cacheId;

    // Mark geometry as updated
    m_geometryNeedUpdate = false;

    // Clear the previous geometry
    m_vertices.clear();
    m_outlineVertices.clear();
    m_bounds = FloatRect();

    // No text: nothing to draw
    if (m_string.isEmpty())
        return;

    // Calculate and update the line offsets
    updateLineOffsets();

    // Compute values related to the text style
    const bool  isBold             = m_style & Bold;
    const bool  isUnderlined       = m_style & Underlined;
    const bool  isStrikeThrough    = m_style & StrikeThrough;
    const float italicShear        = (m_style & Italic) ? sf::degrees(12).asRadians() : 0.f;
    const float underlineOffset    = m_font->getUnderlinePosition(m_characterSize);
    const float underlineThickness = m_font->getUnderlineThickness(m_characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    const float strikeThroughOffset = m_font->getGlyph(U'x', m_characterSize, isBold).bounds.getCenter().y;

    // Precompute the variables needed by the algorithm
    const auto [whitespaceWidth, letterSpacing, lineSpacing] = getSpacing();
    float x = m_lineOffsets[0]; // there will always be at least one line
    auto  y = static_cast<float>(m_characterSize);

    // Create one quad for each character
    auto          minX             = static_cast<float>(m_characterSize);
    auto          minY             = static_cast<float>(m_characterSize);
    float         maxX             = 0.f;
    float         maxY             = 0.f;
    std::uint32_t prevChar         = 0;
    std::size_t   line             = 0;
    float         horizontalOffset = x;
    for (std::size_t i = 0; i < m_string.getSize(); ++i)
    {
        const std::uint32_t curChar = m_string[i];

        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            continue;

        // Apply the kerning offset
        x += m_font->getKerning(prevChar, curChar, m_characterSize, isBold);

        // If we're using the underlined style and there's a new line, draw a line
        if (isUnderlined && (curChar == U'\n' && prevChar != U'\n'))
        {
            addLine(m_vertices, horizontalOffset, x, y, m_fillColor, underlineOffset, underlineThickness);

            if (m_outlineThickness != 0)
                addLine(m_outlineVertices, horizontalOffset, x, y, m_outlineColor, underlineOffset, underlineThickness, m_outlineThickness);
        }

        // If we're using the strike through style and there's a new line, draw a line across all characters
        if (isStrikeThrough && (curChar == U'\n' && prevChar != U'\n'))
        {
            addLine(m_vertices, horizontalOffset, x, y, m_fillColor, strikeThroughOffset, underlineThickness);

            if (m_outlineThickness != 0)
                addLine(m_outlineVertices,
                        horizontalOffset,
                        x,
                        y,
                        m_outlineColor,
                        strikeThroughOffset,
                        underlineThickness,
                        m_outlineThickness);
        }

        prevChar = curChar;

        // Handle special characters
        if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t'))
        {
            // Update the current bounds (min coordinates)
            minX = std::min(minX, x);
            minY = std::min(minY, y);

            switch (curChar)
            {
                case U' ':
                    x += whitespaceWidth;
                    break;
                case U'\t':
                    x += whitespaceWidth * 4;
                    break;
                case U'\n':
                    y += lineSpacing;
                    ++line;
                    horizontalOffset = m_lineOffsets[line];
                    x                = horizontalOffset;
                    break;
            }

            // Update the current bounds (max coordinates)
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Apply the outline
        if (m_outlineThickness != 0)
        {
            const Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold, m_outlineThickness);

            // Add the outline glyph to the vertices
            addGlyphQuad(m_outlineVertices, Vector2f(x, y), m_outlineColor, glyph, italicShear);
        }

        // Extract the current glyph's description
        const Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold);

        // Add the glyph to the vertices
        addGlyphQuad(m_vertices, Vector2f(x, y), m_fillColor, glyph, italicShear);

        // Update the current bounds
        const float left   = glyph.bounds.left;
        const float top    = glyph.bounds.top;
        const float right  = glyph.bounds.left + glyph.bounds.width;
        const float bottom = glyph.bounds.top + glyph.bounds.height;

        minX = std::min(minX, x + left - italicShear * bottom);
        maxX = std::max(maxX, x + right - italicShear * top);
        minY = std::min(minY, y + top);
        maxY = std::max(maxY, y + bottom);

        // Advance to the next character
        x += glyph.advance + letterSpacing;
    }

    // If we're using outline, update the current bounds
    if (m_outlineThickness != 0)
    {
        const float outline = std::abs(std::ceil(m_outlineThickness));
        minX -= outline;
        maxX += outline;
        minY -= outline;
        maxY += outline;
    }

    // If we're using the underlined style, add the last line
    if (isUnderlined && (x > 0))
    {
        addLine(m_vertices, horizontalOffset, x, y, m_fillColor, underlineOffset, underlineThickness);

        if (m_outlineThickness != 0)
            addLine(m_outlineVertices, horizontalOffset, x, y, m_outlineColor, underlineOffset, underlineThickness, m_outlineThickness);
    }

    // If we're using the strike through style, add the last line across all characters
    if (isStrikeThrough && (x > 0))
    {
        addLine(m_vertices, horizontalOffset, x, y, m_fillColor, strikeThroughOffset, underlineThickness);

        if (m_outlineThickness != 0)
            addLine(m_outlineVertices, horizontalOffset, x, y, m_outlineColor, strikeThroughOffset, underlineThickness, m_outlineThickness);
    }

    // Update the bounding rectangle
    m_bounds.left   = minX;
    m_bounds.top    = minY;
    m_bounds.width  = maxX - minX;
    m_bounds.height = maxY - minY;
}


////////////////////////////////////////////////////////////
void Text::updateLineOffsets() const
{
    // temporary use m_lineOffsets to store widths of each line
    m_lineOffsets.clear();

    // Precompute the variables needed by the algorithm
    const bool isBold                                        = m_style & Bold;
    const auto [whitespaceWidth, letterSpacing, lineSpacing] = getSpacing();

    Vector2f      position;
    std::uint32_t prevChar = 0;
    float         maxWidth = 0.f;
    for (std::size_t i = 0; i < m_string.getSize(); ++i)
    {
        const std::uint32_t curChar = m_string[i];

        // Apply the kerning offset
        position.x += m_font->getKerning(prevChar, curChar, m_characterSize, isBold);
        prevChar = curChar;

        // Handle special characters
        switch (curChar)
        {
            case U' ':
                position.x += whitespaceWidth;
                continue;
            case U'\t':
                position.x += whitespaceWidth * 4;
                continue;
            case U'\n':
                position.y += lineSpacing;
                if (position.x > maxWidth)
                    maxWidth = position.x;
                m_lineOffsets.push_back(position.x);
                position.x = 0;
                continue;
        }

        // For regular characters, add the advance offset of the glyph
        position.x += m_font->getGlyph(curChar, m_characterSize, isBold).advance + letterSpacing;
    }

    // add final part of the string since last newline as the final line
    // (this is the entire string if text is single line without newlines)
    m_lineOffsets.push_back(position.x);

    // convert widths into offsets depending on line alignment
    for (std::size_t i = 0; i < m_lineOffsets.size(); ++i)
    {
        switch (m_lineAlignment)
        {
            case Right:
                m_lineOffsets[i] = maxWidth - m_lineOffsets[i];
                break;
            case Center:
                m_lineOffsets[i] = (maxWidth - m_lineOffsets[i]) / 2.f;
                break;
            case Left:
                m_lineOffsets[i] = 0.f;
                break;
        }
        m_lineOffsets[i] = std::round(m_lineOffsets[i]);
    }
}


////////////////////////////////////////////////////////////
Text::Spacing Text::getSpacing() const
{
    const bool  isBold          = m_style & Bold;
    float       whitespaceWidth = m_font->getGlyph(U' ', m_characterSize, isBold).advance;
    const float letterSpacing   = (whitespaceWidth / 3.f) * (m_letterSpacingFactor - 1.f);
    whitespaceWidth += letterSpacing;
    const float lineSpacing = m_font->getLineSpacing(m_characterSize) * m_lineSpacingFactor;
    return {whitespaceWidth, letterSpacing, lineSpacing};
}

} // namespace sf
