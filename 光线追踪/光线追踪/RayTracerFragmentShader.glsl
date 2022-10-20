#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

//贴图
uniform sampler2D historyTexture; //历史纹理
uniform sampler2D texVertex; //点坐标
uniform sampler2D texIndex; //索引
uniform sampler2D texMaterial; //材质光照参数


uniform int vertexNum;
uniform int indexNum;

// ************ 随机数功能 ************** //
uniform float randOrigin;
uint wseed;
float randcore(uint seed) {
	seed = (seed ^ uint(61)) ^ (seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> uint(4));
	seed *= uint(0x27d4eb2d);
	wseed = seed ^ (seed >> uint(15));
	return float(wseed) * (1.0 / 4294967296.0);
}
//生成0.0到1.0的随机数
float rand() {
	return randcore(wseed);
}

//屏幕及相机
uniform int screenWidth;
uniform int screenHeight;
struct Camera {
	vec3 camPos;
	vec3 front;
	vec3 right;
	vec3 up;
	float halfH;
	float halfW;
	vec3 leftbottom;
	int LoopNum;
};
uniform struct Camera camera;

//三角形
struct Triangle {
	vec3 v0, v1, v2;
	vec3 n0, n1, n2;
	vec2 u0, u1, u2;
	vec3 ka, kd, ks;
};
float At(sampler2D dataTex, float index) {
	float row = (index + 0.5) / textureSize(dataTex, 0).x;
	float y = (int(row) + 0.5) / textureSize(dataTex, 0).y;
	float x = (index + 0.5 - int(row) * textureSize(dataTex, 0).x) / textureSize(dataTex, 0).x;
	vec2 texCoord = vec2(x, y);
	return texture2D(dataTex, texCoord).x;
}

Triangle getTriangle(int index) {
	Triangle tri_t;
	float face0Index = At(texIndex, float(index * 3));
	float face1Index = At(texIndex, float(index * 3 + 1));
	float face2Index = At(texIndex, float(index * 3 + 2));

	tri_t.v0.x = At(texVertex, face0Index * 8.0);
	tri_t.v0.y = At(texVertex, face0Index * 8.0 + 1.0);
	tri_t.v0.z = At(texVertex, face0Index * 8.0 + 2.0);

	tri_t.v1.x = At(texVertex, face1Index * 8.0);
	tri_t.v1.y = At(texVertex, face1Index * 8.0 + 1.0);
	tri_t.v1.z = At(texVertex, face1Index * 8.0 + 2.0);

	tri_t.v2.x = At(texVertex, face2Index * 8.0);
	tri_t.v2.y = At(texVertex, face2Index * 8.0 + 1.0);
	tri_t.v2.z = At(texVertex, face2Index * 8.0 + 2.0);

	//材质
	tri_t.ka.x = At(texMaterial, face0Index * 9.0);
	tri_t.ka.y = At(texMaterial, face0Index * 9.0 + 1.0);
	tri_t.ka.z = At(texMaterial, face0Index * 9.0 + 2.0);

	tri_t.kd.x = At(texMaterial, face0Index * 9.0 + 3.0);
	tri_t.kd.y = At(texMaterial, face0Index * 9.0 + 4.0);
	tri_t.kd.z = At(texMaterial, face0Index * 9.0 + 5.0);

	tri_t.ks.x = At(texMaterial, face0Index * 9.0 + 6.0);
	tri_t.ks.y = At(texMaterial, face0Index * 9.0 + 7.0);
	tri_t.ks.z = At(texMaterial, face0Index * 9.0 + 8.0);
	
	//tri_t.v0 = tri[0].v0;
	//tri_t.v1 = tri[0].v1;
	//tri_t.v2 = tri[0].v2;

	return tri_t;
}


//光线定义
struct Ray {
	vec3 origin;
	vec3 direction;
};

struct hitRecord {
	vec3 Normal;
	vec3 Pos;
	vec3 direction;
	vec3 albedo;
	int materialIndex;
	int triangleIndex;
	int times;
	vec3 value;
};
hitRecord rec;


// 返回值：ray到三角形交点的距离
float hitTriangle(Triangle tri, Ray r) {
	// 找到三角形所在平面法向量
	vec3 A = tri.v1 - tri.v0;
	vec3 B = tri.v2 - tri.v0;
	vec3 N = normalize(cross(A, B));
	// Ray与平面平行，没有交点
	if (dot(N, r.direction) == 0) return -1.0;
	float D = -dot(N, tri.v0);
	float t = -(dot(N, r.origin) + D) / dot(N, r.direction);
	if (t < 0) return -1.0;
	// 计算交点
	vec3 pHit = r.origin + t * r.direction;
	vec3 edge0 = tri.v1 - tri.v0;
	vec3 C0 = pHit - tri.v0;
	if (dot(N, cross(edge0, C0)) < 0) return -1.0;
	vec3 edge1 = tri.v2 - tri.v1;
	vec3 C1 = pHit - tri.v1;
	if (dot(N, cross(edge1, C1)) < 0) return -1.0;
	vec3 edge2 = tri.v0 - tri.v2;
	vec3 C2 = pHit - tri.v2;
	if (dot(N, cross(edge2, C2)) < 0) return -1.0;
	// 光线与Ray相交
	return t - 0.00001; //防止多次反射
}

