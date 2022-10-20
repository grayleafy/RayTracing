
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

class Scene {
public:
	void pushModel(Model &model) {
		for (int i = 0; i < model.meshes.size(); i++) {
			meshes.push_back(model.meshes[i]);
		}
	}

	vector<Mesh> meshes;
	
};

#endif
#pragma once
