#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
int debugFlag = 0;

//��ͼ
uniform sampler2D historyTexture; //��ʷ����
uniform sampler2D texVertex; //������
uniform sampler2D texIndex; //����
uniform sampler2D texMaterial; //���ʹ��ղ���
uniform sampler2D texTriangle; //����������
uniform sampler2D texBVH; //BVH��������


uniform int vertexNum;
uniform int indexNum;
uniform int triangleNum;
uniform int bvhNum;

struct Bound3f {
	vec3 pMin, pMax;
};

// ************ ��������� ************** //
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
//����0.0��1.0�������
float rand() {
	return randcore(wseed);
}

//��Ļ�����
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

//������
struct Triangle {
	vec3 v0, v1, v2;
	vec3 n0, n1, n2;
	vec2 u0, u1, u2;
	vec3 ka, kd, ks; //ks�������ʣ������ʣ��ֲڶ�
};

struct Sphere {
	vec3 center;
	float radius;
	vec3 albedo;
	float specularRate;
	float refractionRate;
	float roughness;
};
uniform Sphere sphere[10];
uniform int sphereNum = 0;

//��ȡ��������
float At(sampler2D dataTex, int index) {
	//float row = (index + 0.5) / textureSize(dataTex, 0).x;
	//float y = (int(row) + 0.5) / textureSize(dataTex, 0).y;
	//float x = (index + 0.5 - int(row) * textureSize(dataTex, 0).x) / textureSize(dataTex, 0).x;
	//vec2 texCoord = vec2(x, y);
	
	//return texture2D(dataTex, texCoord).x;
	
	int y = index / textureSize(dataTex, 0).x;
	int x = index - y * textureSize(dataTex, 0).x;
	return texelFetch(dataTex, ivec2(x, y), 0).x;

}

struct bvhNode{
	vec3 maxBound;
	vec3 minBound;
	int triangleNum;
	int offsetOrRc;
	int axis;
};

bvhNode getLinearBVHNode(int offset){
	bvhNode temp;
	int offset_t = offset * 9;
	temp.maxBound.x = At(texBVH, offset_t + 0);
	temp.maxBound.y = At(texBVH, offset_t + 1);
	temp.maxBound.z = At(texBVH, offset_t + 2);

	temp.minBound.x = At(texBVH, offset_t + 3);
	temp.minBound.y = At(texBVH, offset_t + 4);
	temp.minBound.z = At(texBVH, offset_t + 5);

	temp.triangleNum = int(At(texBVH, offset_t + 6) + 0.5);
	temp.offsetOrRc = int(At(texBVH, offset_t + 7) + 0.5);
	temp.axis = int(At(texBVH, offset_t + 8) + 0.5);

	return temp;
}



//��ȡ������
Triangle getTriangle(int offset){
	Triangle tri;

	tri.v0.x = At(texTriangle, int(offset * 33 + 0));
	tri.v0.y = At(texTriangle, int(offset * 33 + 1));
	tri.v0.z = At(texTriangle, int(offset * 33 + 2));
	tri.n0.x = At(texTriangle, int(offset * 33 + 3));
	tri.n0.y = At(texTriangle, int(offset * 33 + 4));
	tri.n0.z = At(texTriangle, int(offset * 33 + 5));
	tri.u0.x = At(texTriangle, int(offset * 33 + 6));
	tri.u0.y = At(texTriangle, int(offset * 33 + 7));

	tri.v1.x = At(texTriangle, int(offset * 33 + 8));
	tri.v1.y = At(texTriangle, int(offset * 33 + 9));
	tri.v1.z = At(texTriangle, int(offset * 33 + 10));
	tri.n1.x = At(texTriangle, int(offset * 33 + 11));
	tri.n1.y = At(texTriangle, int(offset * 33 + 12));
	tri.n1.z = At(texTriangle, int(offset * 33 + 13));
	tri.u1.x = At(texTriangle, int(offset * 33 + 14));
	tri.u1.y = At(texTriangle, int(offset * 33 + 15));

	tri.v2.x = At(texTriangle, int(offset * 33 + 16));
	tri.v2.y = At(texTriangle, int(offset * 33 + 17));
	tri.v2.z = At(texTriangle, int(offset * 33 + 18));
	tri.n2.x = At(texTriangle, int(offset * 33 + 19));
	tri.n2.y = At(texTriangle, int(offset * 33 + 20));
	tri.n2.z = At(texTriangle, int(offset * 33 + 21));
	tri.u2.x = At(texTriangle, int(offset * 33 + 22));
	tri.u2.y = At(texTriangle, int(offset * 33 + 23));

	tri.ka.x = At(texTriangle, int(offset * 33 + 24));
	tri.ka.y = At(texTriangle, int(offset * 33 + 25));
	tri.ka.z = At(texTriangle, int(offset * 33 + 26));

	tri.kd.x = At(texTriangle, int(offset * 33 + 27));
	tri.kd.y = At(texTriangle, int(offset * 33 + 28));
	tri.kd.z = At(texTriangle, int(offset * 33 + 29));

	tri.ks.x = At(texTriangle, int(offset * 33 + 30));
	tri.ks.y = At(texTriangle, int(offset * 33 + 31));
	tri.ks.z = At(texTriangle, int(offset * 33 + 32));

	return tri;
}

