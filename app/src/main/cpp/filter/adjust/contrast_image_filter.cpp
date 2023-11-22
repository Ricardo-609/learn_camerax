#include "contrast_image_filter.h"


void ContrastImageFilter::OnInit() {
    ImageFilter::OnInit();
    m_contrastLocation = glGetUniformLocation(m_program_id, "m_contrast");
    setValue(m_contrast);
}

const GLchar *ContrastImageFilter::GetFragmentShader() {
    return ""
           "varying highp vec2 textureCoordinate;\n"
           " \n"
           " uniform sampler2D inputImageTexture;\n"
           " uniform lowp float m_contrast;\n"
           " \n"
           " void main()\n"
           " {\n"
           "     lowp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);\n"
           "     \n"
           // 画面黑白的比值。简单的线性对比度算法：结果=中值差*对比度+中值
           "     gl_FragColor = vec4(((textureColor.rgb - vec3(0.5)) * m_contrast + vec3(0.5)), textureColor.w);\n"
           " }";

}

void ContrastImageFilter::setValue(float contrast) {
    m_contrast = contrast;
    setFloat(m_contrastLocation, contrast);
}
