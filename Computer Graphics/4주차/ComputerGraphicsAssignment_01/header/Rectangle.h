#pragma once
#include "Shape.h"

class CRectangle : public CShape {
public:
	float m_w;
	float m_h;

	CRectangle(float x, float y, float w, float h);
	void Draw(HDC hdc) const;
};