//插值求法线，需修改
vec3 getTriangleNormal(Triangle tri, vec3 p){
	return normalize(cross(tri.v2 - tri.v0, tri.v1 - tri.v0));
}

int hitObject(Ray r){
	float dis = 100000;
	bool ifHitTriangle = false;
	int hitTriangleIndex = -1;

	// 计算Mesh
	for (int i = 0; i < indexNum / 3; i++) {
		float dis_t = hitTriangle(getTriangle(i), r);
		if (dis_t > 0 && dis_t < dis) {
			dis = dis_t;
			hitTriangleIndex = i;
			ifHitTriangle = true;
		}
	}

	if (hitTriangleIndex == -1){
		return 0;
	}

	//保存碰撞数据
	rec.Pos = r.origin + dis * r.direction;
	rec.triangleIndex = hitTriangleIndex;
	rec.albedo = vec3(0.87,0.77,0.12);
	rec.materialIndex = 1;

	return 1;
}


hitRecord stack[256];
int reflectionNum;

vec3 random_in_unit_sphere(){
	float z = 2.0 * rand() - 1.0;
	float a = rand() * 360.0;
	float b = radians(a);
	return vec3(cos(b) * sqrt(1 - z * z), sin(b) * sqrt(1 - z * z), z);
}



//漫反射方向
vec3 diffuseReflection(vec3 Normal) {	
	vec3 temp = random_in_unit_sphere();
	if (dot(temp, Normal) < 0)	temp = -temp;
	return temp;	
}

//镜面反射方向
vec3 specularReflection(vec3 Normal, vec3 inDirection){
	return inDirection - 2.0 * dot(Normal, inDirection) * Normal;
}

vec3 shading(Ray r){
	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	reflectionNum = 0;

	rec.Pos = r.origin;
	rec.direction = r.direction;
	rec.times = 0;
	rec.value = vec3(1.0f, 1.0f, 1.0f);

	stack[reflectionNum++] = rec;

	vec3 sumValue = vec3(0.0, 0.0, 0.0);
	while (reflectionNum > 0){
		hitRecord now = stack[--reflectionNum];
		r.origin = now.Pos;
		r.direction = now.direction;


		if (hitObject(r) == 1){
			Triangle tri = getTriangle(rec.triangleIndex);

			//光源
			if (tri.ka.x > 0.0 || tri.ka.x > 0.0 || tri.ka.x > 0.0){
				color += vec3(now.value.x * tri.ka.x, now.value.y * tri.ka.y, now.value.z * tri.ka.z);
				continue;
			}

			//反射次数上限
			if (now.times > 5)	continue;

			//漫反射
			if (length(tri.kd) > 0.0){
				int t = 10;
				if (now.times > 1){
					t = 1;
				}
				for (int i = 0; i < t; i++){
					hitRecord temp;
					temp.Normal = getTriangleNormal(tri, rec.Pos);
					if (dot(now.direction, temp.Normal) > 0){
						temp.Normal = -temp.Normal;
					}
					temp.direction = diffuseReflection(temp.Normal);
					temp.Pos = rec.Pos;
					temp.value = now.value * sqrt(tri.kd) / t;
					temp.times = now.times + 1;
					stack[reflectionNum++] = temp;
				}
			}
			//镜面反射
			if (length(tri.ks) > 0.0){
				hitRecord temp;
				temp.Normal = getTriangleNormal(tri, rec.Pos);
				if (now.direction.x * temp.Normal.x + now.direction.y * temp.Normal.y + now.direction.z * temp.Normal.z > 0){
					temp.Normal = -temp.Normal;
				}
				temp.direction = specularReflection(temp.Normal, now.direction);
				temp.Pos = rec.Pos;
				temp.value = now.value * tri.ks;
				temp.times = now.times + 1;
				stack[reflectionNum++] = temp;
			}

		}
		else{
			continue;
		}
	}
	//还没有除以value
	//color /= sumValue;
	return color;
}

void main() {
	//随机数初始化
	wseed = uint(randOrigin * float(6.95857) * (TexCoords.x * TexCoords.y));


	// 获取历史帧信息
	vec3 hist = texture(historyTexture, TexCoords).rgb;

	//光线方向
	Ray cameraRay;
	cameraRay.origin = camera.camPos;
	cameraRay.direction = normalize(camera.leftbottom + (TexCoords.x * 2.0 * camera.halfW) * camera.right + (TexCoords.y * 2.0 * camera.halfH) * camera.up);
	
	//路径追踪
	vec3 curColor = shading(cameraRay);

	//合并结果
	curColor = (1.0 / float(camera.LoopNum))*curColor + (float(camera.LoopNum - 1) / float(camera.LoopNum))*hist;
	FragColor = vec4(curColor, 1.0);
}