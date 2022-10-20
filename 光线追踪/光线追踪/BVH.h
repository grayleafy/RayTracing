#ifndef BVH_H
#define BVH_H

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

class Triangle {
public:
	glm::vec3 v[3];
	glm::vec3 n[3];
	glm::vec2 tex[3];
	glm::vec3 ka, kd, ks;
	glm::vec3 center;



	void getCenter() {
		center = glm::vec3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < 3; i++) {
			center += v[i] / 3.0f;
		}
	}

	//返回最小边界
	glm::vec3 getMin() {
		float x = min(min(v[0].x, v[1].x), v[2].x);
		float y = min(min(v[0].y, v[1].y), v[2].y);
		float z = min(min(v[0].z, v[1].z), v[2].z);
		return glm::vec3(x, y, z);
	}
	//返回最大边界
	glm::vec3 getMax() {
		float x = max(max(v[0].x, v[1].x), v[2].x);
		float y = max(max(v[0].y, v[1].y), v[2].y);
		float z = max(max(v[0].z, v[1].z), v[2].z);
		return glm::vec3(x, y, z);
	}
};

//返回下边界
glm::vec3 getMinBound(glm::vec3 a, glm::vec3 b) {
	float x = min(a.x, b.x);
	float y = min(a.y, b.y);
	float z = min(a.z, b.z);
	return glm::vec3(x, y, z);
}
//返回上边界
glm::vec3 getMaxBound(glm::vec3 a, glm::vec3 b) {
	float x = max(a.x, b.x);
	float y = max(a.y, b.y);
	float z = max(a.z, b.z);
	return glm::vec3(x, y, z);
}

//节点
class BVHNode {
public:
	glm::vec3 minBound, maxBound;
	int dimension;
	int triangleNum;
	Triangle triangle;
	BVHNode *lc, *rc;

	//计算分割维度
	void calDimension() {
		glm::vec3 dif = maxBound - minBound;
		if (dif.x > dif.y && dif.x > dif.z)	dimension = 1;
		else if (dif.y > dif.z)				dimension = 2;
		else								dimension = 3;
	}
};

//树
class BVHTree {
public:
	BVHNode *root;
	vector<Triangle> triangles;

	void build(vector<Mesh> &meshes) {
		//读取三角形
		for (int i = 0; i < meshes.size(); i++) {
			for (int j = 0; 3 * j < meshes[i].indices.size(); j += 3) {
				Triangle temp;
				temp.ka = meshes[i].ambient;
				temp.kd = meshes[i].diffuse;
				temp.ks = meshes[i].specular;

				for (int k = 0; k < 3; k++) {
					temp.v[k] = meshes[i].vertices[meshes[i].indices[j + k]].Position;
					temp.n[k] = meshes[i].vertices[meshes[i].indices[j + k]].Normal;
					temp.tex[k] = meshes[i].vertices[meshes[i].indices[j + k]].TexCoords;
				}
				temp.getCenter();
				triangles.push_back(temp);
			}
		}
		
		//求根节点包围盒
		glm::vec3 maxBound = triangles[0].getMax();
		glm::vec3 minBound = triangles[0].getMin();
		for (int i = 1; i < triangles.size(); i++) {
			maxBound = getMaxBound(maxBound, triangles[i].getMax());
			minBound = getMinBound(minBound, triangles[i].getMin());
		}
		root = new BVHNode;
		root->maxBound = maxBound;
		root->minBound = minBound;
		root->triangleNum = triangles.size();

		
		//建树
		split(root, 0, triangles.size());

	}

	//分割节点，左闭右开
	void split(BVHNode *now, int l, int r) {
		if (now->triangleNum <= 0)	return;
		root->calDimension();

		//求分割点，然后巴拉巴拉
	}
};



#endif
#pragma once
