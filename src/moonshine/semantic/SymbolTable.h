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
    typedef SymbolTable* weak_table_type;

    weak_table_type parentTable() const;
    key_type name() const;
    SymbolTableEntryKind kind() const;
    weak_type_type type() const;
    table_type link() const;
    const std::vector<std::shared_ptr<SymbolTableEntry>>& supers() const;
    bool hasReturn() const;
    void setName(const key_type& name);
    void setKind(const SymbolTableEntryKind& kind);
    void setType(type_type type);
    void setLink(const table_type& link);
    void setParent(SymbolTable* parent);
    void setHasReturn(const bool& hasReturn);
    void addSuper(const std::shared_ptr<SymbolTableEntry>& super);
    void removeSuper(const std::string& super);
private:
    key_type name_;
    SymbolTableEntryKind kind_;
    type_type type_;
    table_type link_;
    weak_table_type parent_ = nullptr;
    bool hasReturn_ = false;
    std::vector<std::shared_ptr<SymbolTableEntry>> supers_;
};

class SymbolTable
{
public:
    typedef std::shared_ptr<SymbolTableEntry> entry_type;
    typedef SymbolTableEntry* weak_entry_type;
    typedef std::unordered_map<SymbolTableEntry::key_type, entry_type> map_type;

    weak_entry_type parentEntry() const;
    void setParentEntry(weak_entry_type parent);
    std::shared_ptr<SymbolTableEntry> operator[](const SymbolTableEntry::key_type& name);
    std::shared_ptr<SymbolTableEntry> get(const SymbolTableEntry::key_type& name);
    void addEntry(const entry_type& entry);

    map_type::iterator begin();
    map_type::iterator end();

    void print(std::ostream& s, std::string pad) const;
private:
    // TODO: unordered_multimap?
    map_type entries_;
    weak_entry_type parent_ = nullptr;
};

}}
