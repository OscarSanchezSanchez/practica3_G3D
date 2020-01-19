#include "BOX.h"
#include "auxiliar.h"


#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>


//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);
std::vector<glm::mat4> models(2);

//desplazamientos por teclado
float displacement = 0.1f;
float displacementLight = 5.0f;
//Giro de c�mara por teclado
float yaw_angle = 0.01f;


//Movimiento de c�mara con el rat�n
const float orbitAngle = 0.1f;
float lastX = 0.0f;
float lastY = 0.0f;
float desplX = 0.0f;
float desplY = 0.0f;


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
//Texturas  
unsigned int colorTexId;  
unsigned int emiTexId;

struct program
{
	unsigned int vshader;
	unsigned int fshader;
	unsigned int program;

};

std::vector<program> programs(2);


//Variables Uniform
struct uniform
{
	int uModelViewMat;
	int uModelViewProjMat;
	int uNormalMat;
};

std::vector<uniform> uniforms(2);

//Variables uniformes posición e intesidad de la luz
int uLightPosition;
int uLightIntensity;

glm::vec3 lightPosition(0.0f);
glm::vec3 lightIntensity(0.1f);

//Texturas Uniform  
int uColorTex;  
int uEmiTex;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//VAO
unsigned int vao;
//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;


//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname, struct program *program, struct uniform *uniform);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShader("../shaders_P3/shader.v0.vert", "../shaders_P3/shader.v0.frag", &programs[0], &uniforms[0]);
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag", &programs[1], &uniforms[1]);

	initObj();

	glutMainLoop();

	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv){

	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas OGL");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);

}
void initOGL(){

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);


	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -10.0f;

}

void destroy()
{
	glDetachShader(programs[0].program, programs[0].vshader);
	glDetachShader(programs[0].program, programs[0].fshader);
	glDeleteShader(programs[0].vshader);
	glDeleteShader(programs[0].fshader);
	glDeleteProgram(programs[0].program);
	glDetachShader(programs[1].program, programs[1].vshader);
	glDetachShader(programs[1].program, programs[1].fshader);
	glDeleteShader(programs[1].vshader);
	glDeleteShader(programs[1].fshader);
	glDeleteProgram(programs[1].program);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);  
	glDeleteTextures(1, &emiTexId);

}

void initShader(const char *vname, const char *fname, struct program *program, struct uniform *uniform)
{
	program->vshader = loadShader(vname, GL_VERTEX_SHADER);
	program->fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program->program = glCreateProgram();
	glAttachShader(program->program, program->vshader);
	glAttachShader(program->program, program->fshader);
	glLinkProgram(program->program);

	//comprobacion de errores en el enlazado de shader al programa
	int linked;
	glGetProgramiv(program->program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program->program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program->program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program->program);
		program->program = 0;
		exit(-1);
	}

	uniform->uNormalMat = glGetUniformLocation(program->program, "normal");
	uniform->uModelViewMat = glGetUniformLocation(program->program, "modelView");
	uniform->uModelViewProjMat = glGetUniformLocation(program->program, "modelViewProj");

	uColorTex = glGetUniformLocation(program->program, "colorTex");
	uEmiTex = glGetUniformLocation(program->program, "emiTex");

	uLightPosition = glGetUniformLocation(program->program, "lightPosition");
	uLightIntensity = glGetUniformLocation(program->program, "lightIntensity");

	inPos = glGetAttribLocation(program->program, "inPos");
	inColor = glGetAttribLocation(program->program, "inColor");
	inNormal = glGetAttribLocation(program->program, "inNormal");
	inTexCoord = glGetAttribLocation(program->program, "inTexCoord");

	glUseProgram(program->program);
	if (uColorTex != -1) {
		glUniform1i(uColorTex, 0);
	}

	if (uEmiTex != -1) {
		glUniform1i(uEmiTex, 1);
	}

}
void initObj(){

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexPos, GL_STATIC_DRAW);

	glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexColor, GL_STATIC_DRAW);


	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}
	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}
	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}
	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex,GL_STATIC_DRAW);

	colorTexId = loadTex("../img/color2.png");  
	emiTexId = loadTex("../img/emissive.png");
	models[0] = glm::mat4(1.0f);
	models[1] = glm::mat4(1.0f);

}

GLuint loadShader(const char *fileName, GLenum type){ 

	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);
	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar * *)& source, (const GLint*)& fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << fileName << std::endl;
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader; 
}