//���߶���
struct Ray {
	vec3 origin;
	vec3 direction;
	float hitMin;
};

struct hitRecord {
	vec3 Normal;
	vec3 Pos;
	vec3 albedo;
	int materialIndex; //0��Դ�� 1�����䣬 2���䣬 3���䣬 4����
	float roughness; //�ֲڶ�
	float refraction; //������
	float rayHitMin; 
};
hitRecord rec;


// ����ֵ��ray�������ν���ľ���
float hitTriangle(Triangle tri, Ray r) {
	// �ҵ�����������ƽ�淨����
	vec3 A = tri.v1 - tri.v0;
	vec3 B = tri.v2 - tri.v0;
	vec3 N = normalize(cross(A, B));
	// Ray��ƽ��ƽ�У�û�н���
	if (dot(N, r.direction) == 0) return -1.0;
	float D = -dot(N, tri.v0);
	float t = -(dot(N, r.origin) + D) / dot(N, r.direction);
	if (t < 0) return -1.0;
	// ���㽻��
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
	// ������Ray�ཻ
	return t - 0.00001; //��ֹ��η���
}

// ����ֵ��ray���򽻵�ľ���
float hitSphere(Sphere s, Ray r) {
	vec3 oc = r.origin - s.center;
	float a = dot(r.direction, r.direction);
	float b = 2.0 * dot(oc, r.direction);
	float c = dot(oc, oc) - s.radius * s.radius;
	float discriminant = b * b - 4 * a * c;
	if (discriminant > 0.0) {
		float dis = (-b - sqrt(discriminant)) / (2.0 * a);
		if (dis > 0.0) return dis;
		else return -1.0;
	}
	else return -1.0;
}

//��ֵ���ߣ����޸�
vec3 getTriangleNormal(Triangle tri, Ray r){
	vec3 res = normalize(cross(tri.v2 - tri.v0, tri.v1 - tri.v0));
	if (dot(res, r.direction) > 0)	res = -res;
	return res;
}





vec3 getBoundp(Bound3f bound, int i) {
	return (i == 0) ? bound.pMin : bound.pMax;
}
bool IntersectBound(Bound3f bounds, Ray ray, vec3 invDir, bool dirIsNeg[3]) {
	// Check for ray intersection against $x$ and $y$ slabs
	float tMin = (getBoundp(bounds, int(dirIsNeg[0])).x - ray.origin.x) * invDir.x;
	float tMax = (getBoundp(bounds, 1 - int(dirIsNeg[0])).x - ray.origin.x) * invDir.x;
	float tyMin = (getBoundp(bounds, int(dirIsNeg[1])).y - ray.origin.y) * invDir.y;
	float tyMax = (getBoundp(bounds, 1 - int(dirIsNeg[1])).y - ray.origin.y) * invDir.y;

	// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
	if (tMin > tyMax || tyMin > tMax) return false;
	if (tyMin > tMin) tMin = tyMin;
	if (tyMax < tMax) tMax = tyMax;

	// Check for ray intersection against $z$ slab
	float tzMin = (getBoundp(bounds, int(dirIsNeg[2])).z - ray.origin.z) * invDir.z;
	float tzMax = (getBoundp(bounds, 1 - int(dirIsNeg[2])).z - ray.origin.z) * invDir.z;

	// Update _tzMax_ to ensure robust bounds intersection
	if (tMin > tzMax || tzMin > tMax) return false;
	if (tzMin > tMin) tMin = tzMin;
	if (tzMax < tMax) tMax = tzMax;

	return tMax > 0;
}

