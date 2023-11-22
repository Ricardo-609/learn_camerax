#include "triangle_drawer.h"
#include "../utils/logger.h"

TriangleDrawer::TriangleDrawer() {

}

TriangleDrawer::~TriangleDrawer() {

}

void TriangleDrawer::OnInit() {
    //创建程序
    m_program_id = OpenGLUtils::CreateProgram(GetVertexShader(), GetFragmentShader());
    // 从opengl程序中获取执行name的位置。注意：这个name必须与vertext shader中定义的name相同
    m_vertex_pos_handler = glGetAttribLocation(m_program_id, "aPosition");
}


void TriangleDrawer::Release() {
    glDisableVertexAttribArray(m_vertex_pos_handler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteProgram(m_program_id);
}

void TriangleDrawer::DoDraw(int textureId, void *vertexPos, void *texturePos) {
    //启用顶点的句柄（启用顶点属性，默认是禁用的）
    glEnableVertexAttribArray(m_vertex_pos_handler);
    // 解析顶点数据，应用参数信息到逐个顶点属性
    // 参数m_vertex_pos_handler: 指定配置的的顶点属性；参数2：顶点属性大小，vec3,由三个值组成
    // 参数3：指定数据类型；参数4：步长，连续顶点属性组之间的间隔；参数5：位置数据在缓冲中起始位置的偏移量
    glVertexAttribPointer(m_vertex_pos_handler, 3, GL_FLOAT, GL_FALSE, 0, m_vertex_coors);
    //开始绘制
    // 参数：渲染的图元模式；起始位置；顶点数量
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);      // 画三角形，从第0个顶点开始一个画3次

}
// 顶点着色器（图形渲染管线的第一个阶段），
// vec4(r,g,b,a)
const char *TriangleDrawer::GetVertexShader() {
    return "attribute vec4 aPosition;  \n"                      // 此处aPosition对应上面的aPosition
           "void main()                              \n"
           "{                                        \n"
           "   gl_Position = aPosition;              \n"        // gl_Position 位置向量
           "}                                        \n";;
}
// 片段着色器
const char *TriangleDrawer::GetFragmentShader() {
    return "precision mediump float;                     \n"
           "void main()                                  \n"
           "{                                            \n"
           "   gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );  \n" // gl_FragColor 颜色向量
           "}                                            \n";
}