/***
 *    Description:  moose.vec class.
 *
 *        Created:  2020-03-30

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  GPLv3
 */

#ifndef MOOSE_VEC_H
#define MOOSE_VEC_H

class MooseVec {

public:
    MooseVec(const string& path, unsigned int n, const string& dtype);

    MooseVec(const ObjId& oid);

    const string dtype() const;

    const size_t size() const;

    const string path() const;

    unsigned int len();

    const ObjId& getItemRef(const size_t i) const;
    ObjId getItem(const size_t i) const;

    void setAttrOneToAll(const string& name, const py::object& val);

    void setAttrOneToOne(const string& name, const py::sequence& val);

    vector<py::object> getAttr(const string& name);

    const vector<ObjId>& objs() const;

    ObjId connectToSingle(const string& srcfield, const ObjId& tgt, const string& tgtfield, const string& msgtype);

    ObjId connectToVec(const string& srcfield, const MooseVec& tgt, const string& tgtfield, const string& msgtype);

private:
    std::string path_;
    std::vector<ObjId> objs_;
};

#endif /* end of include guard: MOOSE_VEC_H */
