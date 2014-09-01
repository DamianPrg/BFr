BFr
===

Simple Bitmap Font Renderer for OpenGL 3 and 4

#### Features
 - Simple to use
 - Coloring text
 - Text shadow
 - Width and height of text  
 - 1 header file solution  

#### Required libs
- GLM
- FreeImage

#### How to use

Firstly we need to call 
```
BFr::Init( HERE_WIDTH_OF_APP_WINDOW, HERE_HEIGHT_OF_APP_WINDOW );
```

This function will load shaders, load default fonts, etc..

Next if we want to draw text we need to call 
```
BFr::DrawText( "Your text here :)" );
```
there are optional arguments, position and last argument is color.

```
BFr::DrawText( "Hello world", glm::vec2( 20.0f, 20.0f ), glm::vec3(2.0f, 1.0f, 1.0f) ); // red text on 20.0f, 20.0f
