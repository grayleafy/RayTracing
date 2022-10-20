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

//三角形位置排序比较函数
static int BVH_dimension = 0;
bool cmpTriangle(Triangle &a, Triangle &b) {
	return a.center[BVH_dimension - 1] < b.center[BVH_dimension - 1];
}

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
		
		

		
		//建树
		root = new BVHNode;
		split(root, triangles.begin(), triangles.end());

	}

	//分割节点，左闭右开
	void split(BVHNode *now, vector<Triangle>::iterator begin, vector<Triangle>::iterator end) {
		//求包围盒
		now->maxBound = begin->getMax();
		now->minBound = begin->getMin();
		now->triangleNum = 1;
		for (auto it = begin + 1; it != end; it++) {
			now->maxBound = getMaxBound(now->maxBound, it->getMax());
			now->minBound = getMinBound(now->minBound, it->getMin());
			now->triangleNum++;
		}

		//输出
		int printFlag = 0;
		if (printFlag) {
			cout << "node: (" << now->maxBound.x << ", " << now->maxBound.y << ", " << now->maxBound.z << "), ("
						   	<< now->minBound.x << ", " << now->minBound.y << ", " << now->minBound.z << ")\n";
		}

		//保存三角形
		if (begin + 1 == end) {		
			now->triangleNum = 1;
			now->triangle = *begin;
			return;
		}

		//计算分割维度
		now->calDimension();
		//cout << "dimension:" << now->dimension << endl;
		//cout << "size： " << now->triangleNum << endl;

		//求分割点
		int mid = now->triangleNum / 2;
		BVH_dimension = now->dimension;
		int dim = now->dimension;
		nth_element(begin, begin + mid, end, [dim](const Triangle &a, const Triangle &b) {return a.center[dim - 1] < b.center[dim - 1]; });
		//cout << "mid: " << mid << "ok\n";

		now->lc = new BVHNode;
		split(now->lc, begin, begin + mid);
		now->rc = new BVHNode;
		split(now->rc, begin + mid, end);
	}


};



#endif
#pragma once
