#ifndef SHADER_HPP
#define SHADER_HPP

GLboolean compileCheck(GLuint shader);
GLuint initShaderObj(const GLchar* srcfile, GLenum shaderType);
GLuint createShaderPgm();
void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle);
GLint checkShaderLinkStatus(GLuint pgmHandle);

#endif
