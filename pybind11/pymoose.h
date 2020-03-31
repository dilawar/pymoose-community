// =====================================================================================
//
//       Filename:  pymoose.h
//
//    Description: pymoose module.
//
//        Version:  1.0
//        Created:  03/17/2020 05:32:37 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//   Organization:  NCBS Bangalore
//
// =====================================================================================

#ifndef PYMOOSE_H
#define PYMOOSE_H

#include "../external/prettyprint.hpp"

template <typename T>
void setField(const ObjId& id, const string& fname, T val)
{
    // cout << "Setting " << fname << " to value " << val << endl;
    Field<T>::set(id, fname, val);
}

template <typename T>
T getField(const ObjId& id, const string& fname)
{
    return Field<T>::get(id, fname);
}

// FIXME: Is it most efficient?
// See discussion here: https://github.com/pybind/pybind11/issues/1042
template <typename T = double>
py::array_t<T> getFieldNumpy(const ObjId& id, const string& fname)
{
    auto v = Field<vector<T>>::get(id, fname);
    return py::array_t<T>(v.size(), v.data());
}

#endif /* end of include guard: PYMOOSE_H */
