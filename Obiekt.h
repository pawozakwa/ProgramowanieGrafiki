/************************* 
*	Autor: Pawe≥ Woünica
*
*	Rendered object class header
**************************/

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include GLEW
#include <GL/glew.h>

#include <vector>
#include <iostream>

using namespace glm;
using namespace std;


class Obiekt
{
public:
	Obiekt();
	~Obiekt();
	bool LoadModel(char* path);

	vector<vec3> getVertices();
	vector<vec2> getUvs();
	vector<vec3> getNormals();

	float RotacjaX, RotacjaY, RotacjaZ;
	int PozycjaX ,PozycjaY, PozycjaZ;
	int PredkoscX ,PredkoscY, PredkoscZ;

private:
	//Parametry obiektu
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;

	std::vector< glm::vec3 > out_vertices;
	std::vector< glm::vec2 > out_uvs;
	std::vector< glm::vec3 > out_normals;

};