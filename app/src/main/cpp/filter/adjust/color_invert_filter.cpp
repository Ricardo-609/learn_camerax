#include "color_invert_filter.h"


const GLchar *ColorInvertImageFilter::GetVertexShader() {
    return ImageFilter::GetVertexShader();
}

const GLchar *ColorInvertImageFilter::GetFragmentShader() {
    return ""
           "varying highp vec2 textureCoordinate;\n"
           "\n"
           "uniform sampler2D inputImageTexture;\n"
           "\n"
           "void main()\n"
           "{\n"
           "    lowp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);\n"
           "    \n"
           // textureColor.w其实就是textureColor.a 透明度，这里不参与反相，因为我们透明度本来是1(完全不透明)，参与反相的话就是0了，完全就是透明了
           "    gl_FragColor = vec4((1.0 - textureColor.rgb), textureColor.w);\n"
           "}";;
}
