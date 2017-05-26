#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <GL/glew.h>
#include <include/GL/gl.h>
#include <include/GL/glext.h>
#include <include/GL/glut.h>
#include <include/GL/glm/glm.hpp>
#include <include/GL/glm/gtc/matrix_transform.hpp>
#include <include/GL/glm/gtx/transform2.hpp>
#include <include/GL/glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include <include/GL/glui.h>

#include <iomanip> // setprecision
#include <sstream> // stringstream


#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)
using namespace std;
using glm::mat4;
using glm::vec3;

GLuint g_vao;
GLuint g_programHandle;
GLuint g_winWidth = 900;
GLuint g_winHeight = 800;
GLint g_angle = 0;
GLuint g_frameBuffer;
// transfer function
GLuint g_bfTexObj;
GLuint g_texWidth;
GLuint g_texHeight;
GLuint g_rcVertHandle;
GLuint g_rcFragHandle;
GLuint g_bfVertHandle;
GLuint g_bfFragHandle;
GLuint *pngTex;
GLuint trTex;

float g_stepSize = 150.0;
float g_NumberOfSlices = 512.0; // 504
float g_MinGrayVal = 0.4196; // 0
float g_MaxGrayVal = 1.0; // 1
float g_OpacityVal = 40.0; // 40
float g_ColorVal = 1.0; // 0.4
float g_AbsorptionModeIndex = 1.0; // -1.0 ? 1
float g_SlicesOverX = 8.0; // 16
float g_SlicesOverY = 8.0; // 16

int maxTexturesNumber = 8;

int tr_width = 256;
int tr_height = 10;

int png_width = 4096;
int png_height = 4096;

int   last_x, last_y;
float rotationX = 0.0, rotationY = 0.0;
float initialFoV = 45.0f;
float FoV=45.0f;

int main_window;

/** Pointers to the windows and some of the controls we'll create **/
GLUI *glui, *glui2;
GLUI_Spinner    *light0_spinner, *light1_spinner;
GLUI_RadioGroup *radio;
GLUI_Panel      *obj_panel;
GLUI_StaticText *fps_val;

int checkForOpenGLError(const char* file, int line)
{
    // return 1 if an OpenGL error occured, 0 otherwise.
    GLenum glErr;
    int retCode = 0;

    glErr = glGetError();
    while(glErr != GL_NO_ERROR)
    {
    	cout << "glError in file " << file
    	     << "@line " << line << gluErrorString(glErr) << endl;
    	retCode = 1;
    	exit(EXIT_FAILURE);
    }
    return retCode;
}
void keyboard(unsigned char key, int x, int y);
void display(void);
void initVBO();
void initShader();
void initFrameBuffer(GLuint, GLuint, GLuint);
GLuint initFace2DTex(GLuint texWidth, GLuint texHeight);

double GetTickCount(void)
{
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now))
    return 0;
  return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

void CalculateFrameRate()
{
  static float framesPerSecond    = 0.0f;       // This will store our fps
  static float lastTime   = 0.0f;       // This will hold the time from the last frame
  float currentTime = GetTickCount() * 0.001f;
  //printf("%.1f FPS\n", currentTime);
  ++framesPerSecond;
  if( currentTime - lastTime >= 1.0f )
  {
    stringstream stream;
    stream << fixed << setprecision(2) << framesPerSecond
      << "\tSpeed: " << 1000.0/double(framesPerSecond);
    string str = "FPS: " + stream.str();

    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    fps_val->set_text(cstr);
    delete [] cstr;

    // lastTime++;
    lastTime = currentTime;
    // if(SHOW_FPS == 1) fprintf(stderr, "\nCurrent Frames Per Second: %d\n\n", (int)framesPerSecond);
    framesPerSecond = 0;
  }
}

