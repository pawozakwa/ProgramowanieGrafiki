/************************* 
*	Autor: Pawe≥ Woünica
*
*
**************************/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>



// Include GLEW
#include <GL/glew.h>


// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

#include "Obiekt.h"


bool loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
){
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];
		
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
	
	}

	return true;
}

GLuint loadBMP_custom(const char * imagepath){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file)							    {printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0;}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

mat4 Przesun(float x, float y, float z){

	mat4 Przesuniecie = glm::translate(
		glm::mat4(1.0f),
		glm::vec3(x, y, z));

	return Przesuniecie;

}

mat4 Obroc(float angle, int os){
	//os ->     0 dla x, 1 dla y, 2 dla z
	mat4 Obrot = mat4(1.0f);

	switch (os)
	{
	case 0:
		Obrot = rotate(
			mat4(1.0f),
			angle,
			vec3(1.f, 0.f, 0.f));
		return Obrot;
	case 1:
		Obrot = rotate(
			mat4(1.0f),
			angle,
			vec3(0.f, 1.f, 0.f));
		return Obrot;
	case 2:
		Obrot = rotate(
			mat4(1.0f),
			angle,
			vec3(0.f, 0.f, 1.f));
		return Obrot;
	default:
		cout << "Nieprawidlowy parametr osi obrotu" << endl;
		return Obrot;

	}


}

void randColors(GLfloat *tablicaKolorow){
	int length= sizeof(tablicaKolorow) / sizeof(GLuint);

	for (int i = 0; i < length; i++)
	{

		tablicaKolorow[i] += (rand() % 1000 - 500) /1000;
		if(tablicaKolorow[i] > 1.f)
			tablicaKolorow[i] = 1.f;
		if(tablicaKolorow[i] < 0.f)
			tablicaKolorow[i] = 0.f;
	}
}

