/***
 *    Description:  moose.vec class.
 *
 *        Created:  2020-03-30

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  GPLv3
 */

#ifndef VEC_H

#define VEC_H

#include <vector>
using namespace std;

class MooseVec {

public:
    MooseVec(const string& path, size_t n = 1, const string& dtype = "Neutral")
        : path_(path), n_(n), dtype_(dtype)
    {
        if (!mooseExists(path)) {
            objs_.clear();
            ObjId o = mooseCreate(dtype, path, n);
            for (size_t i = 0; i < n; i++) objs_.push_back(ObjId(o, i));
        } else {
            objs_.clear();
            auto o = ObjId(path);
            for (size_t i = 0; i < o.element()->numData(); i++)
                objs_.push_back(ObjId(o, i));
        }
    }

    size_t len()
    {
        return objs_.size();
    }

    ObjId getElem(const size_t i)
    {
        return objs_[i];
    }

    template <typename T>
    void setAttrOneToAll(const string& name, const T& val)
    {
        for (auto& o : objs_) setProperty<T>(o, name, val);
    }

    template <typename T>
    void setAttrOneToOne(const string& name, const vector<T>& val)
    {
        if (val.size() != objs_.size())
            throw runtime_error(
                "Length of sequence on the right hand side "
                "does not match size of vector. "
                "Expected " +
                to_string(objs_.size()) + ", got " + to_string(val.size()));
        for (size_t i = 0; i < objs_.size(); i++)
            setProperty<T>(objs_[i], name, val[i]);
    }

    vector<py::object> getAttr(const string& name)
    {
        vector<py::object> res(objs_.size());
        for (size_t i = 0; i < objs_.size(); i++) 
            res[i] = getProperty(objs_[i], name);
        return res;
    }

private:
    std::string path_;
    size_t n_;
    const std::string dtype_;
    std::vector<ObjId> objs_;
};

#endif /* end of include guard: VEC_H */
