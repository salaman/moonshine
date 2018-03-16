#pragma once

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include <ostream>

namespace moonshine { namespace semantic {

enum class SymbolTableEntryKind
{
    FUNCTION,
    CLASS,
    VARIABLE,
    PARAMETER
};

class SymbolTable;

class SymbolTableEntry
{
public:
    typedef std::string key_type;

    SymbolTable* parentTable() const;
    key_type name() const;
    SymbolTableEntryKind kind() const;
    std::string type() const;
    SymbolTable* link() const;
    void setName(const key_type& name);
    void setKind(const SymbolTableEntryKind& kind);
    void setType(const std::string& type);
    void setLink(const std::shared_ptr<SymbolTable>& link);
private:
    key_type name_;
    SymbolTableEntryKind kind_;
    std::string type_; // TODO: not a string?
    std::shared_ptr<SymbolTable> link_;
    SymbolTable* parent_ = nullptr;

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
    SymbolTableEntry* operator[](const SymbolTableEntry::key_type& name);
    void addEntry(const std::shared_ptr<SymbolTableEntry>& entry);

    map_type::iterator begin();
    map_type::iterator end();

    void print(std::ostream& s) const;
private:
    std::unordered_map<SymbolTableEntry::key_type, std::shared_ptr<SymbolTableEntry>> entries_;
    SymbolTableEntry* parent_ = nullptr;
};

}}