void Draw(GLuint tablicaBufora[], int indexTablicyBufora, int iloscTrojkatow){

	//Funkcja rysuje dana ilosc trojkatow(3arg) na danej tablicy punktÛw(1arg)
	//		pod podanym indeksem(2arg)

	glBindBuffer(GL_ARRAY_BUFFER, tablicaBufora[indexTablicyBufora]);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);


	// Rysuj Prymitywy !
	glDrawArrays(GL_TRIANGLES, 0, iloscTrojkatow*3); // 12 trojkatow

}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	vec3 pozycja = vec3(-8, 0, 0);

	int szerokoscEkranu = 1024;
	int wysokoscEkranu = 768;

	float katPoziomy = 3.14f/4.f;
	float katPionowy = 0.0f;

	float katWidzenia = 45.0f;

	float predkosc = 3.0f;
	float czuloscMyszy = 0.001f;

	double xMouse, yMouse;

	Obiekt kostka = Obiekt();
	Obiekt kwadrat = Obiekt();
	Obiekt piramida = Obiekt();

	Obiekt lampa = Obiekt();

	kwadrat.PozycjaY = -1;
	kostka.PozycjaY = 3;
	lampa.PozycjaZ = 5;

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(szerokoscEkranu, wysokoscEkranu, "Silniczek Pawla", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.f, 0.3f, 0.1f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);






	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");


	//Deklaracje macierzy
	mat4 Projekcja, Kamera, Model, MVP, MVPkwadratu, MVPkostki, MVPlampy;





	GLuint Texture = loadBMP_custom("uvtemplate.bmp");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");




	static const GLfloat g_wierzcholki_piramidy[] = {
		//piramida

		//podstawa
		-1.0f, 0.0f,-1.0f,
		0.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		0.0f, 0.0f, 0.0f,
		-1.0f, 0.0f,-1.0f,

		//scianki
		-1.0f, 0.0f,-1.0f,
		0.0f, 1.5f, 0.0f,
		-1.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 1.0f,
		0.0f, 1.5f, 0.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		0.0f, 1.5f, 0.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		0.0f, 1.5f, 0.0f,
		-1.0f, 0.0f,-1.0f
	};

	static const GLfloat g_wierzcholki_kwadratu[] = {
		//skladany kwadrat

		//srodek
		-1.0f, 0.0f,-1.0f,
		0.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		0.0f, 0.0f, 0.0f,
		-1.0f, 0.0f,-1.0f,

		//zewnetrzne trojaty
		-1.0f, 0.0f,-1.0f,
		-2.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 2.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		2.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,

		1.0f, 0.0f,-1.0f,
		0.0f, 0.0f,-2.0f,
		-1.0f, 0.0f,-1.0f
	};





	static const GLfloat g_wierzcholki_kostki[] = { 
		//prostopadloscian
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end

		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end

		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,

		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,

		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,

		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,

		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,

		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,

		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};




	static GLfloat g_color_buffer_data[] = {
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f,
		0.435f,  0.602f,  0.223f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.014f,  0.184f,  0.576f,
		0.771f,  0.328f,  0.970f,
		0.406f,  0.615f,  0.116f,
		0.676f,  0.977f,  0.133f,
		0.971f,  0.572f,  0.833f,
		0.140f,  0.616f,  0.489f,
		0.997f,  0.513f,  0.064f,
		0.945f,  0.719f,  0.592f,
		0.543f,  0.021f,  0.978f,
		0.279f,  0.317f,  0.505f,
		0.167f,  0.620f,  0.077f,
		0.347f,  0.857f,  0.137f,
		0.055f,  0.953f,  0.042f,
		0.714f,  0.505f,  0.345f,
		0.783f,  0.290f,  0.734f,
		0.722f,  0.645f,  0.174f,
		0.302f,  0.455f,  0.848f,
		0.225f,  0.587f,  0.040f,
		0.517f,  0.713f,  0.338f,
		0.053f,  0.959f,  0.120f,
		0.393f,  0.621f,  0.362f,
		0.673f,  0.211f,  0.457f,
		0.820f,  0.883f,  0.371f,
		0.982f,  0.099f,  0.879f,
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f,
		0.435f,  0.602f,  0.223f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.014f,  0.184f,  0.576f,
		0.771f,  0.328f,  0.970f,
		0.406f,  0.615f,  0.116f,
		0.676f,  0.977f,  0.133f,
		0.971f,  0.572f,  0.833f,
		0.140f,  0.616f,  0.489f,
		0.997f,  0.513f,  0.064f,
		0.945f,  0.719f,  0.592f,
		0.543f,  0.021f,  0.978f,
		0.279f,  0.317f,  0.505f,
		0.167f,  0.620f,  0.077f,
		0.347f,  0.857f,  0.137f,
		0.055f,  0.953f,  0.042f,
		0.714f,  0.505f,  0.345f,
		0.783f,  0.290f,  0.734f,
		0.722f,  0.645f,  0.174f,
		0.302f,  0.455f,  0.848f,
		0.225f,  0.587f,  0.040f,
		0.517f,  0.713f,  0.338f,
		0.053f,  0.959f,  0.120f,
		0.393f,  0.621f,  0.362f,
		0.673f,  0.211f,  0.457f,
		0.820f,  0.883f,  0.371f,
		0.982f,  0.099f,  0.879f
	};

	// Two UV coordinatesfor each vertex. They were created with Blender. You'll learn shortly how to do this yourself.
	static const GLfloat g_uv_buffer_data[] = {
		0.000059f, 1.0f-0.000004f,
		0.000103f, 1.0f-0.336048f,
		0.335973f, 1.0f-0.335903f,
		1.000023f, 1.0f-0.000013f,
		0.667979f, 1.0f-0.335851f,
		0.999958f, 1.0f-0.336064f,
		0.667979f, 1.0f-0.335851f,
		0.336024f, 1.0f-0.671877f,
		0.667969f, 1.0f-0.671889f,
		1.000023f, 1.0f-0.000013f,
		0.668104f, 1.0f-0.000013f,
		0.667979f, 1.0f-0.335851f,
		0.000059f, 1.0f-0.000004f,
		0.335973f, 1.0f-0.335903f,
		0.336098f, 1.0f-0.000071f,
		0.667979f, 1.0f-0.335851f,
		0.335973f, 1.0f-0.335903f,
		0.336024f, 1.0f-0.671877f,
		1.000004f, 1.0f-0.671847f,
		0.999958f, 1.0f-0.336064f,
		0.667979f, 1.0f-0.335851f,
		0.668104f, 1.0f-0.000013f,
		0.335973f, 1.0f-0.335903f,
		0.667979f, 1.0f-0.335851f,
		0.335973f, 1.0f-0.335903f,
		0.668104f, 1.0f-0.000013f,
		0.336098f, 1.0f-0.000071f,
		0.000103f, 1.0f-0.336048f,
		0.000004f, 1.0f-0.671870f,
		0.336024f, 1.0f-0.671877f,
		0.000103f, 1.0f-0.336048f,
		0.336024f, 1.0f-0.671877f,
		0.335973f, 1.0f-0.335903f,
		0.667969f, 1.0f-0.671889f,
		1.000004f, 1.0f-0.671847f,
		0.667979f, 1.0f-0.335851f
	};





	//GLfloat *g_color_buffer_data = new GLfloat[234];
	/*GLfloat g_color_buffer_data[234];

	for (int i = 0; i < 234; i++)
	{
	g_color_buffer_data[i] = 0.5f;
	}
	*/




	//Wczytywanie obiektu
	static const GLfloat g_element_buffer_data[] = {0, 1, 2};

	// Funkcje sprawdzania glebokosci na planie
	glEnable(GL_DEPTH_TEST);

	// Wyswietl fragment jezeli jest blizej kamery
	glDepthFunc(GL_LESS);


	
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.
	bool res = loadOBJ("lamp.obj", vertices, uvs, normals);

	glm::vec3* tmp_tab = new vec3[vertices.size()];
	for (int i = 0; i < vertices.size(); i++)
	{
		tmp_tab[i] = vertices.at(i);
	}

	GLuint vertexbuffer[4];
	glGenBuffers(4, vertexbuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_wierzcholki_kostki), g_wierzcholki_kostki, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_wierzcholki_piramidy), g_wierzcholki_piramidy, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_wierzcholki_kwadratu), g_wierzcholki_kwadratu, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tmp_tab) * 3, &tmp_tab, GL_STATIC_DRAW);

	GLuint uvbuffer[2];
	glGenBuffers(2, uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);




	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);

	do{
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 234, g_color_buffer_data, GL_STATIC_DRAW);


		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);



		//Stablizacja czasu pojedyÒczej klatki

		static double lastTime = glfwGetTime();
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);

		//Zczytanie pozycji Myszy
		glfwGetCursorPos(window, &xMouse, &yMouse);
		glfwSetCursorPos(window, szerokoscEkranu/2, wysokoscEkranu/2);



		katPoziomy += czuloscMyszy * float(szerokoscEkranu/2 - xMouse);
		katPionowy += czuloscMyszy * float(wysokoscEkranu/2 - yMouse);


		// Wektor kierunku patrzenia
		glm::vec3 direction(
			cos(katPionowy) * sin(katPoziomy),
			sin(katPionowy),
			cos(katPionowy) * cos(katPoziomy)
			);

		// Wektor w prawo od kierunku patrzenia
		glm::vec3 right = glm::vec3(
			sin(katPoziomy - 3.14f/2.0f),
			0,
			cos(katPoziomy - 3.14f/2.0f)
			);

		// wektor w gore wyciagniety z dwoch pozostalych
		glm::vec3 up = glm::cross( right, direction );


		//Sterowanie chodem
		// Move forward
		if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS){
			pozycja += direction * deltaTime * predkosc;
		}
		// Move backward
		if (glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS){
			pozycja -= direction * deltaTime * predkosc;
		}
		// Strafe right
		if (glfwGetKey(window, GLFW_KEY_D ) == GLFW_PRESS){
			pozycja += right * deltaTime * predkosc;
		}
		// Strafe left
		if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS){
			pozycja -= right * deltaTime * predkosc;
		}

		// Obroty kostki
		if (glfwGetKey(window, GLFW_KEY_J ) == GLFW_PRESS){
			kostka.RotacjaY -= deltaTime * 100.f;
			if(kostka.RotacjaY <= 0) kostka.RotacjaY = 360;
		}
		if (glfwGetKey(window, GLFW_KEY_L ) == GLFW_PRESS){
			kostka.RotacjaY += deltaTime * 100.f;
			if(kostka.RotacjaY >= 360) kostka.RotacjaY = 0;
		}

		if (glfwGetKey(window, GLFW_KEY_I ) == GLFW_PRESS){
			kostka.RotacjaZ -= deltaTime * 100.f;
			if(kostka.RotacjaZ <= 0) kostka.RotacjaZ = 360;
		}
		if (glfwGetKey(window, GLFW_KEY_K ) == GLFW_PRESS){
			kostka.RotacjaZ += deltaTime * 100.f;
			if(kostka.RotacjaZ >= 360) kostka.RotacjaZ = 0;
		}


		////////////////



		//Macierz projekcji czyli wyúwietlanego obszaru
		Projekcja = perspective(katWidzenia, 4.0f / 3.0f, 0.1f, 100.0f);

		// Macierz kamery czyli gdzie i w ktorym kierunku patrzy kamera
		Kamera = lookAt(pozycja, pozycja + direction, up);

		//Macierz modelu
		Model = mat4(1.0f);


		//Iloczyn Modelu, Kamery i Projekcji
		//Ewentualnie translacji, rotacji i tak dalej
		MVP = Projekcja*Kamera*Model;

		MVPkwadratu = Projekcja * Kamera * Przesun(kwadrat.PozycjaX, kwadrat.PozycjaY, kwadrat.PozycjaZ) * Model;

		MVPkostki = Projekcja * Kamera * Przesun(kostka.PozycjaX, kostka.PozycjaY, kostka.PozycjaZ)
			* Obroc(kostka.RotacjaX,0) * Obroc(kostka.RotacjaY,1) * Obroc(kostka.RotacjaZ,2)
			* Model;

		MVPlampy = Projekcja * Kamera * Przesun(lampa.PozycjaX, lampa.PozycjaY, lampa.PozycjaZ)
			* Obroc(kostka.RotacjaX,0) * Obroc(kostka.RotacjaY,1) * Obroc(kostka.RotacjaZ,2)
			* Model;

		/*
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,					// attribute. No particular reason for 1, but must match the layout in the shader.
			3,					// size
			GL_FLOAT,			// type
			GL_FALSE,			// normalized?
			0,					// stride
			(void*)0			// array buffer offset
			);
			*/
		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);


		// Atrybut o numerze 0 - wierzcho≥ki



		glEnableVertexAttribArray(0);

		//Wybranie macierzy MVP, wrzucanie do bufora i rysowanie kostki
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPkostki[0][0]);
		Draw(vertexbuffer, 0, 12);

		//Wybranie macierzy MVP, wrzucanie do bufora i rysowanie piramidy
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		Draw(vertexbuffer, 1, 8);


		//Wybranie macierzy MVP, wrzucanie do bufora i rysowanie kwadratu
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPkwadratu[0][0]);
		Draw(vertexbuffer, 2, 8);


		//Wybranie macierzy lapmy i wyúwietlanie lampy
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPlampy[0][0]);
		Draw(vertexbuffer, 3, 20);
		

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Check if the ESC key was pressed or the window was closed

		lastTime = currentTime;




	}while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0 );

	//delete[] g_color_buffer_data;

	// Cleanup VBO
	glDeleteBuffers(2, vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

