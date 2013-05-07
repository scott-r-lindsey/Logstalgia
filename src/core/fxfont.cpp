/*
    Copyright (c) 2009 Andrew Caudwell (acaudwell@gmail.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
       derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "fxfont.h"

FXFontManager fontmanager;

FXFont::FXFont() {
    ft = 0;
}

FXFont::FXFont(FTFont* ft) {
    this->ft = ft;
    init();
}

void FXFont::init() {
    shadow          = false;
    shadow_strength = 0.7;
    shadow_offset   = vec2f(1.0, 1.0);

    round           = false;
    align_right     = false;
    align_top       = true;
}

FTFont* FXFont::getFTFont() const{
    return ft;
}

void FXFont::roundCoordinates(bool round) {
    this->round = round;
}

void FXFont::alignRight(bool align_right) {
    this->align_right = align_right;
}

void FXFont::alignTop(bool align_top) {
    this->align_top = align_top;
}

void FXFont::shadowStrength(float s) {
    shadow_strength = s;
}

void FXFont::shadowOffset(float x, float y) {
    shadow_offset = vec2f(x,y);
}

void FXFont::dropShadow(bool shadow) {
    this->shadow = shadow;
}

int FXFont::getFontSize() const{
    return ft->FaceSize();
}

float FXFont::getHeight() const{
    return ft->Ascender() + ft->Descender();
}

float FXFont::getWidth(const std::string & text) const{
    FTBBox bb = ft->BBox(text.c_str());

    float width = (bb.Upper().X() - bb.Lower().X());

    return width;
}

void FXFont::render(float x, float y, const std::string & text) const{


    if(round) {
        x = roundf(x);
        y = roundf(y);
    }

#ifdef _RPI
    //FIXME
#else
    glPushMatrix();
        glTranslatef(x,y,0.0f);
        glScalef(1.0, -1.0, 1.0f);
        ft->Render(text.c_str());
    glPopMatrix();
#endif
}

void FXFont::print(float x, float y, const char *str, ...) const{
    char buf[4096];

    va_list vl;

    va_start (vl, str);
    vsnprintf (buf, 4096, str, vl);
    va_end (vl);

    std::string text = std::string(buf);

    draw(x, y, text);
}

void FXFont::draw(float x, float y, const std::string & text) const{

    if(align_right) {
        x -= getWidth(text);
    }

    if(align_top) {
        y += getHeight();
    }

    if(shadow) {
#ifdef _RPI
    //FIXME
#else
        glPushAttrib(GL_CURRENT_BIT);
        vec4f current = display.currentColour();
        glColor4f(0.0f, 0.0f, 0.0f, shadow_strength * current.w);
        render(x + shadow_offset.x, y + shadow_offset.y, text);
        glPopAttrib();
#endif
    }

    render(x, y, text);
}

// FXFontManager

void FXFontManager::setDir(std::string font_dir) {
    this->font_dir = font_dir;
}

void FXFontManager::purge() {

    for(std::map<std::string,fontSizeMap*>::iterator it = fonts.begin(); it!=fonts.end();it++) {
        fontSizeMap* sizemap = it->second;

        for(fontSizeMap::iterator ft_it = sizemap->begin(); ft_it != sizemap->end(); ft_it++) {
            delete ft_it->second;
        }
        delete sizemap;
    }

    fonts.clear();
}

FXFont FXFontManager::grab(std::string font_file, int size) {

    char buf[256];

    if(font_dir.size()>0 && font_file[0] != '/') {
        font_file = font_dir + font_file;
    }

    //sprintf(buf, "%s:%i", font_file.c_str(), size);
    //std::string font_key = std::string(buf);

    fontSizeMap* sizemap = fonts[font_file];
    
    if(!sizemap) {
        sizemap = fonts[font_file] = new fontSizeMap;
    }
    
    fontSizeMap::iterator ft_it = sizemap->find(size);   
    FTFont* ft;
    
    if(ft_it == sizemap->end()) {  
        ft = create(font_file, size);
        sizemap->insert(std::pair<int,FTFont*>(size,ft));
    } else {
        ft = ft_it->second;
    }

    return FXFont(ft);
}

FTFont* FXFontManager::create(std::string font_file, int size) {

    FTFont* ft = new  FTTextureFont(font_file.c_str());

    if(ft->Error() || !ft->FaceSize(size)) {
        delete ft;
        throw FXFontException(font_file);
    }

    return ft;
}