void loadImage(string pathToDir, GLuint* texture, int* width, int* height)
{
  // texture = new GLuint[maxTexturesNumber];
  glEnable(GL_TEXTURE_2D);
  glGenTextures(maxTexturesNumber, texture);

  for (int i = 0; i < maxTexturesNumber; i++) {
    // string pathToFile = pathToDir + "bonsai.raw.png";
    string pathToFile = pathToDir + "slicemap_" + to_string(i) + ".png";
    cout << pathToFile << endl;
    glBindTexture(GL_TEXTURE_2D, texture[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char* tempTexture = SOIL_load_image(pathToFile.c_str(), width, height, 0, SOIL_LOAD_RGBA);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempTexture);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(tempTexture);
  }
}

void loadImage2(const char* pathToFile, GLuint* texture, int* width, int* height)
{
  unsigned char* tempTexture = SOIL_load_image(pathToFile, width, height, 0, SOIL_LOAD_RGBA);

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempTexture);
  glGenerateMipmap(GL_TEXTURE_2D);

  SOIL_free_image_data(tempTexture);
}

void render(GLenum cullFace);

void init()
{
  g_texWidth = g_winWidth;
  g_texHeight = g_winHeight;
  initVBO();
  initShader();

  pngTex = new GLuint[maxTexturesNumber];
  loadImage("../sprites/", pngTex, &png_width, &png_height);
  // loadImage("../", pngTex, &png_width, &png_height);
  loadImage2("../cm_BrBG_r.png", &trTex, &tr_width, &tr_height);

  g_bfTexObj = initFace2DTex(g_texWidth, g_texHeight);
  GL_ERROR();

  initFrameBuffer(g_bfTexObj, g_texWidth, g_texHeight);
  GL_ERROR();
  glutPostRedisplay();
}
// init the vertex buffer object
void initVBO()
{
    GLfloat vertices[24] = {
    	0.0, 0.0, 0.0,
    	0.0, 0.0, 1.0,
    	0.0, 1.0, 0.0,
    	0.0, 1.0, 1.0,
    	1.0, 0.0, 0.0,
    	1.0, 0.0, 1.0,
    	1.0, 1.0, 0.0,
    	1.0, 1.0, 1.0
    };

    GLuint indices[36] = {
    	1,5,7,
    	7,3,1,
    	0,2,6,
      6,4,0,
    	0,1,3,
    	3,2,0,
    	7,5,4,
    	4,6,7,
    	2,3,7,
    	7,6,2,
    	1,0,4,
    	4,5,1
    };
    GLuint gbo[2];

    glGenBuffers(2, gbo);
    GLuint vertexdat = gbo[0];
    GLuint veridxdat = gbo[1];
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // used in glDrawElement()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    // vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); // for vertexloc
    glEnableVertexAttribArray(1); // for vertexcol

    // the vertex location is the same as the vertex color
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    // glBindVertexArray(0);
    g_vao = vao;
}

void drawBox(GLenum glFaces)
{
    glEnable(GL_CULL_FACE);
    glCullFace(glFaces);
    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glDisable(GL_CULL_FACE);
}

// check the compilation result
GLboolean compileCheck(GLuint shader)
{
    GLint err;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &err);
    if (GL_FALSE == err)
    {
    	GLint logLen;
    	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    	if (logLen > 0)
    	{
  	    char* log = (char *)malloc(logLen);
  	    GLsizei written;
  	    glGetShaderInfoLog(shader, logLen, &written, log);
  	    cerr << "Shader log: " << log << endl;
  	    free(log);
    	}
    }
    return err;
}

