#pragma once

#include "moonshine/semantic/Type.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>

namespace moonshine { namespace semantic {

enum class SymbolTableEntryKind
{
    FUNCTION,
    CLASS,
    VARIABLE,
    PARAMETER,
    BLOCK
};

class SymbolTable;

class SymbolTableEntry
{
public:
    typedef std::string key_type;
    typedef std::unique_ptr<SymbolType> type_type;
    typedef SymbolType* weak_type_type;
    typedef std::shared_ptr<SymbolTable> table_type;

    SymbolTable* parentTable() const;
    key_type name() const;
    SymbolTableEntryKind kind() const;
    weak_type_type type() const;
    table_type link() const;
    bool hasReturn() const;
    void setName(const key_type& name);
    void setKind(const SymbolTableEntryKind& kind);
    void setType(type_type type);
    void setLink(const std::shared_ptr<SymbolTable>& link);
    void setParent(SymbolTable* parent);
    void setHasReturn(const bool& hasReturn);
private:
    key_type name_;
    SymbolTableEntryKind kind_;
    type_type type_;
    table_type link_;
    SymbolTable* parent_ = nullptr;
    bool hasReturn_ = false;

    friend class CompareSymbolTableEntry;
};

struct CompareSymbolTableEntry
{
    bool operator()(const SymbolTableEntry& lhs, const SymbolTableEntry& rhs) const
    {
        return lhs.name_ < rhs.name_;
    }

    bool operator()(const std::unique_ptr<SymbolTableEntry>& lhs, const std::unique_ptr<SymbolTableEntry>& rhs) const
    {
        return lhs->name_ < rhs->name_;
    }
};

class SymbolTable
{
public:
    typedef std::unordered_map<SymbolTableEntry::key_type, std::shared_ptr<SymbolTableEntry>> map_type;

    void setParentEntry(SymbolTableEntry* parent);

    SymbolTableEntry* parentEntry() const;
    std::shared_ptr<SymbolTableEntry> operator[](const SymbolTableEntry::key_type& name);
    void addEntry(const std::shared_ptr<SymbolTableEntry>& entry);

    map_type::iterator begin();
    map_type::iterator end();

    void print(std::ostream& s, const int& level, std::string pad) const;
private:
    // TODO: unordered_multimap?
    std::unordered_map<SymbolTableEntry::key_type, std::shared_ptr<SymbolTableEntry>> entries_;
    SymbolTableEntry* parent_ = nullptr;
};

}}
