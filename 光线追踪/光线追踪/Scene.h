
#ifndef SCENE_H
#define SCENE_H

#include "glad/glad.h" 

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "stb_image.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Mesh.h"
#include "Shader.h"
#include "Model.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

class Sphere {
public:
	glm::vec3 center;
	float radius;
	glm::vec3 albedo;
	float specularRate;
	float refractionRate;
	float roughness;

	Sphere(glm::vec3 c, float r, glm::vec3 albedo, float sr, float rr, float rn) {
		center = c;
		radius = r;
		this->albedo = albedo;
		specularRate = sr;
		refractionRate = rr;
		roughness = rn;
	}
};

class Scene {
public:
	void pushModel(Model &model) {
		for (int i = 0; i < model.meshes.size(); i++) {
			meshes.push_back(model.meshes[i]);
		}
	}

	void pushModel(Model &model, glm::vec3 scale, glm::vec3 traslation = glm::vec3(1.0, 1.0, 1.0)) {
		for (int i = 0; i < model.meshes.size(); i++) {
			Mesh temp = model.meshes[i];
			for (int j = 0; j < temp.vertices.size(); j++) {
				temp.vertices[j].Position *= scale;
				temp.vertices[j].Position += traslation;
			}
			meshes.push_back(temp);
			cout << "specular: " << temp.specular.x << endl;
		}
		
	}

	void pushSphere(Sphere s) {
		spheres.push_back(s);
	}

	void setSphere(Shader &shader) {
		shader.use();
		shader.setInt("sphereNum", (int)spheres.size());
		for (int i = 0; i < spheres.size(); i++) {
			string s = "sphere[";
			s = s + to_string(i) + "].";

			shader.setVec3(s + "center", spheres[i].center);
			shader.setFloat(s + "radius", spheres[i].radius);
			shader.setVec3(s + "albedo", spheres[i].albedo);
			shader.setFloat(s + "specularRate", spheres[i].specularRate);
			shader.setFloat(s + "refractionRate", spheres[i].refractionRate);
			shader.setFloat(s + "roughness", spheres[i].roughness);
		}
	}

	vector<Sphere> spheres;
	vector<Mesh> meshes;
	
};

#endif
#pragma once