bool IntersectBound_slow(Bound3f bounds, Ray ray, vec3 invDir, bool dirIsNeg[3]) {
	// Check for ray intersection against $x$ and $y$ slabs
	float tMin = (getBoundp(bounds, 1).x - ray.origin.x) / ray.direction.x;
	float tMax = (getBoundp(bounds, 0).x - ray.origin.x) / ray.direction.x;
	if (tMin > tMax) {
		float temp = tMin;
		tMin = tMax;
		tMax = temp;
	}
	float tyMin = (getBoundp(bounds, 1).y - ray.origin.y) / ray.direction.y;
	float tyMax = (getBoundp(bounds, 0).y - ray.origin.y) / ray.direction.y;
	if (tyMin > tyMax) {
		float temp = tyMin;
		tyMin = tyMax;
		tyMax = temp;
	}

	// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
	if (tMin > tyMax || tyMin > tMax) return false;
	if (tyMin > tMin) tMin = tyMin;
	if (tyMax < tMax) tMax = tyMax;

	// Check for ray intersection against $z$ slab
	float tzMin = (getBoundp(bounds, 1).z - ray.origin.z) / ray.direction.z;
	float tzMax = (getBoundp(bounds, 0).z - ray.origin.z) / ray.direction.z;
	if (tzMin > tzMax) {
		float temp = tzMin;
		tzMin = tzMax;
		tzMax = temp;
	}

	// Update _tzMax_ to ensure robust bounds intersection
	if (tMin > tzMax || tzMin > tMax) return false;
	if (tzMin > tMin) tMin = tzMin;
	if (tzMax < tMax) tMax = tzMax;

	return tMax > 0;
}

