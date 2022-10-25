#ifndef __RT_Screen_h__
#define __RT_Screen_h__

#include<glad/glad.h>
#include<GLFW/glfw3.h>

//屏幕坐标
const float ScreenVertices[] = {
	//位置坐标(x,y)     //纹理坐标

	//三角形1
	-1.0f, 1.0f,   0.0f, 1.0f, //左上角 
	-1.0f, -1.0f,  0.0f, 0.0f, //左下角
	1.0f, -1.0f,   1.0f, 0.0f, //右下角

	//三角形2
	-1.0f,  1.0f,  0.0f, 1.0f, //左上角
	1.0f, -1.0f,   1.0f, 0.0f, //右下角
	1.0f,  1.0f,   1.0f, 1.0f  //右上角
};

class RT_Screen {
public:
	void init() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenVertices), ScreenVertices, GL_STATIC_DRAW);
		//位置
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//纹理
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	void DrawScreen() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	~RT_Screen(){
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}
private:
	unsigned int VBO, VAO;
};

class ScreenFBO {
public:
	void configuration(int SCR_WIDTH, int SCR_HEIGHT) {
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			// 没有正确实现，则报错
		}
		// 绑定到默认FrameBuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		//glDisable(GL_DEPTH_TEST);
	}

	void unBind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void BindAsTexture() {
		// 作为第0个纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	}
	~ScreenFBO(){
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &framebuffer);
		glDeleteTextures(1, &textureColorbuffer);
	}
private:
	// framebuffer配置
	unsigned int framebuffer;
	// 颜色附件纹理
	unsigned int textureColorbuffer;
	// 深度和模板附件的renderbuffer object
	unsigned int rbo;
};

class RenderBuffer {
public:
	void init(int SCR_WIDTH, int SCR_HEIGHT) {
		fbo[0].configuration(SCR_WIDTH, SCR_HEIGHT);
		fbo[1].configuration(SCR_WIDTH, SCR_HEIGHT);
		currentIndex = 0;
	}
	void setCurrentBuffer(int LoopNum) {
		int histIndex = LoopNum % 2;
		int curIndex = (histIndex == 0 ? 1 : 0);

		fbo[curIndex].Bind();
		fbo[histIndex].BindAsTexture();
	}
	void setCurrentAsTexture(int LoopNum) {
		int histIndex = LoopNum % 2;
		int curIndex = (histIndex == 0 ? 1 : 0);
		fbo[curIndex].BindAsTexture();
	}
private:
	// 用于渲染当前帧的索引
	int currentIndex;
	ScreenFBO fbo[2];
};

#endif
#pragma once
