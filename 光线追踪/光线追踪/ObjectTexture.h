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
	int meshNum, meshFaceNum;

	void bindTex(Shader &shader) {

		//shader.setInt("meshNum", meshNum);
		//shader.setInt("bvhNodeNum", meshFaceNum);

		glActiveTexture(GL_TEXTURE0 + 1);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, tex_v);

		// ���������
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

};


void getTexture(std::vector<Mesh> & data, Shader& shader, ObjectTexture& objTex, float Scale = 1.0f, glm::vec3 bias = glm::vec3(0.0f)) {
	int dataSize_v = 0, dataSize_f = 0;
	for (int i = 0; i < data.size(); i++) {
		// �����ζ������
		dataSize_v += data[i].vertices.size();
		// ��������������
		dataSize_f += data[i].indices.size();
	}
	std::cout << "dataSize_t = " << dataSize_v << std::endl;
	std::cout << "dataSize_f = " << dataSize_f << std::endl;


	//�����ά��С
	int v_size_x = ceilf(sqrt(dataSize_v));
	int v_size_y = (dataSize_v + v_size_x - 1) / v_size_x;
	int f_size_x = ceilf(sqrt(dataSize_f));
	int f_size_y = (dataSize_f + f_size_x - 1) / f_size_x;

	//3���������꣬ 3�����ߣ� 2������
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
	
	// �󶨵�������
	shader.use();

	//���㣬ʹ������1�� texVertex
	glGenTextures(1, &objTex.tex_v);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_v);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, v_size_x * 8, v_size_y, 0, GL_RED, GL_FLOAT, vertices);
	// ����ڲ�ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_v);
	shader.setInt("texVertex", 1);
	shader.setInt("vertexNum", dataSize_v);

	//������ʹ������2�� texIndex
	glGenTextures(1, &objTex.tex_f);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, f_size_x, f_size_y, 0, GL_RED, GL_FLOAT, indices);
	// ����ڲ�ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_f);
	shader.setInt("texIndex", 2);
	shader.setInt("indexNum", dataSize_f);

	//���ʣ�ʹ������3�� texMaterial
	glGenTextures(1, &objTex.tex_m);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_m);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, v_size_x * 9, v_size_y, 0, GL_RED, GL_FLOAT, material);
	// ����ڲ�ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, objTex.tex_m);
	shader.setInt("texMaterial", 3);


	glActiveTexture(GL_TEXTURE0);



	

	// ɾ������
	delete[] vertices;
	delete[] indices;
	// �Ȳ�������ɾ��
}





#endif






