#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include <math.h>
using namespace std;


class Mat4 {
public:
	float m[4][4];
	Mat4()
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = 0.0f;
	}

	Mat4 _Identity()
	{
		Mat4 mat;
		for (int i = 0; i < 4; i++)
			mat.m[i][i] = 1.0f;
		return mat;
	}

	float& operator[](int index)
	{
		int row = index / 4;
		int col = index % 4;
		return m[row][col];
	}
	float& operator()(int row, int col)
	{
		return m[row][col];
	}

	Mat4 operator*(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][0] * mat.m[0][j] +
					m[i][1] * mat.m[1][j] +
					m[i][2] * mat.m[2][j] +
					m[i][3] * mat.m[3][j];
			}
		}
		return result;
	}
	Mat4& operator*(float val)
	{
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m[i][j] *= val;
			}
		}
		return *this;
	}
	Mat4& operator*=(const Mat4& mat)
	{
		*this = *this * mat;
		return *this;
	}
	Mat4 operator+(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][j] + mat.m[i][j];
			}
		}
		return result;
	}
	Mat4& operator+=(const Mat4& mat)
	{
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m[i][j] += mat.m[i][j];
			}
		}
		return *this;
	}
	Mat4 operator-(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][j] - mat.m[i][j];
			}
		}
		return result;
	}
	Mat4 operator-() const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = -m[i][j];
			}
		}
		return result;
	}
	Mat4& operator-=(const Mat4& mat)
	{
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m[i][j] -= mat.m[i][j];
			}
		}
		return *this;
	}

	Mat4 Dot(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][0] * mat.m[0][j] +
					m[i][1] * mat.m[1][j] +
					m[i][2] * mat.m[2][j] +
					m[i][3] * mat.m[3][j];
			}
		}
		return result;
	}

	Mat4 Transpose() const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[j][i];
			}
		}
		return result;
	}

	Mat4 RotateX(float angle)
	{
		Mat4 mat = _Identity();
		float rad = angle * (float)M_PI / 180.0f;
		float c = cosf(rad);
		float s = sinf(rad);
		mat.m[1][1] = c;  mat.m[1][2] = -s;
		mat.m[2][1] = s;  mat.m[2][2] = c;
		return mat;
	}
	Mat4 RotateY(float angle)
	{
		Mat4 mat = _Identity();
		float rad = angle * (float)M_PI / 180.0f;
		float c = cosf(rad);
		float s = sinf(rad);
		mat.m[0][0] = c;  mat.m[0][2] = s;
		mat.m[2][0] = -s;  mat.m[2][2] = c;
		return mat;
	}
	Mat4 RotateZ(float angle)
	{
		Mat4 mat = _Identity();
		float rad = angle * (float)M_PI / 180.0f;
		float c = cosf(rad);
		float s = sinf(rad);
		mat.m[0][0] = c;  mat.m[0][1] = -s;
		mat.m[1][0] = s;  mat.m[1][1] = c;
		return mat;
	}
	Mat4 Translate(float x, float y, float z)
	{
		Mat4 mat = _Identity();
		mat.m[0][3] = x;
		mat.m[1][3] = y;
		mat.m[2][3] = z;
		return mat;
	}
	Mat4 Scale(float x, float y, float z)
	{
		Mat4 mat = _Identity();
		mat.m[0][0] = x;
		mat.m[1][1] = y;
		mat.m[2][2] = z;
		return mat;
	}

	Mat4 invert()
	{
		Mat4 original = *this;
		Mat4 inv;
		float det = original[0] * original[0] + original[1] * original[4] + original[2] * original[8] + original[3] * original[12];
		if (det == 0) {
			std::cout << "Matrix inversion failed: determinant is zero." << std::endl;
			return inv;
		}
		det = 1.0f / det;
		inv[0] = original[5] * original[10] * original[15] - original[5] * original[11] * original[14] - original[9] * original[6] * original[15] + original[9] * original[7] * original[14] + original[13] * original[6] * original[11] - original[13] * original[7] * original[10];
		inv[4] = -original[4] * original[10] * original[15] + original[4] * original[11] * original[14] + original[8] * original[6] * original[15] - original[8] * original[7] * original[14] - original[12] * original[6] * original[11] + original[12] * original[7] * original[10];
		inv[8] = original[4] * original[9] * original[15] - original[4] * original[11] * original[13] - original[8] * original[5] * original[15] + original[8] * original[7] * original[13] + original[12] * original[5] * original[11] - original[12] * original[7] * original[9];
		inv[12] = -original[4] * original[9] * original[14] + original[4] * original[10] * original[13] + original[8] * original[5] * original[14] - original[8] * original[6] * original[13] - original[12] * original[5] * original[10] + original[12] * original[6] * original[9];
		inv[1] = -original[1] * original[10] * original[15] + original[1] * original[11] * original[14] + original[9] * original[2] * original[15] - original[9] * original[3] * original[14] - original[13] * original[2] * original[11] + original[13] * original[3] * original[10];
		inv[5] = original[0] * original[10] * original[15] - original[0] * original[11] * original[14] - original[8] * original[2] * original[15] + original[8] * original[3] * original[14] + original[12] * original[2] * original[11] - original[12] * original[3] * original[10];
		inv[9] = -original[0] * original[9] * original[15] + original[0] * original[11] * original[13] + original[8] * original[1] * original[15] - original[8] * original[3] * original[13] - original[12] * original[1] * original[11] + original[12] * original[3] * original[9];
		inv[13] = original[0] * original[9] * original[14] - original[0] * original[10] * original[13] - original[8] * original[1] * original[14] + original[8] * original[2] * original[13] + original[12] * original[1] * original[10] - original[12] * original[2] * original[9];
		inv[2] = original[1] * original[6] * original[15] - original[1] * original[7] * original[14] - original[5] * original[2] * original[15] + original[5] * original[3] * original[14] + original[13] * original[2] * original[7] - original[13] * original[3] * original[6];
		inv[6] = -original[0] * original[6] * original[15] + original[0] * original[7] * original[14] + original[4] * original[2] * original[15] - original[4] * original[3] * original[14] - original[12] * original[2] * original[7] + original[12] * original[3] * original[6];
		inv[10] = original[0] * original[5] * original[15] - original[0] * original[7] * original[13] - original[4] * original[1] * original[15] + original[4] * original[3] * original[13] + original[12] * original[1] * original[7] - original[12] * original[3] * original[5];
		inv[14] = -original[0] * original[5] * original[14] + original[0] * original[6] * original[13] + original[4] * original[1] * original[14] - original[4] * original[2] * original[13] - original[12] * original[1] * original[6] + original[12] * original[2] * original[5];
		inv[3] = -original[1] * original[6] * original[11] + original[1] * original[7] * original[10] + original[5] * original[2] * original[11] - original[5] * original[3] * original[10] - original[9] * original[2] * original[7] + original[9] * original[3] * original[6];
		inv[7] = original[0] * original[6] * original[11] - original[0] * original[7] * original[10] - original[4] * original[2] * original[11] + original[4] * original[3] * original[10] + original[8] * original[2] * original[7] - original[8] * original[3] * original[6];
		inv[11] = -original[0] * original[5] * original[11] + original[0] * original[7] * original[9] + original[4] * original[1] * original[11] - original[4] * original[3] * original[9] - original[8] * original[1] * original[7] + original[8] * original[3] * original[5];
		inv[15] = original[0] * original[5] * original[10] - original[0] * original[6] * original[9] - original[4] * original[1] * original[10] + original[4] * original[2] * original[9] + original[8] * original[1] * original[6] - original[8] * original[2] * original[5];
		for (int i = 0; i < 16; i++) {
			inv[i] = inv[i] * det;
		}
		return inv;
	}

};
