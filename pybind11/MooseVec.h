/***
 *    Description:  moose.vec class.
 *
 *        Created:  2020-03-30

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  GPLv3
 */

#ifndef MOOSE_VEC_H
#define MOOSE_VEC_H

class MooseVec 
{

public:
    MooseVec(const string& path, unsigned int n, const string& dtype);

    MooseVec(const Id& id);

    MooseVec(const ObjId& oid);

    const ObjId& obj() const;

    const string dtype() const;

    const size_t size() const;

    const string path() const;

    const string name() const;

    unsigned int len();

    const ObjId& getItemRef(const size_t i) const;

    // Get vector element. Vector element could be `dataIndex` or `fieldIndex`.
    ObjId getItem(const size_t i) const;
    ObjId getDataItem(const size_t i) const;
    ObjId getFieldItem(const size_t i) const;

    void setAttrOneToAll(const string& name, const py::object& val);

    void setAttrOneToOne(const string& name, const py::sequence& val);

    vector<py::object> getAttribute(const string& name);
    py::array_t<double> getAttributeNumpy(const string& name);

    vector<ObjId> objs() const;

    ObjId connectToSingle(const string& srcfield, const ObjId& tgt, const string& tgtfield, const string& msgtype);

    ObjId connectToVec(const string& srcfield, const MooseVec& tgt, const string& tgtfield, const string& msgtype);

    size_t id() const;

    // Iterator interface. Create copy of ObjId 
    void generateIterator();
    const vector<ObjId>& objref() const;

private:
    ObjId oid_;
    std::string path_;

    // Objects 
    // Turns a ObjId to vector. Use dataIndex whenever available such as 
    // >>> a = moose.Pool('a', 100)
    // >>> av = moose.vec(a)
    // will use dataIndex for indexing.
    // For FieldElementInfo, create objects.
    vector<ObjId> objs_;
};

#endif /* end of include guard: MOOSE_VEC_H */