// init shader object
GLuint initShaderObj(const GLchar* srcfile, GLenum shaderType)
{
    ifstream inFile(srcfile, ifstream::in);
    // use assert?
    if (!inFile)
    {
    	cerr << "Error openning file: " << srcfile << endl;
    	exit(EXIT_FAILURE);
    }

    const int MAX_CNT = 10000;
    GLchar *shaderCode = (GLchar *) calloc(MAX_CNT, sizeof(GLchar));
    inFile.read(shaderCode, MAX_CNT);
    if (inFile.eof())
    {
    	size_t bytecnt = inFile.gcount();
    	*(shaderCode + bytecnt) = '\0';
    }
    else if(inFile.fail()) cout << srcfile << "read failed \n";
    else cout << srcfile << "is too large\n";
    // create the shader Object
    GLuint shader = glCreateShader(shaderType);
    if (0 == shader) cerr << "Error creating vertex shader.\n";
    const GLchar* codeArray[] = {shaderCode};
    glShaderSource(shader, 1, codeArray, NULL);
    free(shaderCode);

    // compile the shader
    glCompileShader(shader);
    if (GL_FALSE == compileCheck(shader)) cerr << "shader compilation failed\n";
    return shader;
}

GLint checkShaderLinkStatus(GLuint pgmHandle)
{
    GLint status;
    glGetProgramiv(pgmHandle, GL_LINK_STATUS, &status);
    if (GL_FALSE == status)
    {
    	GLint logLen;
    	glGetProgramiv(pgmHandle, GL_INFO_LOG_LENGTH, &logLen);
    	if (logLen > 0)
    	{
    	    GLchar * log = (GLchar *)malloc(logLen);
    	    GLsizei written;
    	    glGetProgramInfoLog(pgmHandle, logLen, &written, log);
    	    cerr << "Program log: " << log << endl;
    	}
    }
    return status;
}

// link shader program
GLuint createShaderPgm()
{
    // Create the shader program
    GLuint programHandle = glCreateProgram();
    if (0 == programHandle)
    {
    	cerr << "Error create shader program" << endl;
    	exit(EXIT_FAILURE);
    }
    return programHandle;
}

GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
    GLuint backFace2DTex;
    glGenTextures(1, &backFace2DTex);
    glBindTexture(GL_TEXTURE_2D, backFace2DTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bfTexWidth, bfTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    return backFace2DTex;
}

void checkFramebufferStatus()
{
    GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (complete != GL_FRAMEBUFFER_COMPLETE)
    {
    	cout << "framebuffer is not complete" << endl;
    	exit(EXIT_FAILURE);
    }
}

