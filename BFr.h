#ifndef _BFr_h
#define _BFr_h

/****
 *
 *  BFr     Bitmap Font Renderer for OpenGL 3
 *  Author: Damian Balandowski
 *  Github: https://github.com/DamianPrg/BFr
 *
 ****/

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "FreeImage.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "GL/glew.h"

namespace BFr {
    
    GLuint bfr_program;
    GLuint bfr_vertex;
    GLuint bfr_fragment;
    glm::mat4 bfr_projection;
    
    class Texture
    {
        
    public:
        
        void Load(std::string path)
        {
            textureHandle = FreeImage_Load(FreeImage_GetFileType(path.c_str(), 0), path.c_str());
            
            if(!textureHandle)
            {
                std::cout << "Texture error" << std::endl;
                return;
            }
            
            textureHandle32 = FreeImage_ConvertTo32Bits(textureHandle);
            
            width = FreeImage_GetWidth(textureHandle32);
            height = FreeImage_GetHeight(textureHandle32);
            
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(textureHandle32));

            glGenerateMipmap(GL_TEXTURE_2D);
            
            FreeImage_Unload(textureHandle32);
            
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        int GetWidth() { return width; }
        int GetHeight() { return height; }
        
        GLuint GetGLTexture() { return tex; }
        
    private:
        int width, height;
        
        FIBITMAP* textureHandle;
        FIBITMAP* textureHandle32;
        GLuint   tex;
    };
    
    class VertexBuffer
    {
    public:
        void AddVertexPosition(float x, float y, float z)
        {
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        
        void AddTextureCoord(float x, float y)
        {
            textureCoords.push_back(x);
            textureCoords.push_back(y);
        }
        
        void Clear()
        {
            positions.clear();
            textureCoords.clear();
        }
        
        void Update()
        {
            if(positions.size() >= 3)
            {
                glBindBuffer(GL_ARRAY_BUFFER, position);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * positions.size(), &positions[0]);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            if(textureCoords.size() >= 3)
            {
                glBindBuffer(GL_ARRAY_BUFFER, textureCoord);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * textureCoords.size(), &textureCoords[0]);
                glBindBuffer(GL_ARRAY_BUFFER,0);
            }
        }
        
        void Create()
        {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            
            if(positions.size() >= 3)
            {
                glGenBuffers(1, &position);
                glBindBuffer(GL_ARRAY_BUFFER, position);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), &positions[0], GL_STREAM_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            if(textureCoords.size() >= 3)
            {
                // texcoord
                glGenBuffers(1, &textureCoord);
                glBindBuffer(GL_ARRAY_BUFFER,textureCoord);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float)*textureCoords.size(), &textureCoords[0], GL_STREAM_DRAW);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            
            glBindVertexArray(0);
        }
        
