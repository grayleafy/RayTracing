#pragma once
#ifndef __ObjectTexture_h__
#define __ObjectTexture_h__


#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Mesh.h"
#include "Shader.h"
//#include "BVHTree.h"

#include <cmath>
#include <vector>

class ObjectTexture {
public:
	GLuint tex_v;
	GLuint tex_f;
	GLuint tex_m;
	GLuint tex_tri;
	GLuint tex_bvh;
	int meshNum, meshFaceNum;

	void bindTex(Shader &shader) {

		//shader.setInt("meshNum", meshNum);
		//shader.setInt("bvhNodeNum", meshFaceNum);

		glActiveTexture(GL_TEXTURE0 + 1);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, tex_v);

		// 激活并绑定纹理
		glActiveTexture(GL_TEXTURE0 + 2);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, tex_f);

		glActiveTexture(GL_TEXTURE0 + 3);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, tex_m);

		shader.setInt("texVertex", 1);
		shader.setInt("texIndex", 2);
		shader.setInt("texMaterial", 3);
	}

	void bindTexBVH(Shader &shader) {

		//shader.setInt("meshNum", meshNum);
		//shader.setInt("bvhNodeNum", meshFaceNum);

		glActiveTexture(GL_TEXTURE0 + 4);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, tex_tri);

		// 激活并绑定纹理
		glActiveTexture(GL_TEXTURE0 + 5);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, tex_bvh);


		shader.setInt("texTriangle", 4);
		shader.setInt("texBVH", 5);
	}

};

void getTexture(BVHTree &tree, Shader& shader, ObjectTexture& objTex) {
	int triangleNum = 0, nodeNum = 0;
	triangleNum = tree.triangles.size();
	nodeNum = tree.size;
	cout << "三角形数量: " << triangleNum << "\n";
	cout << "bvh结点: " << nodeNum << "\n";
	
	//三角形
	{
		//求二维大小
		int y = ceilf(sqrt(triangleNum * 33));
		int x = (y + 33 - 1) / 33;

		float *triangles = new float[x * y * 33];
		int cnt = 0;
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++) {
				for (int k = 0; k < 3; k++) { //点个顶点包括8个浮点数数据
					triangles[i * x * 33 + j * 33 + k * 8 + 0] = tree.triangles[cnt].v[k][0];
					triangles[i * x * 33 + j * 33 + k * 8 + 1] = tree.triangles[cnt].v[k][1];
					triangles[i * x * 33 + j * 33 + k * 8 + 2] = tree.triangles[cnt].v[k][2];

					triangles[i * x * 33 + j * 33 + k * 8 + 3] = tree.triangles[cnt].n[k][0];
					triangles[i * x * 33 + j * 33 + k * 8 + 4] = tree.triangles[cnt].n[k][1];
					triangles[i * x * 33 + j * 33 + k * 8 + 5] = tree.triangles[cnt].n[k][2];

					triangles[i * x * 33 + j * 33 + k * 8 + 6] = tree.triangles[cnt].tex[k][0];
					triangles[i * x * 33 + j * 33 + k * 8 + 7] = tree.triangles[cnt].tex[k][1];
				}
				//光照参数
				triangles[i * x * 33 + j * 33 + 24 + 0] = tree.triangles[cnt].ka[0];
				triangles[i * x * 33 + j * 33 + 24 + 1] = tree.triangles[cnt].ka[1];
				triangles[i * x * 33 + j * 33 + 24 + 2] = tree.triangles[cnt].ka[2];

				triangles[i * x * 33 + j * 33 + 24 + 3] = tree.triangles[cnt].kd[0];
				triangles[i * x * 33 + j * 33 + 24 + 4] = tree.triangles[cnt].kd[1];
				triangles[i * x * 33 + j * 33 + 24 + 5] = tree.triangles[cnt].kd[2];

				triangles[i * x * 33 + j * 33 + 24 + 6] = tree.triangles[cnt].ks[0];
				triangles[i * x * 33 + j * 33 + 24 + 7] = tree.triangles[cnt].ks[1];
				triangles[i * x * 33 + j * 33 + 24 + 8] = tree.triangles[cnt].ks[2];

				cnt++;
				if (cnt >= triangleNum)	break;
			}
			if (cnt >= triangleNum)	break;
		}
		// 绑定到纹理中
		shader.use();

		//三角形，使用纹理4， texTriangle
		glGenTextures(1, &objTex.tex_tri);
		glBindTexture(GL_TEXTURE_2D, objTex.tex_tri);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, x * 33, y, 0, GL_RED, GL_FLOAT, triangles);
		// 最近邻插值
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_2D, objTex.tex_tri);
		shader.setInt("texTriangle", 4);
		shader.setInt("triangleNum", triangleNum);
		//删除
		delete[] triangles;
	}
	
	//BVH树
	{
		int y = ceilf(sqrt(nodeNum * 9));
		int x = (y + 9 - 1) / 9;
		float *bvhData = new float[x * y * 9];
		int cnt = 0;
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++) {
				bvhData[i * x * 9 + j * 9 + 0] = tree.linearTree[cnt].maxBound[0];
				bvhData[i * x * 9 + j * 9 + 1] = tree.linearTree[cnt].maxBound[1];
				bvhData[i * x * 9 + j * 9 + 2] = tree.linearTree[cnt].maxBound[2];

				bvhData[i * x * 9 + j * 9 + 3] = tree.linearTree[cnt].minBound[0];
				bvhData[i * x * 9 + j * 9 + 4] = tree.linearTree[cnt].minBound[1];
				bvhData[i * x * 9 + j * 9 + 5] = tree.linearTree[cnt].minBound[2];

				bvhData[i * x * 9 + j * 9 + 6] = tree.linearTree[cnt].triangleNum;
				bvhData[i * x * 9 + j * 9 + 7] = tree.linearTree[cnt].offsetOrRc;
				bvhData[i * x * 9 + j * 9 + 8] = tree.linearTree[cnt].axis;
				//cout << "axis: " << tree.linearTree[cnt].axis << endl;
				cnt++;

				if (cnt >= nodeNum)	break;
			}
			if (cnt >= nodeNum)	break;
		}
		// 绑定到纹理中
		shader.use();

		//BVH，使用纹理5， texBVH
		glGenTextures(1, &objTex.tex_bvh);
		glBindTexture(GL_TEXTURE_2D, objTex.tex_bvh);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, x * 9, y, 0, GL_RED, GL_FLOAT, bvhData);
		// 最近邻插值
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, objTex.tex_bvh);
		shader.setInt("texBVH", 5);
		shader.setInt("bvhNum", nodeNum);
		//删除
		delete[] bvhData;

	}
}