// init the framebuffer, the only framebuffer used in this program
void initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight)
{
    // create a depth buffer for our framebuffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);

    // attach the texture and the depth buffer to the framebuffer
    glGenFramebuffers(1, &g_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    checkFramebufferStatus();
    glEnable(GL_DEPTH_TEST);
}

void rcSetUinforms()
{
  // GLint maxTexturesNumberLoc = glGetUniformLocation(g_programHandle, "maxTexturesNumber");
  // if (maxTexturesNumberLoc >= 0) glUniform1i(maxTexturesNumberLoc, maxTexturesNumber);
  // else cout << "maxTexturesNumber is not bind to the uniform\n";

  GLint stepSizeLoc = glGetUniformLocation(g_programHandle, "uSteps");
  GL_ERROR();
  if (stepSizeLoc >= 0) glUniform1f(stepSizeLoc, g_stepSize);
  else cout << "uSteps is not bind to the uniform\n";
  GL_ERROR();
  GLint transferFuncLoc = glGetUniformLocation(g_programHandle, "uTransferFunction");
  if (transferFuncLoc >= 0)
  {
    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_2D, trTex);
    	glUniform1i(transferFuncLoc, 0);
  }
  else cout << "uBackCoord is not bind to the uniform\n";
  GL_ERROR();
  GLint backFaceLoc = glGetUniformLocation(g_programHandle, "uBackCoord");
  if (backFaceLoc >= 0)
  {
    	glActiveTexture(GL_TEXTURE1);
    	glBindTexture(GL_TEXTURE_2D, g_bfTexObj);
    	glUniform1i(backFaceLoc, 1);
  }
  else cout << "uBackCoord is not bind to the uniform\n";
  GL_ERROR();

  string sprites;
  GLint volumeLoc;
  for (int i = 0; i < maxTexturesNumber; i++)
  {
    sprites = "uSliceMaps[" + to_string(i) + "]";
    volumeLoc = glGetUniformLocation(g_programHandle, sprites.c_str());
    if (volumeLoc >= 0)
    {
    	glActiveTexture(33986 + i);
    	glBindTexture(GL_TEXTURE_2D, *pngTex + i);
    	glUniform1i(volumeLoc, 2 + i);
    }
    else cout << "uSliceMaps is not bind to the uniform\n";
  }

  GLint uNumberOfSlicesLoc = glGetUniformLocation(g_programHandle, "uNumberOfSlices");
  if (uNumberOfSlicesLoc >= 0) glUniform1f(uNumberOfSlicesLoc, g_NumberOfSlices);
  else cout << "uNumberOfSlices is not bind to the uniform\n";

  GLint uMinGrayValLoc = glGetUniformLocation(g_programHandle, "uMinGrayVal");
  if (uMinGrayValLoc >= 0) glUniform1f(uMinGrayValLoc, g_MinGrayVal);
  else cout << "uMinGrayVal is not bind to the uniform\n";

  GLint uMaxGrayValLoc = glGetUniformLocation(g_programHandle, "uMaxGrayVal");
  if (uMaxGrayValLoc >= 0) glUniform1f(uMaxGrayValLoc, g_MaxGrayVal);
  else cout << "uMaxGrayVal is not bind to the uniform\n";

  GLint uOpacityValLoc = glGetUniformLocation(g_programHandle, "uOpacityVal");
  if (uOpacityValLoc >= 0) glUniform1f(uOpacityValLoc, g_OpacityVal);
  else cout << "uOpacityVal is not bind to the uniform\n";

  GLint uColorValLoc = glGetUniformLocation(g_programHandle, "uColorVal");
  if (uColorValLoc >= 0) glUniform1f(uColorValLoc, g_ColorVal);
  else cout << "uColorVal is not bind to the uniform\n";

  GLint uAbsorptionModeIndexLoc = glGetUniformLocation(g_programHandle, "uAbsorptionModeIndex");
  if (uAbsorptionModeIndexLoc >= 0) glUniform1f(uAbsorptionModeIndexLoc, g_AbsorptionModeIndex);
  else cout << "uAbsorptionModeIndex is not bind to the uniform\n";

  GLint uSlicesOverXLoc = glGetUniformLocation(g_programHandle, "uSlicesOverX");
  if (uSlicesOverXLoc >= 0) glUniform1f(uSlicesOverXLoc, g_SlicesOverX);
  else cout << "uSlicesOverX is not bind to the uniform\n";

  GLint uSlicesOverYLoc = glGetUniformLocation(g_programHandle, "uSlicesOverY");
  if (uSlicesOverYLoc >= 0) glUniform1f(uSlicesOverYLoc, g_SlicesOverY);
  else cout << "uSlicesOverY is not bind to the uniform\n";
}

// init the shader object and shader program
void initShader()
{
// vertex shader object for first pass
    g_bfVertHandle = initShaderObj("../shader/firstPass.vert", GL_VERTEX_SHADER);
// fragment shader object for first pass
    g_bfFragHandle = initShaderObj("../shader/firstPass.frag", GL_FRAGMENT_SHADER);
// vertex shader object for second pass
    g_rcVertHandle = initShaderObj("../shader/secondPass.vert", GL_VERTEX_SHADER);
// fragment shader object for second pass
    g_rcFragHandle = initShaderObj("../shader/secondPass.frag", GL_FRAGMENT_SHADER);
// create the shader program , use it in an appropriate time
    g_programHandle = createShaderPgm();
}