        void Draw()
        {
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(positions.size() / 3));
            glBindVertexArray(0);
        }
    private:
        GLuint vao;
        GLuint position;
        GLuint textureCoord;
        
        std::vector<float> positions;
        std::vector<float> textureCoords;
    };
    
    std::string ContentAfter(std::string src, char afterChar)
    {
        int pos = 0;
        
        int afterCharPos = 0;
        
        while(pos < src.size())
        {
            if(src[pos] == afterChar)
            {
                afterCharPos = pos;
                break;
            }
            
            pos ++;
        }
        
        afterCharPos += 1;
        
        return src.substr(afterCharPos, src.size() - afterCharPos);
    }
    
    struct CharDesc
    {
        int char_id;
        int x;
        int y;
        int w;
        int h;
        int xoffset;
        int yoffset;
        int xadvance;
    };
    
    class Font
    {
    public:
        // path to font without extenstion
        // example: data/myfonts/OpenSans12
        void Load(std::string name)
        {
            chars.clear();
            
            std::string fntPath = name;
            fntPath += ".fnt";
            
            std::string texPath = name;
            texPath += ".png";
            
            std::vector<std::string> contents;
            
            std::ifstream fontfile(fntPath.c_str());
            
            if(fontfile.is_open())
            {
                while(!fontfile.eof())
                {
                    std::string tmp = "";
                    fontfile >> tmp;
                    contents.push_back(tmp);
                }
                
                fontfile.close();
            }
            else
            {
                
                return;
            }
            
            // parse
            for(int i = 0; i < contents.size(); i++)
            {
                if(contents[i] == "char")
                {
                    CharDesc charDesc;
                    charDesc.char_id = atoi(ContentAfter(contents[i+1], '=').c_str());
                    charDesc.x = atoi(ContentAfter(contents[i+2], '=').c_str());
                    charDesc.y = atoi(ContentAfter(contents[i+3], '=').c_str());
                    charDesc.w = atoi(ContentAfter(contents[i+4], '=').c_str());
                    charDesc.h = atoi(ContentAfter(contents[i+5], '=').c_str());
                    charDesc.xoffset = atoi(ContentAfter(contents[i+6], '=').c_str());
                    charDesc.yoffset = atoi(ContentAfter(contents[i+7], '=').c_str());
                    charDesc.xadvance = atoi(ContentAfter(contents[i+8], '=').c_str());

                    chars.push_back(charDesc);
                }
            }

            texture.Load(texPath);
        }
        
        Texture& GetTexture() { return texture; }
        
        CharDesc GetCharDesc(char character)
        {
            for(CharDesc desc : chars)
            {
                if(desc.char_id == static_cast<int>(character))
                    return desc;
            }
            
            return CharDesc();
        }
        
        Texture texture;
    private:
        std::vector<CharDesc> chars;
    };
    
    class Text
    {
    public:
        
        void Create ( std::string t, std::string font_path = "OpenSans12" )
        {
            font.Load(font_path);
            
            data = t;
            
            int cursorX = 0;
            
            for(int i = 0; i < data.size(); i++)
            {
                CharDesc cd = font.GetCharDesc(data[i]);
                
                vertexBuffer.AddVertexPosition(cd.xoffset + cursorX, cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX + cd.w, cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX, cd.h + cd.yoffset, 0.0f);
                
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX, cd.h + cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX + cd.w, cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset + cursorX + cd.w, cd.yoffset + cd.h, 0.0f);
                
                //float width = static_cast<float>(Text::GetFont()->GetTexture()->GetWidth());
                //float height = static_cast<float>(Text::GetFont()->GetTexture()->GetHeight());
                float width = static_cast<float>(font.GetTexture().GetWidth());
                float height = static_cast<float>(font.GetTexture().GetHeight());
                
                float charX = (float)cd.x;
                float charY = (float)cd.y;
                
                float charW = (float)cd.w;
                float charH = (float)cd.h;
                
                vertexBuffer.AddTextureCoord(charX / width, charY / height);
                vertexBuffer.AddTextureCoord((charX+charW)/width, (charY)/height);
                vertexBuffer.AddTextureCoord(charX / width, (charY+charH)/height);
                
                vertexBuffer.AddTextureCoord(charX / width, (charY+charH)/height);
                vertexBuffer.AddTextureCoord((charX+charW)/width, (charY)/height);
                vertexBuffer.AddTextureCoord((charX+charW)/width, (charY+charH)/height);
                
                
                cursorX += cd.xadvance;

            }
            
            vertexBuffer.Create();
        }
        
        void SetText(std::string new_text)
        {
            vertexBuffer.Clear();
            int cursorX = 0;
            
            this->data = "";
            this->data += new_text;
            
            for(int i = 0; i < data.size(); i++)
            {
                CharDesc cd = font.GetCharDesc(data[i]);
                
                vertexBuffer.AddVertexPosition(cd.xoffset + cursorX, cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX + cd.w, cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX, cd.h + cd.yoffset, 0.0f);
                
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX, cd.h + cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset +cursorX + cd.w, cd.yoffset, 0.0f);
                vertexBuffer.AddVertexPosition(cd.xoffset + cursorX + cd.w, cd.yoffset + cd.h, 0.0f);
                
                float width = static_cast<float>(font.GetTexture().GetWidth());
                float height = static_cast<float>(font.GetTexture().GetHeight());
                
                float charX = (float)cd.x;
                float charY = (float)cd.y;
                
                float charW = (float)cd.w;
                float charH = (float)cd.h;
                
                vertexBuffer.AddTextureCoord(charX / width, charY / height);
                vertexBuffer.AddTextureCoord((charX+charW)/width, (charY)/height);
                vertexBuffer.AddTextureCoord(charX / width, (charY+charH)/height);
                
                vertexBuffer.AddTextureCoord(charX / width, (charY+charH)/height);
                vertexBuffer.AddTextureCoord((charX+charW)/width, (charY)/height);
                vertexBuffer.AddTextureCoord((charX+charW)/width, (charY+charH)/height);
                
                cursorX += cd.xadvance;
            }
            
            vertexBuffer.Update();
        }
        
        void Draw()
        {
            transform = glm::translate( glm::mat4(1.0f), glm::vec3(position, 0.0f) );
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            
            glUseProgram( bfr_program );
            
            glUniformMatrix4fv(glGetUniformLocation(bfr_program, "projection"), 1, GL_FALSE, &bfr_projection[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(bfr_program, "model"), 1, GL_FALSE, &transform[0][0]);
            
            glUniform3f( glGetUniformLocation(bfr_program, "color"), color.r, color.g, color.b );
            glUniform1f( glGetUniformLocation(bfr_program, "opacity"), opacity );
            
            // bind font texture
            glActiveTexture(GL_TEXTURE0 + 0);//0 index
            glUniform1f(glGetUniformLocation(bfr_program, "tex0"), 0);
            glBindTexture(GL_TEXTURE_2D, font.GetTexture().GetGLTexture());
            
            vertexBuffer.Draw();
            
            glUseProgram( 0 );
            
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }
        
        void SetFont ( std::string path )
        {
            font.Load(path);
        }
        
        glm::vec2 GetPosition() { return position; }
        void SetPosition(glm::vec2 new_pos) { position = new_pos; }
        
        void SetColor(glm::vec3 new_color) { color = new_color; }
        void SetOpacity(float amount) { opacity = amount; }
        
    private:
        Font font;
        std::string data;
        VertexBuffer vertexBuffer;
        
        glm::mat4 transform;
        glm::vec2 position;
        
        glm::vec3 color;
        float     opacity;
    };
    
    Font  openSans;
    Text   _text;
    std::string defaultFontPath;
    
    inline void __Init(int window_width, int window_height)
    {
        glewExperimental = GL_TRUE;
        glewInit();
        
        bfr_vertex=0;
        bfr_fragment=0;
        
        const char* vs =
        "#version 330 core\n"
        "layout(location=0) in vec3 vPosition;\n"
        "layout(location=1) in vec2 vTexCoord;\n"
        "out vec2 fTexCoord;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 model;\n"
        "void main()\n"
        "{\n"
        "   fTexCoord = vTexCoord;\n"
        "   gl_Position = projection * model * vec4(vPosition,1.0);\n"
        "}\n";
        
        bfr_vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(bfr_vertex,1,(const char**)&vs,NULL);
        glCompileShader(bfr_vertex);
        
        
        const char* fs =
        "#version 330 core\n"
        "in vec2 fTexCoord;\n"
        "out vec4 out_color;\n"
        "uniform sampler2D tex0;\n"
        "uniform vec3 color = vec3(1.0,1.0,1.0);\n"
        "uniform float opacity = 1.0;\n"
        "void main()\n"
        "{"
        "     vec3 crgb = texture(tex0, vec2(fTexCoord.x,-fTexCoord.y)).rgb;\n"
        "     float alph = texture(tex0, vec2(fTexCoord.x,-fTexCoord.y)).a;\n"
        "     out_color = vec4(crgb*color, alph*opacity);\n"
        "}\n";
        
        bfr_fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(bfr_fragment, 1, (const char**)&fs, NULL);
        glCompileShader(bfr_fragment);
        
        bfr_program = glCreateProgram();
        
        glAttachShader(bfr_program, bfr_vertex);
        glAttachShader(bfr_program, bfr_fragment);
        
        glLinkProgram(bfr_program);
        
        
        _text.Create("AAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaá", "OpenSans12");
        
        bfr_projection =glm::ortho(0.0f,static_cast<float>(window_width),static_cast<float>(window_height),0.0f);
        defaultFontPath = "OpenSans12";
    }
    
    // For fast text drawing using default font
    void DrawText( std::string data, glm::vec2 pos = glm::vec2(0.0f), glm::vec3 color=glm::vec3(1.0f) )
    {
       
        _text.SetText(data);
        _text.SetPosition(pos);
        _text.SetColor(color);
        _text.SetOpacity(1.0f);
        
        
        _text.Draw();
         _text.SetFont(defaultFontPath);
    }
    
    // could be slow, recommended: DrawText or Text class.
    void DrawText ( std::string data, std::string font_path, glm::vec2 pos = glm::vec2(0.0f), glm::vec3 color=glm::vec3(1.0f) )
    {
        _text.SetFont(font_path);
        DrawText(data,pos,color);
    }
}

#endif
