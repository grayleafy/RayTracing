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
		center = getMax() + getMin();
		center /= 2.0;
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
	int dimension = -1;;
	int triangleNum;
	int index;
	BVHNode *lc, *rc;

	//计算分割维度
	void calDimension() {
		glm::vec3 dif = maxBound - minBound;
		if (dif.x > dif.y && dif.x > dif.z)	dimension = 0;
		else if (dif.y > dif.z)				dimension = 1;
		else								dimension = 2;
	}
};

//线性BVH结点
class LinearBVHNode {
public:
	glm::vec3 minBound, maxBound;
	int triangleNum;
	int offsetOrRc;
	int axis;
};


//树
class BVHTree {
public:
	BVHNode *root;
	LinearBVHNode *linearTree;
	int LinearTreeCnt = 0;
	vector<Triangle> triangles;
	int size = 0;

	void build(vector<Mesh> &meshes) {
		//读取三角形
		//int cnt = 0;
		for (int i = 0; i < meshes.size(); i++) {
			for (int j = 0; j < meshes[i].indices.size(); j += 3) {
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
		
		//cout << "triangle size : " << triangles.size() << endl;
		//cout << "mesh size :" << cnt << endl;

		//求根节点包围盒
		glm::vec3 maxBound = triangles[0].getMax();
		glm::vec3 minBound = triangles[0].getMin();
		for (int i = 1; i < triangles.size(); i++) {
			maxBound = getMaxBound(maxBound, triangles[i].getMax());
			minBound = getMinBound(minBound, triangles[i].getMin());
		}
		
		

		
		//建树
		root = new BVHNode;
		size = 1;
		split(root, triangles.begin(), triangles.end());

	}

	class Bound {
	public:
		glm::vec3 maxP, minP;


		Bound() {

		}

		Bound(glm::vec3 maxP, glm::vec3 minP) {
			this->maxP = maxP;
			this->minP = minP;
		}

		Bound(Triangle tri) {
			maxP = tri.getMax();
			minP = tri.getMin();
		}

		Bound getUnion(Bound b) {
			return Bound(getMaxBound(maxP, b.maxP), getMinBound(minP, b.minP));
		}

		float getArea() {
			glm::vec3 dif = maxP - minP;
			return 2.0 * (dif.x * dif.y + dif.x * dif.z + dif.y + dif.z);
		}
	};

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
			now->index = begin - triangles.begin();
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
		nth_element(begin, begin + mid, end, [dim](const Triangle &a, const Triangle &b) {return a.center[dim] < b.center[dim]; });
		

		//SAH优化
		/*
		int n = 0;
		for (auto it = begin; it != end; it++)	n++;
		int axis = 0;
		int mid = 0;
		float area = 10000000000.0f;
		for (int i = 0; i < 3; i++) {
			sort(begin, end, [i](const Triangle &a, const Triangle &b) {return a.center[i] < b.center[i]; });
			vector<Bound> boundLeft(n), boundRight(n);
			boundLeft[0] = (Bound(*begin));
			boundRight[n - 1] = (Bound(*(end - 1)));
			for (int j = 1; j < n; j++){
				boundLeft[j] = boundLeft[j - 1].getUnion(Bound(*(begin + j)));
				boundRight[n - 1 - j] = boundRight[n - 1 - j + 1].getUnion(Bound(*(end - 1 - j)));
			}

			for (int j = 0; j < n - 1; j++) {
				float area_t = boundLeft[j].getArea() + boundRight[j + 1].getArea();
				if (area_t < area) {
					area = area_t;

					axis = i;
					mid = j;
				}
			}
		}

		sort(begin, end, [axis](const Triangle &a, const Triangle &b) {return a.center[axis] < b.center[axis]; });
		mid++;
		now->dimension = axis;
		*/

		now->lc = new BVHNode;
		size++;
		split(now->lc, begin, begin + mid);
		now->rc = new BVHNode;
		size++;
		split(now->rc, begin + mid, end);
	}

	//构造线性BVH树
	void buildLinerTree() {
		linearTree = new LinearBVHNode[size];

		addLinearNode(root);
	}

	void addLinearNode(BVHNode *now) {
		int k = LinearTreeCnt++;
		linearTree[k].maxBound = now->maxBound;
		linearTree[k].minBound = now->minBound;
		linearTree[k].triangleNum = now->triangleNum;
		linearTree[k].offsetOrRc = now->index;
		linearTree[k].axis = now->dimension;
		

		if (linearTree[k].triangleNum == 1)	return;

		if (now->lc != NULL)	addLinearNode(now->lc);
		linearTree[k].offsetOrRc = LinearTreeCnt;
		if (now->rc != NULL)	addLinearNode(now->rc);
	}

	/*
	void printLinearNode(int k) {
		if (linearTree[k].rc < k && linearTree[k].triangleNum != 1) {
			cout << "linearNode: (" << linearTree[k].maxBound.x << ", " << linearTree[k].maxBound.y << ", " << linearTree[k].maxBound.z << "), ("
				<< linearTree[k].minBound.x << ", " << linearTree[k].minBound.y << ", " << linearTree[k].minBound.z << ")\n";
			cout << "k = " << k << ", rc = " << linearTree[k].rc << endl;
			cout << "size: " << linearTree[k].triangleNum << endl;
		}
		
		if (linearTree[k].triangleNum == 1)	return;
		printLinearNode(k + 1);
		printLinearNode(linearTree[k].rc);
	}
	*/
};



#endif
#pragma once