// link the shader objects using the shader program
void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle)
{
    const GLsizei maxCount = 2;
    GLsizei count;
    GLuint shaders[maxCount];
    glGetAttachedShaders(shaderPgm, maxCount, &count, shaders);

    GL_ERROR();
    for (int i = 0; i < count; i++) {
	     glDetachShader(shaderPgm, shaders[i]);
    }
    // Bind index 0 to the shader input variable "VerPos"
    glBindAttribLocation(shaderPgm, 0, "position");
    // Bind index 1 to the shader input variable "VerClr"
    glBindAttribLocation(shaderPgm, 1, "vertColor");
    GL_ERROR();
    glAttachShader(shaderPgm,newVertHandle);
    glAttachShader(shaderPgm,newFragHandle);
    GL_ERROR();
    glLinkProgram(shaderPgm);
    if (GL_FALSE == checkShaderLinkStatus(shaderPgm))
    {
    	cerr << "Failed to relink shader program!" << endl;
    	exit(EXIT_FAILURE);
    }
    GL_ERROR();
}

void display()
{
    int tx, ty, tw, th;
    GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );

    glEnable(GL_DEPTH_TEST);
    GL_ERROR();
    // render to texture
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_frameBuffer);
    glViewport(0, 0, g_winWidth, g_winHeight);
    linkShader(g_programHandle, g_bfVertHandle, g_bfFragHandle);
    glUseProgram(g_programHandle);
    // cull front face
    render(GL_FRONT);
    glUseProgram(0);
    GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    tx = (g_winWidth - tw) / 2;
    glViewport(-tx, 0, g_winWidth, g_winHeight);

    linkShader(g_programHandle, g_rcVertHandle, g_rcFragHandle);
    GL_ERROR();
    glUseProgram(g_programHandle);
    rcSetUinforms();
    GL_ERROR();
    render(GL_BACK);
    glUseProgram(0);
    GL_ERROR();

    glutSwapBuffers();
}

float angleX=0;
float angleY=0;

void render(GLenum cullFace)
{
    GL_ERROR();
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //  transform the box
    glm::mat4 projection = glm::perspective(FoV, (GLfloat)g_winWidth/g_winHeight, 0.1f, 400.f);
    // glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.0f, 2.0f),
    // 				 glm::vec3(0.0f, 0.0f, 0.0f),
    // 				 glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
    				 glm::vec3(0.0f, 0.0f, 0.0f),
    				 glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 model = mat4(1.0f);
    //model *= glm::rotate((float)g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // to make the "head256.raw" i.e. the volume data stand up.
    //model *= glm::rotate(180.0f, vec3(1.0f, 0.0f, 0.0f));
    model *= glm::rotate(angleX, vec3(0.0f, 1.0f, 0.0f));
    model *= glm::rotate((180.0f+angleY), vec3(1.0f, 0.0f, 0.0f));

    model *= glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f));
    // notice the multiplication order: reverse order of transform
    glm::mat4 mvp = projection * view * model;
    GLuint mvpIdx = glGetUniformLocation(g_programHandle, "MVP");
    if (mvpIdx >= 0)
    {
    	glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);
    }
    else
    {
    	cerr << "can't get the MVP" << endl;
    }
    GL_ERROR();
    drawBox(cullFace);
    GL_ERROR();
}

void myGlutMouse(int button, int button_state, int x, int y )
{
  if ( button == GLUT_LEFT_BUTTON && button_state == GLUT_DOWN )
  {
    last_x = x;
    last_y = y;
  }

  if (button == 3)
  {
    if (FoV > 1) FoV-=1;
  }

  if (button == 4)
  {
    if (FoV < 179) FoV+=1;
  }
  glutPostRedisplay();
}

void myGlutMotion(int x, int y )
{
  rotationX += (float) (y - last_y);
  rotationY += (float) (x - last_x);

  angleY= 180 * rotationX/g_winWidth;
  angleX= 180 * rotationY/g_winHeight;

  last_x = x;
  last_y = y;

  glutPostRedisplay();
}


