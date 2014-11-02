/************************* 
*	Autor: Paweł Woźnica
*
*
**************************/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

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

mat4 Przesun(float x, float y, float z){

	mat4 Przesuniecie = glm::translate(
		glm::mat4(1.0f),
		glm::vec3(x, y, z));

	return Przesuniecie;

}

void Draw(GLuint tablicaBufora[], int indexTablicyBufora, int iloscTrojkatow){

	//Funkcja rysuje dana ilosc trojkatow(3arg) na danej tablicy punktów(1arg)
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

	vec3 pozycja = vec3(-3, 1, -5);

	int szerokoscEkranu = 1024;
	int wysokoscEkranu = 768;

	float katPoziomy = 3.14f;
	float katPionowy = 0.0f;

	float katWidzenia = 45.0f;

	float predkosc = 3.0f;
	float czuloscMyszy = 0.001f;

	double xMouse, yMouse;


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
	mat4 Projekcja, Kamera, Model, MVP, MVPkwadratu, MVPkostki;

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




	static const GLfloat g_color_buffer_data[] = {
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


	static const GLfloat g_element_buffer_data[] = {0, 1, 2};

	// Funkcje sprawdzania glebokosci na planie
	glEnable(GL_DEPTH_TEST);

	// Wyswietl fragment jezeli jest blizej kamery
	glDepthFunc(GL_LESS);






	GLuint vertexbuffer[3];
	glGenBuffers(3, vertexbuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_wierzcholki_kostki), g_wierzcholki_kostki, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_wierzcholki_piramidy), g_wierzcholki_piramidy, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_wierzcholki_kwadratu), g_wierzcholki_kwadratu, GL_STATIC_DRAW);




	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	

	do{

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);


		//Stablizacja czasu pojedyńczej klatki

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
		if (glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS){
			pozycja += direction * deltaTime * predkosc;
		}
		// Move backward
		if (glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS){
			pozycja -= direction * deltaTime * predkosc;
		}
		// Strafe right
		if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
			pozycja += right * deltaTime * predkosc;
		}
		// Strafe left
		if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS){
			pozycja -= right * deltaTime * predkosc;
		}

		

		//Macierz projekcji czyli wyświetlanego obszaru
		Projekcja = perspective(katWidzenia, 4.0f / 3.0f, 0.1f, 100.0f);

		// Macierz kamery czyli gdzie i w ktorym kierunku patrzy kamera
		Kamera = lookAt(pozycja, pozycja + direction, up);

		//Macierz modelu
		Model = mat4(1.0f);


		//Iloczyn Modelu, Kamery i Projekcji
		//Ewentualnie translacji, rotacji i tak dalej
		MVP = Projekcja*Kamera*Model;

		MVPkwadratu = Projekcja*Kamera*Przesun(-4, 0, 0)*Model;

		MVPkostki = Projekcja*Kamera*Przesun(4, 1, 0)*Model;
		

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


		// Atrybut o numerze 0 - wierzchołki

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



		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Check if the ESC key was pressed or the window was closed

		lastTime = currentTime;




	}while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO
	glDeleteBuffers(2, vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

