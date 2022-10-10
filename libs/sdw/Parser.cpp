#include "Parser.h"

using namespace std;

Parser::Parser() {
	unordered_map<string, Colour> colours;
	string nextLine;
	ifstream mtlStream("C:\\Users\\izami\\Documents\\UoBYr3\\Graphics\\wireframes\\cornell-box.mtl", ifstream::binary);
	string currentColour;
	
	while (getline(mtlStream, nextLine)) {
		
		auto tokens = split(nextLine, ' ');
		if (tokens[0] == "newmtl") {
			//cout << tokens[1] << endl;
			currentColour = tokens[1];
		}
		else if (tokens[0] == "Kd") {
			//cout << tokens[1] << endl;
			Colour colour = Colour(stof(tokens[1]) * maxColour, stof(tokens[2]) * maxColour, stof(tokens[3]) * maxColour);
			//cout << colour << endl;
			colours[currentColour] = colour;
			//cout << colours[currentColour] << endl;
		}
		//else
			//cout << tokens[0] << endl;
	}
	//cout << colours.empty() << endl;
	//cout << colours["Green"] << endl << colours["Blue"] << endl;
	//auto it = colours.begin();
	//cout << "endl\n";
	ifstream objStream("C:\\Users\\izami\\Documents\\UoBYr3\\Graphics\\wireframes\\cornell-box.obj", ifstream::binary);
	
	Colour colour;
	while (getline(objStream, nextLine)) {
		auto tokens = split(nextLine, ' ');

		if (tokens[0] == "v") {
			glm::vec3 vertex;
			vertex[0] = stof(tokens[1]); vertex[1] = stof(tokens[2]); vertex[2] = stof(tokens[3]);
			vertices.push_back(vertex * scale);
		}
		else if (tokens[0] == "f") {
			vector<glm::vec3> corners;
			for (int i = 1; i < 4; i++) {
				



				string position = split(tokens[i], '/')[0];
				int index = stoi(position) - 1;
				//int index = stoi(split(tokens[i], '/')[0]) - 1;
				corners.push_back(vertices[index]);
			}
			
			ModelTriangle tri = ModelTriangle(corners[0], corners[1], corners[2], colour);
			triangles.push_back(tri);
		}
		else if (tokens[0] == "usemtl") {
			//cout << colours[tokens[1]] << endl;
			colour = colours[tokens[1]];
		}

	}
}