unsigned int loadTex(const char *fileName){ 
	unsigned char* map;  
	unsigned int w, h;  
	map = loadTexture(fileName, w, h);

	if (!map) { 
		std::cout << "Error cargando el fichero: " 
			<< fileName << std::endl;  
		exit(-1); 
	}

	unsigned int texId;  
	glGenTextures(1, &texId);  
	glBindTexture(GL_TEXTURE_2D, texId); 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, 
		GL_UNSIGNED_BYTE, (GLvoid*)map);

	delete[] map;

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		GL_LINEAR_MIPMAP_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;

}

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (size_t i = 0; i < programs.size(); i++)
	{
		glUseProgram(programs[i].program);

		glm::mat4 modelView = view * models[i];
		glm::mat4 modelViewProj = proj * modelView;
		glm::mat4 normal = glm::transpose(glm::inverse(modelView));

		if (uniforms[i].uModelViewMat != -1)
			glUniformMatrix4fv(uniforms[i].uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));

		if (uniforms[i].uModelViewProjMat != -1)
			glUniformMatrix4fv(uniforms[i].uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));

		if (uniforms[i].uNormalMat != -1)
			glUniformMatrix4fv(uniforms[i].uNormalMat, 1, GL_FALSE,	&(normal[0][0]));

		if (uLightIntensity != -1)
			glUniform3fv(uLightIntensity, 1, &lightIntensity[0]);

		if (uLightPosition != -1)
			glUniform3fv(uLightPosition, 1, &lightPosition[0]);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	}
	//Texturas  
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, emiTexId);
	

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);

	glutSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void resizeFunc(int width, int height)
{
	float aspectRatio = (float)width / (float)height;

	proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 50.0f);

	glViewport(0, 0, width, height);

	glutPostRedisplay();

}

void idleFunc()
{
	models[0] = glm::mat4(1.0f);
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
	models[0] = glm::rotate(models[0], angle, glm::vec3(1.0f, 1.0f, 0.0f));
	glutPostRedisplay();

	models[1] = glm::mat4(1.0f);
	//rotacion sobre eje y
	models[1] = glm::rotate(models[1], angle, glm::vec3(0, 4, 0));

	//translacion sobre x para desplazar el cuadrado
	models[1] = glm::translate(models[1], glm::vec3(3, 0, 0));

	//rotacion sobre Y para simular la orbitacion del objeto
	models[1] = glm::rotate(models[1], angle, glm::vec3(0, 1, 0));

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
    std::cout << "Se ha pulsado la tecla " << key << std::endl << std::endl;

    glm::mat4 rotation(1.0f);

    switch (key)
    {
    case 'w':
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, displacement));
        break;
    case 's':
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -displacement));
        break;
    case 'a':
        view = glm::translate(view, glm::vec3(displacement, 0.0f, 0.0f));
        break;
    case 'd':
        view = glm::translate(view, glm::vec3(-displacement, 0.0f, 0.0f));
        break;
    case 'q':
        rotation = glm::rotate(rotation, -yaw_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        view = rotation * view;
        break;
    case 'e':
        rotation = glm::rotate(rotation, yaw_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        view = rotation * view;
        break;
    case 'l':
        lightIntensity = glm::min(lightIntensity + glm::vec3(0.1f), glm::vec3(1.0f));
        break;
    case 'm':
        lightIntensity = glm::min(lightIntensity - glm::vec3(0.1f), glm::vec3(1.0f));
        break;
    case 'u':
        lightPosition.y += displacementLight;
        break;
    case 'j':
        lightPosition.y -= displacementLight;
        break;
	case 'h':
		lightPosition.x -= displacementLight;
		break;
	case 'k':
		lightPosition.x += displacementLight;
		break;
    default:
        break;
    }

    glutPostRedisplay();

}

void mouseFunc(int button, int state, int x, int y)
{
    if (state == 0)
        std::cout << "Se ha pulsado el boton ";
    else
        std::cout << "Se ha soltado el boton ";

    if (button == 0) std::cout << "de la izquierda del raton " << std::endl;
    if (button == 1) std::cout << "central del raton " << std::endl;
    if (button == 2) std::cout << "de la derecha del raton " << std::endl;

    std::cout << "en la posicion " << x << " " << y << std::endl << std::endl;
}

void mouseMotionFunc(int x, int y)
{

    float xOffset = (float)x - lastX;
    float yOffset = (float)y - lastY;

    lastX = (float)x;
    lastY = (float)y;

    desplX += xOffset;
    desplY += yOffset;

    glm::mat4 rotation(1.0f);

    view = glm::rotate(view, orbitAngle, glm::vec3(desplY, desplX, 0.0));

    glutPostRedisplay();
}

