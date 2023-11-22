#ifndef BLOGDEMO_COLOR_INVERT_FILTER_H
#define BLOGDEMO_COLOR_INVERT_FILTER_H


#include "../base/image_filter.h"

class ColorInvertImageFilter : public ImageFilter {
public:

    virtual const GLchar *GetVertexShader();

    virtual const GLchar *GetFragmentShader();
};


#endif //BLOGDEMO_COLOR_INVERT_FILTER_H
