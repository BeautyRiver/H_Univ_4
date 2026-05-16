#include "Rectangle.h"
CRectangle::CRectangle(float x, float y, float w, float h) : CShape(x, y), m_w(w), m_h(h) {

}

void CRectangle::Draw(HDC hdc) const {
    float left = m_x - (m_w / 2.0f);
    float top = m_y - (m_h / 2.0f);
    float right = m_x + (m_w / 2.0f);
    float bottom = m_y + (m_h / 2.0f);

    Rectangle(hdc, (int)left, (int)top, (int)right, (int)bottom);
}