//�������ײ����������
bool hitBVH(Ray ray){
	bool hit = false;

	vec3 invDir = vec3(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
	bool dirIsNeg[3];
	dirIsNeg[0] = (invDir.x < 0.0); dirIsNeg[1] = (invDir.y < 0.0); dirIsNeg[2] = (invDir.z < 0.0);
	// Follow ray through BVH nodes to find primitive intersections
	int toVisitOffset = 0, currentNodeIndex = 0;
	int nodesToVisit[256];

	Triangle tri;
	///*
	while (true) {
		bvhNode node = getLinearBVHNode(currentNodeIndex);
		// Ray �� BVH�Ľ���
		Bound3f bound; bound.pMin = node.minBound; bound.pMax = node.maxBound;
		if (IntersectBound(bound, ray, invDir, dirIsNeg)) {
			if (node.triangleNum == 1) {
				// Ray �� Ҷ�ڵ�Ľ���
				for (int i = 0; i < node.triangleNum; ++i) {
					int offset = (node.offsetOrRc + i);
					Triangle tri_t = getTriangle(offset);
					float dis_t = hitTriangle(tri_t, ray);
					if (dis_t > 0 && dis_t < ray.hitMin) {
						ray.hitMin = dis_t;
						tri = tri_t;
						hit = true;
					}
				}
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
			else {
				// �� BVH node ���� _nodesToVisit_ stack, advance to near
				if (bool(dirIsNeg[node.axis])) {
					nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
					currentNodeIndex = node.offsetOrRc;
				}
				else {
					nodesToVisit[toVisitOffset++] = node.offsetOrRc;
					currentNodeIndex = currentNodeIndex + 1;
				}
			}
		}
		else {
			if (toVisitOffset == 0) break;
			currentNodeIndex = nodesToVisit[--toVisitOffset];
		}
	}
	//*/

	/*
	float dis = 100000.0;
	int que[256];
	que[0] = 0;
	int cnt = 1;
	while (cnt > 0) {
		int offset_t = que[--cnt];
		bvhNode node = getLinearBVHNode(offset_t);
		// Ray �� BVH�Ľ���
		Bound3f bound; bound.pMin = node.minBound; bound.pMax = node.maxBound;
		//����Ͱ�Χ�����ཻ
		if (IntersectBound(bound, ray, invDir, dirIsNeg)) {
			//�����Ҷ�ӽڵ�
			if (node.triangleNum == 1) {
				Triangle tri_t = getTriangle(node.offsetOrRc);
				float dis_t = hitTriangle(tri_t, ray);
				if (dis_t > 0 && dis_t < ray.hitMin) {
					ray.hitMin = dis_t;
					tri = tri_t;
					hit = true;
				}
			}
			//�������ӽ��������
			else {
				//if (r.direction[node.axis] > 0.0)
				que[cnt++] = offset_t + 1;
				//else
				que[cnt++] = node.offsetOrRc;
			}
		}
	}
	*/

	if (hit) {
		rec.Pos = ray.origin + ray.hitMin * ray.direction;
		rec.Normal = getTriangleNormal(tri, ray);
		//rec.albedo = vec3(0.83, 0.73, 0.1);

		float temp = rand();
		//��Դ
		if (length(tri.ka) > 0.0) {
			rec.materialIndex = 0;
			rec.albedo = tri.ka;
		}
		//������
		else if (temp < tri.ks.x) {
			rec.materialIndex = 1;
			rec.albedo = tri.kd;
		}
		//����
		else if (temp < tri.ks.y) {
			rec.materialIndex = 2;
			rec.albedo = vec3(1.0, 1.0, 1.0);
			rec.roughness = tri.ks.z;
		}
		//����
		else {
			rec.materialIndex = 3;
			rec.albedo = vec3(1.0, 1.0, 1.0);
			rec.refraction = 1.5;
			rec.roughness = tri.ks.z;
		}
		rec.rayHitMin = ray.hitMin;
		
	}
	return hit;
}



vec3 random_in_unit_sphere(){
	float z = 2.0 * rand() - 1.0;
	float a = rand() * 360.0;
	float b = radians(a);
	return vec3(cos(b) * sqrt(1 - z * z), sin(b) * sqrt(1 - z * z), z);
}



//�����䷽��
//vec3 diffuseReflection_q(vec3 Normal) {	
//	vec3 temp = random_in_unit_sphere_q();
//	if (dot(temp, Normal) < 0)	temp = -temp;
//	return temp;	
//}

vec3 random_in_unit_sphere_2() {
	vec3 p;
	do {
		p = 2.0 * vec3(rand(), rand(), rand()) - vec3(1, 1, 1);
	} while (dot(p, p) >= 1.0);
	return p;
}

//������
vec3 diffuseReflection(vec3 Normal) {
	return normalize(Normal + random_in_unit_sphere());
}

//���淴�䷽��
vec3 specularReflection(vec3 Normal, vec3 inDirection, float roughtness){
	return inDirection - 2.0 * dot(Normal, inDirection) * Normal + roughtness * random_in_unit_sphere();
}

//���䷽��
vec3 specularRefraction(vec3 Normal, vec3 inDirection, float roughness, float refraction) {
	Normal *= -dot(inDirection, Normal);
	vec3 temp = inDirection + Normal;
	temp /= refraction;
	temp = temp - Normal;
	temp = normalize(temp);
	return temp + roughness * random_in_unit_sphere();
}

bool hitWorld(Ray r) {
	bool hit = false;
	//������
	if (hitBVH(r)) {
		if (rec.rayHitMin > 0.0 && rec.rayHitMin < r.hitMin) {
			r.hitMin = rec.rayHitMin;
			hit = true;
		}
	}


	//��
	for (int i = 0; i < sphereNum; i++) {
		float dis_t = hitSphere(sphere[i], r);
		if (dis_t > 0.0 && dis_t < r.hitMin) {
			r.hitMin = dis_t;
			hit = true;

			rec.Pos = r.origin + r.hitMin * r.direction;
			rec.Normal = rec.Pos - sphere[i].center;
			bool internal = false;
			if (dot(rec.Normal, r.direction) > 0.0) {
				rec.Normal = -rec.Normal;
				internal = true;
			}

			float temp = rand();
			//������
			if (temp < sphere[i].specularRate) {
				r.hitMin -= 0.00001;
				rec.Pos = r.origin + r.hitMin * r.direction;
				rec.Normal = rec.Pos - sphere[i].center;
				rec.materialIndex = 1;
				rec.albedo = sphere[i].albedo;
			}
			//����
			else if (temp < sphere[i].refractionRate) {
				r.hitMin -= 0.00001;
				rec.Pos = r.origin + r.hitMin * r.direction;
				rec.Normal = rec.Pos - sphere[i].center;
				rec.materialIndex = 2;
				rec.albedo = vec3(1.0, 1.0, 1.0);
				rec.roughness = sphere[i].roughness;
			}
			//����
			else {
				r.hitMin += 0.00001;
				rec.Pos = r.origin + r.hitMin * r.direction;
				rec.Normal = rec.Pos - sphere[i].center;
				rec.materialIndex = 3;
				rec.albedo = vec3(1.0, 1.0, 1.0);
				rec.refraction = internal ? 1.0 / 1.5 : 1.5;
				rec.roughness = sphere[i].roughness;
			}
			rec.rayHitMin = r.hitMin;
		}
	}


	return hit;
}


vec3 shading(Ray r) {
	vec3 color = vec3(1.0, 1.0, 1.0);
	bool hitLight = false;
	for (int i = 0; i < 10; i++) {
		if (i > 1) {
			if (rand() > 0.8) {
				color = vec3(0, 0, 0);
				break;
			}
		}
		if (hitWorld(r)) {
			r.origin = rec.Pos;
			r.hitMin = 100000;

			color *= rec.albedo;
			
			if (rec.materialIndex == 1) {
				r.direction = diffuseReflection(rec.Normal);
				color *= dot(r.direction, rec.Normal);
			}
			else if (rec.materialIndex == 0) {
				break;
			}
			else if (rec.materialIndex == 2) {
				r.direction = specularReflection(rec.Normal, r.direction, rec.roughness);
			}
			else if (rec.materialIndex == 3) {
				r.direction = specularRefraction(rec.Normal, r.direction, rec.roughness, rec.refraction);
			}

			
		}
		else {
			/*
			if (i == 1) {
				vec3 lightPos = vec3(-4.0, 4.0, -4.0);
				vec3 lightDir = normalize(lightPos - rec.Pos);
				float diff = 0.5 * max(dot(rec.Normal, lightDir), 0.0) + 0.5;
				color *= vec3(diff, diff, diff);
			}
			else {
				float t = 0.5*(r.direction.y + 1.0);
				color *= (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
			}
			break;
			*/
		}
	}
	color *= 2.0 * 3.1415926;
	return color;
}

void main() {
	//�������ʼ��
	wseed = uint(randOrigin * float(6.95857) * (TexCoords.x * TexCoords.y));


	// ��ȡ��ʷ֡��Ϣ
	vec3 hist = texture(historyTexture, TexCoords).rgb;

	//���߷���
	Ray cameraRay;
	cameraRay.origin = camera.camPos;
	cameraRay.direction = normalize(camera.leftbottom + (TexCoords.x * 2.0 * camera.halfW) * camera.right + (TexCoords.y * 2.0 * camera.halfH) * camera.up);
	cameraRay.hitMin = 100000.0;

	//·��׷��
	vec3 curColor = shading(cameraRay);
	
	

	//�ϲ����
	curColor = (1.0 / float(camera.LoopNum))*curColor + (float(camera.LoopNum - 1) / float(camera.LoopNum))*hist;
	if (debugFlag == 0)	FragColor = vec4(curColor, 1.0);
}