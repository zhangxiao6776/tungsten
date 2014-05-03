#ifndef DISTRIBUTION2D_HPP_
#define DISTRIBUTION2D_HPP_

#include <core/Debug.hpp>
#include "math/MathUtil.hpp"

#include <algorithm>

namespace Tungsten {

class Distribution2D
{
    int _w, _h;
    std::vector<float> _marginalPdf, _marginalCdf;
    std::vector<float> _pdf;
    std::vector<float> _cdf;
public:
    Distribution2D(std::vector<float> weights, int w, int h)
    : _w(w), _h(h), _pdf(std::move(weights))
    {
        _cdf.resize(_pdf.size() + h);
        _marginalPdf.resize(h, 0.0f);
        _marginalCdf.resize(h + 1);

        _marginalCdf[0] = 0.0f;
        for (int y = 0, idx = 0; y < h; ++y) {
            int idxP = y*w;
            int idxC = y*(w + 1);

            _cdf[idxC] = 0.0f;
            for (int x = 0; x < w; ++x, ++idxP, ++idxC) {
                _marginalPdf[y] += weights[idxP];
                _cdf[idxC + 1] = _cdf[idxC] + _pdf[idxP];
            }
            _marginalCdf[y + 1] = _marginalCdf[y] + _marginalPdf[y];
        }

        for (int y = 0, idx = 0; y < h; ++y) {
            int idxP = y*w;
            int idxC = y*(w + 1);
            int idxTail = idxC + w;

            for (int x = 0; x < w; ++x, ++idxP, ++idxC) {
                _pdf[idxP] /= _cdf[idxTail];
                _cdf[idxC] /= _cdf[idxTail];
            }
            _cdf[idxTail] = 1.0f;
        }

        for (float &p : _marginalPdf)
            p /= _marginalCdf.back();
        for (float &c : _marginalCdf)
            c /= _marginalCdf.back();
        _marginalCdf.back() = 1.0f;
    }

    void warp(Vec2f &uv, int &row, int &column) const
    {
        row = std::distance(_marginalCdf.begin(), std::upper_bound(_marginalCdf.begin(), _marginalCdf.end(), uv.x())) - 1;
        uv.x() = clamp((uv.x() - _marginalCdf[row])/_marginalPdf[row], 0.0f, 1.0f);
        auto rowStart = _cdf.begin() + row*(_w + 1);
        auto rowEnd = rowStart + (_w + 1);
        column = std::distance(rowStart, std::upper_bound(rowStart, rowEnd, uv.y())) - 1;
        int idxC = row + column*(_w + 1);
        int idxP = row + column*_w;
        uv.y() = clamp((uv.y() - _cdf[idxC])/_pdf[idxP], 0.0f, 1.0f);
    }

    float pdf(int row, int column) const
    {
        return _pdf[row + column*_w]*_marginalPdf[row];
    }
};

}

#endif /* DISTRIBUTION1D_HPP_ */