void rotateDisplay()
{
    g_angle = (g_angle + 1) % 360;
    glutPostRedisplay();
}
void reshape(int w, int h)
{
    g_winWidth = w;
    g_winHeight = h;
    g_texWidth = w;
    g_texHeight = h;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '\x1B':
	exit(EXIT_SUCCESS);
	break;
    }
}

void timerCB(int millisec)
{
  CalculateFrameRate();

glutTimerFunc(millisec, timerCB, millisec);
glutPostRedisplay();
}

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowPosition( 50, 50 );
  glutInitWindowSize(900, 800);

  main_window = glutCreateWindow("OpenGL VRC");
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
  	/* Problem: glewInit failed, something is seriously wrong. */
  	fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }

  glutDisplayFunc(&display);
  glutTimerFunc(10, timerCB, 10);
  glutReshapeFunc(&reshape);
  glutKeyboardFunc(&keyboard);
  glutMotionFunc(&myGlutMotion);
  glutMouseFunc(&myGlutMouse);
  //glutIdleFunc(&rotateDisplay);
  // init();

  /****************************************/
  /*         Here's the GLUI code         */
  /****************************************/

  printf( "GLUI version: %3.2f\n", GLUI_Master.get_version() );

  /*** Create the side subwindow ***/
  glui = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);

  // obj_panel = new GLUI_Rollout(glui, "Properties", false);
  obj_panel = new GLUI_Panel(glui, "");

  /***** Control for object params *****/

  GLUI_Scrollbar *sb;
  GLUI_Separator *separator;

  fps_val = new GLUI_StaticText(obj_panel, "120 FPS");

  separator = new GLUI_Separator(obj_panel);

  GLUI_StaticText *op_label = new GLUI_StaticText(obj_panel, "Opacity:");
  sb = new GLUI_Scrollbar(obj_panel, "Opacity", GLUI_SCROLL_HORIZONTAL, &g_OpacityVal);
  sb->set_float_limits(0, 40);

  separator = new GLUI_Separator(obj_panel);

  GLUI_StaticText *min_gr_label = new GLUI_StaticText(obj_panel, "MinGrayVal:");
  sb = new GLUI_Scrollbar(obj_panel, "MinGrayVal", GLUI_SCROLL_HORIZONTAL, &g_MinGrayVal);
  sb->set_float_limits(0, 1);

  separator = new GLUI_Separator(obj_panel);

  GLUI_StaticText *max_gr_label = new GLUI_StaticText(obj_panel, "MaxGrayVal:");
  sb = new GLUI_Scrollbar(obj_panel, "MaxGrayVal", GLUI_SCROLL_HORIZONTAL, &g_MaxGrayVal);
  sb->set_float_limits(0, 1);

  separator = new GLUI_Separator(obj_panel);

  GLUI_StaticText *color_val_label = new GLUI_StaticText(obj_panel, "ColorVal:");
  sb = new GLUI_Scrollbar(obj_panel, "ColorVal", GLUI_SCROLL_HORIZONTAL, &g_ColorVal);
  sb->set_float_limits(0, 1.0);

  separator = new GLUI_Separator(obj_panel);

  // GLUI_StaticText *step_size_label = new GLUI_StaticText(obj_panel, "StepSize:");
  GLUI_Spinner *spinner = new GLUI_Spinner(obj_panel, "StepSize:", &g_stepSize);
  spinner->set_float_limits(0, 256.0);
  spinner->set_alignment(GLUI_ALIGN_RIGHT);
  sb = new GLUI_Scrollbar(obj_panel, "StepSize", GLUI_SCROLL_HORIZONTAL, &g_stepSize);
  sb->set_float_limits(0, 1024.0);

  init();

  glui->set_main_gfx_window(main_window);

  glutMainLoop();
  return EXIT_SUCCESS;
}
