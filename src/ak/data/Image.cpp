/**
 * Copyright 2017 Michael J. Baker
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include <ak/data/Image.hpp>
#include <ak/math/Scalar.hpp>
#include <cstring>
#include <algorithm>
#include <stdexcept>

using namespace akd;

Image1D::Image1D(const fpSingle* data, size_t components, size_t width) : m_componentCount(components), m_imageData(), m_w(width) {
	m_imageData.resize(m_w*m_componentCount);
	std::memcpy(m_imageData.data(), data, m_imageData.size()*sizeof(fpSingle));
}
Image1D::Image1D(const Image1D& other) : m_componentCount(other.componentCount()), m_imageData(other.m_imageData), m_w(other.m_w) {}
Image1D::Image1D(Image1D&& other) : m_componentCount(other.componentCount()), m_imageData(std::move(other.m_imageData)), m_w(other.m_w)  {}
Image1D::~Image1D() {}

void Image1D::set(uint32 x, fpSingle value) {
	if (m_componentCount == 1) m_imageData[x] = value;
	else if (m_componentCount == 2) set(x, akm::Vec2(value, 0));
	else if (m_componentCount == 3) set(x, akm::Vec3(value, 0, 0));
	else if (m_componentCount == 4) set(x, akm::Vec4(value, 0, 0, 1));
}

void Image1D::set(uint32 x, akm::Vec2 value) {
	if (m_componentCount == 1) set(x, value);
	else if (m_componentCount == 2) { m_imageData[x*2] = value.r; m_imageData[x*2 + 1] = value.g; }
	else if (m_componentCount == 3) set(x, akm::Vec3(value, 0));
	else if (m_componentCount == 4) set(x, akm::Vec4(value, 0, 1));
}

void Image1D::set(uint32 x, akm::Vec3 value) {
	if (m_componentCount == 1) set(x, value);
	else if (m_componentCount == 2) set(x, akm::Vec2(value));
	else if (m_componentCount == 3) { m_imageData[x*3] = value.r; m_imageData[x*3 + 1] = value.g; m_imageData[x*3 + 2] = value.b; }
	else if (m_componentCount == 4) set(x, akm::Vec4(value, 1));
}

void Image1D::set(uint32 x, akm::Vec4 value) {
	if (m_componentCount == 1) set(x, value);
	else if (m_componentCount == 2) set(x, akm::Vec2(value));
	else if (m_componentCount == 3) set(x, akm::Vec3(value));
	else if (m_componentCount == 4) { m_imageData[x*4] = value.r; m_imageData[x*4 + 1] = value.g; m_imageData[x*4 + 2] = value.b;  m_imageData[x*4 + 4] = value.a; }
}


fpSingle Image1D::asR(uint32 x) const {
	if (m_componentCount == 1) return m_imageData[x];
	else if (m_componentCount == 2) return asRG(x).r;
	else if (m_componentCount == 3) return asRGB(x).r;
	else if (m_componentCount == 4) return asRGBA(x).r;
	throw std::runtime_error("Bad component count");
}

akm::Vec2 Image1D::asRG(uint32 x) const {
	if (m_componentCount == 1) return akm::Vec2(asR(x), 0);
	else if (m_componentCount == 2) return akm::Vec2(m_imageData[x*2], m_imageData[x*2 + 1]);
	else if (m_componentCount == 3) return akm::Vec2(asRGB(x));
	else if (m_componentCount == 4) return akm::Vec2(asRGBA(x));
	throw std::runtime_error("Bad component count");
}

akm::Vec3 Image1D::asRGB(uint32 x) const {
	if (m_componentCount == 1) return akm::Vec3(asR(x), 0, 0);
	else if (m_componentCount == 2) return akm::Vec3(asRG(x), 0);
	else if (m_componentCount == 3) return akm::Vec3(m_imageData[x*3], m_imageData[x*3 + 1], m_imageData[x*3 + 2]);
	else if (m_componentCount == 4) return akm::Vec3(asRGBA(x));
	throw std::runtime_error("Bad component count");
}

akm::Vec4 Image1D::asRGBA(uint32 x) const {
	if (m_componentCount == 1) return akm::Vec4(asR(x), 0, 0, 1);
	else if (m_componentCount == 2) return akm::Vec4(asRG(x), 0, 1);
	else if (m_componentCount == 3) return akm::Vec4(asRGB(x), 1);
	else if (m_componentCount == 4) return akm::Vec4(m_imageData[x*4], m_imageData[x*4 + 1], m_imageData[x*4 + 2], m_imageData[x*4 + 3]);
	throw std::runtime_error("Bad component count");
}

size_t Image1D::componentCount() const {
	return m_componentCount;
}

void Image1D::setComponentCount(size_t targetFormat) {
	std::vector<fpSingle> newData;
	newData.resize(m_w*targetFormat, 0);

	for(auto i = 0u; i < m_w; i++) {
		auto minCount = akm::min(targetFormat, m_componentCount);
		for(auto c = 0u; c < minCount; c++) newData[i*targetFormat + c] = newData[i*m_componentCount + c];
		if ((targetFormat == 4) && (m_componentCount < 4)) newData[i*targetFormat + 3] = 1;
	}

	m_imageData = std::move(newData);
	m_componentCount = targetFormat;
}

const fpSingle* Image1D::data() const {
	return m_imageData.data();
}

size_t Image1D::width()  const {
	return m_w;
}

size_t Image1D::height() const {
	return 0;
}

size_t Image1D::depth()  const {
	return 0;
}

Image1D& Image1D::operator=(const Image1D& other) {
	m_w = other.m_w;
	m_componentCount = other.m_componentCount;
	m_imageData = other.m_imageData;
	return *this;
}

Image1D& Image1D::operator=(Image1D&& other) {
	m_w = other.m_w;
	m_componentCount = other.m_componentCount;
	m_imageData = std::move(other.m_imageData);
	return *this;
}



Image2D::Image2D(const fpSingle* data, size_t components, size_t width, size_t height) : m_componentCount(components), m_imageData(), m_w(width), m_h(height) {
	m_imageData.resize(m_w*m_h*m_componentCount);
	std::memcpy(m_imageData.data(), data, m_imageData.size()*sizeof(fpSingle));
}

Image2D::Image2D(const Image2D& other) : m_componentCount(other.m_componentCount), m_imageData(other.m_imageData), m_w(other.m_w), m_h(other.m_h) {}
Image2D::Image2D(Image2D&& other) : m_componentCount(other.m_componentCount), m_imageData(std::move(other.m_imageData)), m_w(other.m_w), m_h(other.m_h) {}
Image2D::~Image2D() {}

void Image2D::set(uint32 x, uint32 y, fpSingle value) {
	if (m_componentCount == 1) m_imageData[x + m_w*y] = value;
	else if (m_componentCount == 2) set(x, y, akm::Vec2(value, 0));
	else if (m_componentCount == 3) set(x, y, akm::Vec3(value, 0, 0));
	else if (m_componentCount == 4) set(x, y, akm::Vec4(value, 0, 0, 1));
}

void Image2D::set(uint32 x, uint32 y, akm::Vec2 value) {
	if (m_componentCount == 1) set(x, y, value);
	else if (m_componentCount == 2) { m_imageData[x*2 + m_w*y*2] = value.r; m_imageData[x*2 + m_w*y*2 + 1] = value.g; }
	else if (m_componentCount == 3) set(x, y, akm::Vec3(value, 0));
	else if (m_componentCount == 4) set(x, y, akm::Vec4(value, 0, 1));
}

void Image2D::set(uint32 x, uint32 y, akm::Vec3 value) {
	if (m_componentCount == 1) set(x, y, value);
	else if (m_componentCount == 2) set(x, y, akm::Vec2(value));
	else if (m_componentCount == 3) { m_imageData[x*3 + m_w*y*3] = value.r; m_imageData[x*3 + m_w*y*3 + 1] = value.g; m_imageData[x*3 + m_w*y*3 + 2] = value.b; }
	else if (m_componentCount == 4) set(x, y, akm::Vec4(value, 1));
}

void Image2D::set(uint32 x, uint32 y, akm::Vec4 value) {
	if (m_componentCount == 1) set(x, y, value);
	else if (m_componentCount == 2) set(x, y, akm::Vec2(value));
	else if (m_componentCount == 3) set(x, y, akm::Vec3(value));
	else if (m_componentCount == 4) { m_imageData[x*4 + m_w*y*4] = value.r; m_imageData[x*4 + m_w*y*4 + 1] = value.g; m_imageData[x*4 + m_w*y*4 + 2] = value.b;  m_imageData[x*4 + m_w*y*4 + 4] = value.a; }
}


fpSingle Image2D::asR(uint32 x, uint32 y) const {
	if (m_componentCount == 1) return m_imageData[x + m_w*y];
	else if (m_componentCount == 2) return asRG(x,y).r;
	else if (m_componentCount == 3) return asRGB(x, y).r;
	else if (m_componentCount == 4) return asRGBA(x, y).r;
	throw std::runtime_error("Bad component count");
}

akm::Vec2 Image2D::asRG(uint32 x, uint32 y) const {
	if (m_componentCount == 1) return akm::Vec2(asR(x, y), 0);
	else if (m_componentCount == 2) return akm::Vec2(m_imageData[x*2 + m_w*y*2], m_imageData[x*2 + m_w*y*2 + 1]);
	else if (m_componentCount == 3) return akm::Vec2(asRGB(x, y));
	else if (m_componentCount == 4) return akm::Vec2(asRGBA(x, y));
	throw std::runtime_error("Bad component count");
}

akm::Vec3 Image2D::asRGB(uint32 x, uint32 y) const {
	if (m_componentCount == 1) return akm::Vec3(asR(x, y), 0, 0);
	else if (m_componentCount == 2) return akm::Vec3(asRG(x, y), 0);
	else if (m_componentCount == 3) return akm::Vec3(m_imageData[x*3 + m_w*y*3], m_imageData[x*3 + m_w*y*3 + 1], m_imageData[x*3 + m_w*y*3 + 2]);
	else if (m_componentCount == 4) return akm::Vec3(asRGBA(x, y));
	throw std::runtime_error("Bad component count");
}

akm::Vec4 Image2D::asRGBA(uint32 x, uint32 y) const {
	if (m_componentCount == 1) return akm::Vec4(asR(x, y), 0, 0, 1);
	else if (m_componentCount == 2) return akm::Vec4(asRG(x, y), 0, 1);
	else if (m_componentCount == 3) return akm::Vec4(asRGB(x, y), 1);
	else if (m_componentCount == 4) return akm::Vec4(m_imageData[x*4], m_imageData[x*4 + m_w*y*4 + 1], m_imageData[x*4 + m_w*y*4 + 2], m_imageData[x*4 + m_w*y*4 + 3]);
	throw std::runtime_error("Bad component count");
}


size_t Image2D::componentCount() const {
	return m_componentCount;
}

void Image2D::setComponentCount(size_t targetFormat) {
	std::vector<fpSingle> newData;
	newData.resize(m_w*targetFormat, 0);

	for(auto i = 0u; i < m_w; i++) {
		for(auto j = 0u; j < m_h; j++) {
			auto minCount = akm::min(targetFormat, m_componentCount);
			for(auto c = 0u; c < minCount; c++) newData[i*targetFormat + j*m_w*targetFormat + c] = newData[i*m_componentCount + j*m_w*m_componentCount + c];
			if ((targetFormat == 4) && (m_componentCount < 4)) newData[i*targetFormat + 3] = 1;
		}
	}

	m_imageData = std::move(newData);
	m_componentCount = targetFormat;
}

const fpSingle* Image2D::data() const {
	return m_imageData.data();
}


size_t Image2D::width()  const {
	return m_w;
}

size_t Image2D::height() const {
	return m_h;
}

size_t Image2D::depth()  const {
	return 0;
}

Image2D& Image2D::operator=(const Image2D& other) {
	m_w = other.m_w;
	m_h = other.m_h;
	m_componentCount = other.m_componentCount;
	m_imageData = other.m_imageData;
	return *this;
}

Image2D& Image2D::operator=(Image2D&& other) {
	m_w = other.m_w;
	m_h = other.m_h;
	m_componentCount = other.m_componentCount;
	m_imageData = std::move(other.m_imageData);
	return *this;
}



Image3D::Image3D(const fpSingle* data, size_t components, size_t width, size_t height, size_t depth) : m_componentCount(components), m_imageData(), m_w(width), m_h(height), m_d(depth) {
	m_imageData.resize(m_w*m_h*m_d*m_componentCount);
	std::memcpy(m_imageData.data(), data, m_imageData.size()*sizeof(fpSingle));
}

Image3D::Image3D(const Image3D& other) : m_componentCount(other.m_componentCount), m_imageData(other.m_imageData), m_w(other.m_w), m_h(other.m_h), m_d(other.m_d) {}
Image3D::Image3D(Image3D&& other) : m_componentCount(other.m_componentCount), m_imageData(std::move(other.m_imageData)), m_w(other.m_w), m_h(other.m_h), m_d(other.m_d) {}
Image3D::~Image3D() {}

void Image3D::set(uint32 x, uint32 y, uint32 z, fpSingle value) {
	if (m_componentCount == 1) m_imageData[x + m_w*y + m_h*m_w*z] = value;
	else if (m_componentCount == 2) set(x, y, z,  akm::Vec2(value, 0));
	else if (m_componentCount == 3) set(x, y, z,  akm::Vec3(value, 0, 0));
	else if (m_componentCount == 4) set(x, y, z,  akm::Vec4(value, 0, 0, 0));
}

void Image3D::set(uint32 x, uint32 y, uint32 z, akm::Vec2 value) {
	if (m_componentCount == 1) set(x, y, z, value);
	else if (m_componentCount == 2) { m_imageData[x*2] = value.r; m_imageData[x*2 + m_w*y*2 + m_h*m_w*z*2 + 1] = value.g; }
	else if (m_componentCount == 3) set(x, y, z, akm::Vec3(value, 0));
	else if (m_componentCount == 4) set(x, y, z, akm::Vec4(value, 0, 1));
}

void Image3D::set(uint32 x, uint32 y, uint32 z, akm::Vec3 value) {
	if (m_componentCount == 1) set(x, y, z,  value);
	else if (m_componentCount == 2) set(x, y, z,  akm::Vec2(value));
	else if (m_componentCount == 3) { m_imageData[x*3] = value.r; m_imageData[x*3 + m_w*y*3 + m_h*m_w*z*3 + 1] = value.g; m_imageData[x*3 + m_w*y*3 + m_h*m_w*z*3 + 2] = value.b; }
	else if (m_componentCount == 4) set(x, y, z,  akm::Vec4(value, 1));
}

void Image3D::set(uint32 x, uint32 y, uint32 z, akm::Vec4 value) {
	if (m_componentCount == 1) set(x, y, z,  value);
	else if (m_componentCount == 2) set(x, y, z,  akm::Vec2(value));
	else if (m_componentCount == 3) set(x, y, z,  akm::Vec3(value));
	else if (m_componentCount == 4) { m_imageData[x*4] = value.r; m_imageData[x*4 + m_w*y*4 + m_h*m_w*z*4 + 1] = value.g; m_imageData[x*4 + m_w*y*4 + m_h*m_w*z*4 + 2] = value.b;  m_imageData[x*4 + m_w*y*4 + m_h*m_w*z*4 + 4] = value.a; }
}

fpSingle Image3D::asR(uint32 x, uint32 y, uint32 z) const {
	if (m_componentCount == 1) return m_imageData[x + m_w*y + m_w*m_h*z];
	else if (m_componentCount == 2) return asRG(x,y, z).r;
	else if (m_componentCount == 3) return asRGB(x, y, z).r;
	else if (m_componentCount == 4) return asRGBA(x, y, z).r;
	throw std::runtime_error("Bad component count");
}

akm::Vec2 Image3D::asRG(uint32 x, uint32 y, uint32 z) const {
	if (m_componentCount == 1) return akm::Vec2(asR(x, y, z), 0);
	else if (m_componentCount == 2) return akm::Vec2(m_imageData[x*2 + m_w*y*2 + m_w*m_h*z*2], m_imageData[x*2 + m_w*y*2 + m_w*m_h*z*2 + 1]);
	else if (m_componentCount == 3) return akm::Vec2(asRGB(x, y, z));
	else if (m_componentCount == 4) return akm::Vec2(asRGBA(x, y, z));
	throw std::runtime_error("Bad component count");
}

akm::Vec3 Image3D::asRGB(uint32 x, uint32 y, uint32 z) const {
	if (m_componentCount == 1) return akm::Vec3(asR(x, y, z), 0, 0);
	else if (m_componentCount == 2) return akm::Vec3(asRG(x, y, z), 0);
	else if (m_componentCount == 3) return akm::Vec3(m_imageData[x*3 + m_w*y*3 + m_w*m_h*z*3], m_imageData[x*3 + m_w*y*3 + m_w*m_h*z*3 + 1], m_imageData[x*3 + m_w*y*3 + m_w*m_h*z*3 + 2]);
	else if (m_componentCount == 4) return akm::Vec3(asRGBA(x, y, z));
	throw std::runtime_error("Bad component count");
}

akm::Vec4 Image3D::asRGBA(uint32 x, uint32 y, uint32 z) const {
	if (m_componentCount == 1) return akm::Vec4(asR(x, y, z), 0, 0, 1);
	else if (m_componentCount == 2) return akm::Vec4(asRG(x, y, z), 0, 1);
	else if (m_componentCount == 3) return akm::Vec4(asRGB(x, y, z), 1);
	else if (m_componentCount == 4) return akm::Vec4(m_imageData[x*4], m_imageData[x*4 + m_w*y*4 + m_w*m_h*z*4 + 1], m_imageData[x*4 + m_w*y*4 + m_w*m_h*z*4 + 2], m_imageData[x*4 + m_w*y*4 + m_w*m_h*z*4 + 3]);
	throw std::runtime_error("Bad component count");
}


size_t Image3D::componentCount() const {
	return m_componentCount;
}

void Image3D::setComponentCount(size_t targetFormat) {
	std::vector<fpSingle> newData;
	newData.resize(m_w*targetFormat, 0);

	for(auto i = 0u; i < m_w; i++) {
		for(auto j = 0u; j < m_h; j++) {
			for(auto k = 0u; k < m_d; k++) {
				auto minCount = akm::min(targetFormat, m_componentCount);
				for(auto c = 0u; c < minCount; c++) newData[i*targetFormat + j*m_w*targetFormat + k*m_w*m_h*targetFormat + c] = newData[i*m_componentCount + j*m_w*m_componentCount + k*m_w*m_h*m_componentCount + c];
				if ((targetFormat == 4) && (m_componentCount < 4)) newData[i*targetFormat + 3] = 1;
			}
		}
	}

	m_imageData = std::move(newData);
	m_componentCount = targetFormat;
}

const fpSingle* Image3D::data() const {
	return m_imageData.data();
}


size_t Image3D::width()  const {
	return m_w;
}

size_t Image3D::height() const {
	return m_h;
}

size_t Image3D::depth()  const {
	return m_d;
}


Image3D& Image3D::operator=(const Image3D& other) {
	m_w = other.m_w;
	m_h = other.m_h;
	m_d = other.m_d;
	m_componentCount = other.m_componentCount;
	m_imageData = other.m_imageData;
	return *this;
}

Image3D& Image3D::operator=(Image3D&& other) {
	m_w = other.m_w;
	m_h = other.m_h;
	m_d = other.m_d;
	m_componentCount = other.m_componentCount;
	m_imageData = std::move(other.m_imageData);
	return *this;
}