//不使用
void getTexture(std::vector<Mesh> & data, Shader& shader, ObjectTexture& objTex, float Scale = 1.0f, glm::vec3 bias = glm::vec3(0.0f)) {
	int dataSize_v = 0, dataSize_f = 0;
	for (int i = 0; i < data.size(); i++) {
		// 三角形顶点个数
		dataSize_v += data[i].vertices.size();
		// 三角形索引个数
		dataSize_f += data[i].indices.size();
	}
	std::cout << "dataSize_t = " << dataSize_v << std::endl;
	std::cout << "dataSize_f = " << dataSize_f << std::endl;


	//计算二维大小
	int v_size_x = ceilf(sqrt(dataSize_v));
	int v_size_y = (dataSize_v + v_size_x - 1) / v_size_x;
	int f_size_x = ceilf(sqrt(dataSize_f));
	int f_size_y = (dataSize_f + f_size_x - 1) / f_size_x;

	//3个顶点坐标， 3个法线， 2个纹理
	float *vertices = new float[v_size_x * v_size_y * (3 + 3 + 2)];
	float *indices = new float[f_size_x * f_size_y];
	float *material = new float[v_size_x * v_size_y * (3 * 3)];
	int index_v = 0, index_f = 0;
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].vertices.size(); j++) {
			vertices[index_v * 8 + 0] = data[i].vertices[j].Position.x;
			vertices[index_v * 8 + 1] = data[i].vertices[j].Position.y;
			vertices[index_v * 8 + 2] = data[i].vertices[j].Position.z;

			vertices[index_v * 8 + 3] = data[i].vertices[j].Normal.x;
			vertices[index_v * 8 + 4] = data[i].vertices[j].Normal.y;
			vertices[index_v * 8 + 5] = data[i].vertices[j].Normal.z;

			vertices[index_v * 8 + 6] = data[i].vertices[j].TexCoords.x;
			vertices[index_v * 8 + 7] = data[i].vertices[j].TexCoords.y;

			material[index_v * 9 + 0] = data[i].ambient.x;
			material[index_v * 9 + 1] = data[i].ambient.y;
			material[index_v * 9 + 2] = data[i].ambient.z;

			material[index_v * 9 + 3] = data[i].diffuse.x;
			material[index_v * 9 + 4] = data[i].diffuse.y;
			material[index_v * 9 + 5] = data[i].diffuse.z;

			material[index_v * 9 + 6] = data[i].specular.x;
			material[index_v * 9 + 7] = data[i].specular.y;
			material[index_v * 9 + 8] = data[i].specular.z;

			index_v++;
		}

		for (int j = 0; j < data[i].indices.size(); j++) {
			indices[index_f] = data[i].indices[j];
			index_f++;
		}
	}
	
	// 绑定到纹理中
	shader.use();

	//顶点，使用纹理1， texVertex
	glGenTextures(1, &objTex.tex_v);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_v);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, v_size_x * 8, v_size_y, 0, GL_RED, GL_FLOAT, vertices);
	// 最近邻插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_v);
	shader.setInt("texVertex", 1);
	shader.setInt("vertexNum", dataSize_v);

	//索引，使用纹理2， texIndex
	glGenTextures(1, &objTex.tex_f);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, f_size_x, f_size_y, 0, GL_RED, GL_FLOAT, indices);
	// 最近邻插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_f);
	shader.setInt("texIndex", 2);
	shader.setInt("indexNum", dataSize_f);

	//材质，使用纹理3， texMaterial
	glGenTextures(1, &objTex.tex_m);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_m);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, v_size_x * 9, v_size_y, 0, GL_RED, GL_FLOAT, material);
	// 最近邻插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_m);
	shader.setInt("texMaterial", 3);


	glActiveTexture(GL_TEXTURE0);



	

	// 删除数组
	delete[] vertices;
	delete[] indices;
	// 等测试完再删除
}





#endif






