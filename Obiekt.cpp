/************************* 
*	Autor: Pawe≥ Woünica
*
*	Rendered object class
**************************/

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
//#include <fstream>

#include <iostream>
#include "Obiekt.h"


using namespace glm;
using namespace std;
/*

class Obiekt
{
public:
	Obiekt();
	~Obiekt();
	bool LoadModel(char* path);

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
*/
Obiekt::Obiekt()
{
	//Obiekt::LoadModel();
	RotacjaZ = RotacjaY = RotacjaX = 0;	
	PozycjaX = PozycjaY = PozycjaZ = 0;
	PredkoscX = PredkoscY = PredkoscZ = 0;

}

Obiekt::~Obiekt()
{
}

vector<vec3> Obiekt::getVertices(){
	return out_vertices;
}

bool Obiekt::LoadModel(char *path){
	FILE * f = fopen(path, "r");
	if( f == NULL ){
		cout << "Impossible to open the file!" << endl;
		return false;
	}

	while(1){

		char lineHeader[128];
		int res = fscanf(f, "%s", lineHeader);
		if(res = EOF)
			break;

		if(strcmp(lineHeader, "v") == 0){
			vec3 v;
			fscanf(f, "%f %f %f/n", &v.x, &v.y, &v.z);
			vertices.push_back(v);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(f, "%f %f\n", &uv.x, &uv.y );
			uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(f, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if(matches != 9){
				cout << "Plik nie moøe byÊ odczytany, zbyt prosty parser" << endl;
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

		}
	}

	//Dla kaødego punktu kaødego trÛjkπta
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){
		unsigned int vertexIndex = vertexIndices[i];
		vec3 vertex = vertices[vertexIndex-1];
		out_vertices.push_back(vertex);
	}

	//Dla  UVs
	for( unsigned int i=0; i<uvIndices.size(); i++ ){
		unsigned int uvIndex = uvIndices[i];
		vec2 uv = uvs[uvIndex-1];
		out_uvs.push_back(uv);
	}

	//Dla normalnych
	for( unsigned int i=0; i<normalIndices.size(); i++ ){
		unsigned int normalIndex = normalIndices[i];
		vec3 normal = vertices[normalIndex-1];
		out_normals.push_back(normal);
	}

	return true;

}