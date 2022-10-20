#pragma once
#ifndef __Shape_h__
#define __Shape_h__

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

class Triangle {
public:
	glm::vec3 v0, v1, v2;
	glm::vec3 n0, n1, n2;
	glm::vec2 u0, u1, u2;
};

struct Bound3f {

	glm::vec3 pMin, pMax;

	Bound3f() {
		float minNum = std::numeric_limits<float>::lowest();
		float maxNum = std::numeric_limits<float>::max();
		pMin = glm::vec3(maxNum, maxNum, maxNum);
		pMax = glm::vec3(minNum, minNum, minNum);
	}
	Bound3f(const glm::vec3 &p1, const glm::vec3 &p2)
		: pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
			std::min(p1.z, p2.z)),
		pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
			std::max(p1.z, p2.z)) {}

	glm::vec3 Diagonal() const { return pMax - pMin; }
	int MaximumExtent() const {
		glm::vec3 d = Diagonal();
		if (d.x > d.y && d.x > d.z)
			return 0;
		else if (d.y > d.z)
			return 1;
		else
			return 2;
	}

	glm::vec3 Offset(const glm::vec3 &p) const {
		glm::vec3 o = p - pMin;
		if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
		if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
		if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
		return o;
	}

	float SurfaceArea() const {
		glm::vec3 d = Diagonal();
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

};

glm::vec3 getBoundp(const Bound3f& bound, const int i) {
	return (i == 0) ? bound.pMin : bound.pMax;
}

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

bool IntersectBound(const Bound3f& bounds, const Ray &ray, const glm::vec3 &invDir, const int* dirIsNeg) {
	// Check for ray intersection against $x$ and $y$ slabs
	float tMin = (getBoundp(bounds, dirIsNeg[0]).x - ray.origin.x) * invDir.x;
	float tMax = (getBoundp(bounds, 1 - dirIsNeg[0]).x - ray.origin.x) * invDir.x;
	float tyMin = (getBoundp(bounds, dirIsNeg[1]).y - ray.origin.y) * invDir.y;
	float tyMax = (getBoundp(bounds, 1 - dirIsNeg[1]).y - ray.origin.y) * invDir.y;

	// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
	if (tMin > tyMax || tyMin > tMax) return false;
	if (tyMin > tMin) tMin = tyMin;
	if (tyMax < tMax) tMax = tyMax;

	// Check for ray intersection against $z$ slab
	float tzMin = (getBoundp(bounds, dirIsNeg[2]).z - ray.origin.z) * invDir.z;
	float tzMax = (getBoundp(bounds, 1 - dirIsNeg[2]).z - ray.origin.z) * invDir.z;

	// Update _tzMax_ to ensure robust bounds intersection
	if (tMin > tzMax || tzMin > tMax) return false;
	if (tzMin > tMin) tMin = tzMin;
	if (tzMax < tMax) tMax = tzMax;

	return tMax > 0;
}


glm::vec3 Min(const glm::vec3 &p1, const glm::vec3 &p2) {
	return glm::vec3(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
		std::min(p1.z, p2.z));
}
glm::vec3 Max(const glm::vec3 &p1, const glm::vec3 &p2) {
	return glm::vec3(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
		std::max(p1.z, p2.z));
}

Bound3f Union(const Bound3f &b1, const Bound3f &b2) {
	Bound3f ret;
	ret.pMin = Min(b1.pMin, b2.pMin);
	ret.pMax = Max(b1.pMax, b2.pMax);
	//std::cout << ret.pMin.x << " " << ret.pMin.y << " " << ret.pMin.z << " " << ret.pMax.x << " " << ret.pMax.y << " " << ret.pMax.z << " " << std::endl;
	return ret;
}

Bound3f Union(const Bound3f &b, const glm::vec3 &p) {
	Bound3f ret;
	ret.pMin = Min(b.pMin, p);
	ret.pMax = Max(b.pMax, p);
	//std::cout << ret.pMin.x << " " << ret.pMin.y << " " << ret.pMin.z << " " << ret.pMax.x << " " << ret.pMax.y << " " << ret.pMax.z << " " << std::endl;
	return ret;
}

Bound3f getTriangleBound(const Triangle& tri) {
	Bound3f triBound = Union(Bound3f(tri.v0, tri.v1), tri.v2);
	//std::cout << triBound.pMin.x << " " << triBound.pMin.y << " " << triBound.pMin.z << " " << triBound.pMax.x << " " << triBound.pMax.y << " " << triBound.pMax.z << " " << std::endl;
	return triBound;
}


// 返回值：ray到三角形交点的距离
float hitTriangle(const Triangle& tri, const Ray& r) {
	// 找到三角形所在平面法向量
	glm::vec3 A = tri.v1 - tri.v0;
	glm::vec3 B = tri.v2 - tri.v0;
	glm::vec3 N = normalize(cross(A, B));
	// Ray与平面平行，没有交点
	if (dot(N, r.direction) == 0) return -1.0;
	float D = -dot(N, tri.v0);
	float t = -(dot(N, r.origin) + D) / dot(N, r.direction);
	if (t < 0) return -1.0;
	// 计算交点
	glm::vec3 pHit = r.origin + t * r.direction;
	glm::vec3 edge0 = tri.v1 - tri.v0;
	glm::vec3 C0 = pHit - tri.v0;
	if (dot(N, cross(edge0, C0)) < 0) return -1.0;
	glm::vec3 edge1 = tri.v2 - tri.v1;
	glm::vec3 C1 = pHit - tri.v1;
	if (dot(N, cross(edge1, C1)) < 0) return -1.0;
	glm::vec3 edge2 = tri.v0 - tri.v2;
	glm::vec3 C2 = pHit - tri.v2;
	if (glm::dot(N, cross(edge2, C2)) < 0) return -1.0;
	// 光线与Ray相交
	return t - 0.00001;
}








#endif





