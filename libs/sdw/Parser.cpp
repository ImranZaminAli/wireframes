#include "Parser.h"

Parser::Parser() {
	
	
	//ifstream mtlStream("cornell-box.mtl", ifstream::binary);
	ifstream mtlStream("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.mtl", ifstream::binary);
	string currentColour;
	while (getline(mtlStream, nextLine)) {
		auto tokens = split(nextLine, ' ');
		
		if (tokens[0] == "newmtl") {
			
			currentColour = tokens[1];
		}
		else if (tokens[0] == "Kd") {
			Colour colour = Colour(stof(tokens[1]) * maxColour, stof(tokens[2]) * maxColour, stof(tokens[3]) * maxColour);
			//std::cout << colour <<  " " << currentColour << std::endl;
			colours[currentColour] = colour;
		}
		else if (tokens[0] == "map_Kd") {
			colours[currentColour].textured = true;
		}
		else if (tokens[0] == "refl") {
			
			colours[currentColour].mirror = true;
		}
		else if (tokens[0] == "refr") {
			colours[currentColour].glass = true;
			colours[currentColour].rf = stof(tokens[1]);
		}
		
	}

	/*for (auto kv : colours) {
		std::cout << kv.second << std::endl;
	}*/
	//std::cout << colours[" "] << std::endl;
	//ifstream objStream("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.obj");
	ifstream objStream("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.obj", ifstream::binary);
	//ifstream objStream("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\sphere.obj", ifstream::binary);
	//ifstream objStream("cornell-box.obj", ifstream::binary);
	
	//readObj("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.obj", 1.0f);
	//readObj("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\untitled.obj", 1.0f);
	//readObj("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\untitled.obj", 1.0f);
	//readObj("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\sphere.obj", 1.0f);
	
	//string file = "C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\untitled.obj";

	Colour colour;
	vector<TexturePoint> textureVertices;
	
	while (getline(objStream, nextLine)) {
		auto tokens = split(nextLine, ' ');

		if (tokens[0] == "v") {
			glm::vec3 vertex;
			vertex[0] = stof(tokens[1]); vertex[1] = stof(tokens[2]); vertex[2] = stof(tokens[3]);
			vertices.push_back(vertex * scale );
			vertexNormals.push_back(glm::vec3(0, 0, 0));
		}
		else if (tokens[0] == "vt") {
			TexturePoint textureVertex = TexturePoint(stof(tokens[1]), stof(tokens[2]));
			/*textureVertex[0] = stof(tokens[1]);
			textureVertex[1] = stof(tokens[2]);*/
			textureVertices.push_back(textureVertex);
		}
		else if (tokens[0] == "f") {
			vector<glm::vec3> corners;
			vector<int> positions;
			for (int i = 1; i < 4; i++) {

				string position = split(tokens[i], '/')[0];
				int index = stoi(position) - 1;
				positions.push_back(index);
				corners.push_back(vertices[index]);
			}
			glm::vec3 normal = glm::normalize(glm::cross(corners[2] - corners[0], corners[1] - corners[0]));

			for (int i = 0; i < positions.size(); i++) {

				vertexNormals[positions[i]] += normal;
			}
		}
	}

	for (int i = 0; i < vertexNormals.size(); i++) {
		vertexNormals[i] = glm::normalize(vertexNormals[i]);
	}



	objStream.clear();
	objStream.seekg(0);

	while (getline(objStream, nextLine)) {

		auto tokens = split(nextLine, ' ');

		if (tokens[0] == "f") {
			vector<glm::vec3> vertexNorms;
			vector<glm::vec3> corners;
			vector<TexturePoint> textureCoords;
			for (int i = 1; i < 4; i++) {
				auto indexes = split(tokens[i], '/');
				int vertexIndex = stoi(indexes[0]) - 1;
				vertexNorms.push_back(vertexNormals[vertexIndex]);
				corners.push_back(vertices[vertexIndex]);
				if (colour.textured) {
					int textureIndex = stoi(indexes[1]) - 1;
					textureCoords.push_back(textureVertices[textureIndex]);
				}
			}

			ModelTriangle tri = ModelTriangle(corners[0], corners[1], corners[2], colour);
			tri.normal = glm::normalize(glm::cross(corners[2] - corners[0], corners[1] - corners[0]));
			tri.vertexNormals = vertexNorms;
			if (colour.textured) {
				tri.texturePoints[0] = textureCoords[0];
				tri.texturePoints[1] = textureCoords[1];
				tri.texturePoints[2] = textureCoords[2];
			}
			triangles.push_back(tri);
		}
		else if (tokens[0] == "usemtl") {
			std::cout << colours[tokens[1]] << std::endl;
			colour = colours[tokens[1]];
		}
	}

}
