#ifndef CTL_MODULELAYOUT_H
#define CTL_MODULELAYOUT_H

#include <vector>

/*
 * NOTE: This is header only.
 */

typedef unsigned int uint; //!< Qt style alias for unsigned int.

namespace CTL {

/*!
 * \class ModuleLayout
 *
 * \brief Simple class that holds the layout of a multi module detector.
 *
 * The ModuleLayout class stores the layout of a detector that consists of multiple individual
 * flat panel modules. That means it holds the information about the arrangement of the individual
 * modules on the entire detector unit. For means of simplicity, this is limited to arrangement
 * patterns with a rectangular grid shape.
 *
 * A ModuleLayout is required if you want to combine projection data of individual modules to a
 * single projection. This can be done using SingleViewData::combined().
 *
 * To define a layout, the number of rows and columns of the grid need to be specified. Then, for
 * each position on the grid, the index of the flat panel module that is located at that spot must
 * be defined. This information is stored internally in a std::vector<int> with row-major order.
 * Negative module indices can be used to define gaps in the layout (see also
 * SingleViewData::combined()).
 *
 * For simple arrangements, the convenience factory method canonicLayout() can be used to easily
 * create the corresponding ModuleLayout.
 */
class ModuleLayout
{
public:
    ModuleLayout(uint nbRows = 0, uint nbCols = 0);

    int& operator()(uint row, uint col);
    const int& operator()(uint row, uint col) const;

    uint columns() const;
    uint rows() const;

    bool isEmpty() const;

    static ModuleLayout canonicLayout(uint nbRows, uint nbCols, bool rowMajorOrder = true);

private:
    uint _rows; //!< The number of rows in the layout.
    uint _cols; //!< The number of columns in the layout.
    std::vector<int> _layout; //!< The internal data vector.
};

/*!
 * Constructs a ModuleLayout object for a module arrangement with \a nbRows rows and \a nbCols
 * columns. The entire layout is initialized with index -1. Use operator() to assign module
 * indices to the layout positions.
 *
 * For simple layouts, consider using canonicLayout() for easy construction.
 *
 * \code
 * // setting up a linear layout with five modules by hand
 * ModuleLayout linearLayout(1,5);
 * for(uint col = 0; col < linearLayout.columns(); ++col)
 *     linearLayout(0, col) = col;
 * // The resulting layout is: | 0 || 1 || 2 || 3 || 4 |
 * // The same result can be achieved with canonicLayout(1,5).
 *
 * // arrange eight modules in a 3x3 square layout that has a gap in the center
 * ModuleLayout squareLayout(3, 3);
 * squareLayout(0,0) = 0, squareLayout(0,1) = 1, squareLayout(0,2) = 2;     // first row
 * squareLayout(1,0) = 3;                                                   // center row
 * // squareLayout(1,1); <- this is the gap (already initialized with -1)   // center row
 * squareLayout(1,2) = 4;                                                   // center row
 * squareLayout(2,0) = 5, squareLayout(2,1) = 6, squareLayout(2,2) = 7;     // last row
 * // The resulting layout is:
 * // | 0 || 1 || 2 |
 * //  -------------
 * // | 3 ||   || 4 |
 * //  -------------
 * // | 5 || 6 || 7 |
 * \endcode
 *
 * \sa canonicLayout().
 */
inline ModuleLayout::ModuleLayout(uint nbRows, uint nbCols)
    : _rows(nbRows)
    , _cols(nbCols)
    , _layout(nbRows * nbCols, -1)
{
}

/*!
 * Returns a (modifiable) reference to the module index at layout position [\a row, \a col].
 * \code
 * // setting up a linear layout with five modules
 * ModuleLayout linearLayout = ModuleLayout::canonicLayout(1, 6);   // layout: | 0 || 1 || 2 || 3 || 4 || 5 |
 *
 * // replace the odd numbered modules by their predecessors
 * for(uint m = 1; m < linearLayout.columns(); m += 2)
 *      linearLayout(0, m) = m-1;                                   // layout: | 0 || 0 || 2 || 2 || 4 || 4 |
 *
 * // remove all modules at these positions
 * for(uint m = 1; m < linearLayout.columns(); m += 2)
 *      linearLayout(0, m) = -1;                                    // layout: | 0 ||   || 2 ||   || 4 ||   |
 * \endcode
 */
inline int& ModuleLayout::operator()(uint row, uint col) { return _layout[row * _cols + col]; }

/*!
 * Returns a constant reference to the module index at layout position [\a row, \a col].
 */
inline const int& ModuleLayout::operator()(uint row, uint col) const { return _layout[row * _cols + col]; }

/*!
 * Returns the number of columns in the layout.
 */
inline uint ModuleLayout::columns() const { return _cols; }

/*!
 * Returns the number of rows in the layout.
 */
inline uint ModuleLayout::rows() const { return _rows; }

/*!
 * Returns true if either the number of rows or columns in this layout is zero.
 */
inline bool ModuleLayout::isEmpty() const { return (_rows == 0) || (_cols == 0); }

// clang-format off
/*!
 * Constructs and returns a ModuleLayout object for a module arrangement with \a nbRows rows
 * and \a nbCols columns. The layout is initialized with increasing module index across the layout.
 * By default, this will be done in row mayor order. To change this behavior to column major
 * order, set \a rowMajorOrder to \c false.
 *
 * \code
 * // setting up a linear layout (e.g. for cylindrical detectors) with ten modules
 * ModuleLayout linearLayout = ModuleLayout::canonicLayout(1, 10);
 * for(uint col = 0; col < linearLayout.columns(); ++col)
 *     std::cout << linearLayout(0, col) << " ";                        // output: 0 1 2 3 4 5 6 7 8 9
 *
 * // setting up a square layout with 3x3 modules in column major order.
 * ModuleLayout squareLayout = ModuleLayout::canonicLayout(3, 3, false);
 * for(uint row = 0; row < squareLayout.rows(); ++row){                 // output:
 *      for(uint col = 0; col < squareLayout.columns(); ++col)          // 0 3 6
 *          std::cout << squareLayout(row, col) << " ";                 // 1 4 7
 *      std::cout << std::endl;                                         // 2 5 8
 * }
 * \endcode
 */
// clang-format on
inline ModuleLayout ModuleLayout::canonicLayout(uint nbRows, uint nbCols, bool rowMajorOrder)
{
    ModuleLayout ret(nbRows, nbCols);
    int idx = 0;
    if(rowMajorOrder) // row major order
        for(uint r = 0; r < nbRows; ++r)
            for(uint c = 0; c < nbCols; ++c)
                ret(r, c) = idx++;
    else // column major order
        for(uint c = 0; c < nbCols; ++c)
            for(uint r = 0; r < nbRows; ++r)
                ret(r, c) = idx++;

    return ret;
}

} // namespace CTL

/*! \file */

#endif // CTL_MODULELAYOUT_H
