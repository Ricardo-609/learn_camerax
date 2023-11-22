#include "brightness_filter.h"

void BrightnessFilter::OnInit() {
    ImageFilter::OnInit();
    m_brightnessLocation = glGetUniformLocation(m_program_id, "brightness");
    setValue(m_brightness);
}

const GLchar *BrightnessFilter::GetFragmentShader() {
    return ""
           "varying highp vec2 textureCoordinate;\n"
           " \n"
           " uniform sampler2D inputImageTexture;\n"
           " uniform lowp float brightness;\n"          // 通过界面seekbar拖动来动态修改
           " \n"
           " void main()\n"
           " {\n"
           "     lowp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);\n"
           "     \n"
           "     gl_FragColor = vec4((textureColor.rgb + vec3(brightness)), textureColor.w);\n"     // 全方位线性增加色值
           " }";

}

void BrightnessFilter::setValue(float value) {
    m_brightness = value;
    setFloat(m_brightnessLocation, m_brightness);
}
