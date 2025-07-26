// GridTransformer.h
#ifndef GRIDTRANSFORMER_H
#define GRIDTRANSFORMER_H

#include <cassert>
#include <vector>
#include "led-matrix.h" // Ensure this is included

// If Canvas and CanvasTransformer are defined in a different header, include it directly
#include "some-other-header.h" // Replace with the actual header file name

namespace rgb_matrix {
  // Forward declarations are not necessary if the full definitions are included.
  // class Canvas; 
  // class CanvasTransformer;
}

class GridTransformer : public rgb_matrix::Canvas, public rgb_matrix::CanvasTransformer {
public:
    struct Panel {
        int order;
        int rotate;
        int parallel;
    };

    GridTransformer(int width, int height, int panel_width, int panel_height,
                    int chain_length, const std::vector<Panel>& panels);

    virtual ~GridTransformer() {}

    // Canvas interface implementation:
    virtual int width() const {
        return _width;
    }
    virtual int height() const {
        return _height;
    }
    virtual void Clear() {
        assert(_source != NULL);
        _source->Clear();
    }
    virtual void Fill(uint8_t red, uint8_t green, uint8_t blue) {
        assert(_source != NULL);
        _source->Fill(red, green, blue);
    }
    virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);
    // Transformer interface implementation:
    virtual rgb_matrix::Canvas* Transform(rgb_matrix::Canvas* source);
    // Other attribute accessors.
    int getRows() const {
        return _rows;
    }
    int getColumns() const {
        return _cols;
    }

private:
    int _width, _height, _panel_width, _panel_height, _chain_length, _rows, _cols;
    rgb_matrix::Canvas* _source;
    std::vector<Panel> _panels;
};

#endif // GRIDTRANSFORMER_H