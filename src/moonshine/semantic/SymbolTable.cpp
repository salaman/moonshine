#include "SymbolTable.h"

#include <exception>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace moonshine { namespace semantic {

SymbolTableEntry* SymbolTable::parentEntry() const
{
    return parent_;
}

SymbolTableEntry::key_type SymbolTableEntry::name() const
{
    return name_;
}

SymbolTableEntryKind SymbolTableEntry::kind() const
{
    return kind_;
}

SymbolTableEntry::weak_type_type SymbolTableEntry::type() const
{
    return type_.get();
}

SymbolTableEntry::table_type SymbolTableEntry::link() const
{
    return link_;
}

void SymbolTableEntry::setName(const SymbolTableEntry::key_type& name)
{
    name_ = name;
}

void SymbolTableEntry::setKind(const SymbolTableEntryKind& kind)
{
    kind_ = kind;
}

void SymbolTableEntry::setType(SymbolTableEntry::type_type type)
{
    type_ = std::move(type);
}

void SymbolTableEntry::setLink(const SymbolTableEntry::table_type& link)
{
    if (!link) {
        throw std::invalid_argument("SymbolTableEntry::setLink: link cannot be nullptr");
    }

    link_ = link;
    link_->setParentEntry(this);
}

SymbolTable::entry_type SymbolTable::operator[](const SymbolTableEntry::key_type& name)
{
    auto entry = entries_.find(name);

    // we found the entry in this table
    if (entry != entries_.end()) {
        return entry->second;
    }

    if (parent_ && parent_->kind() == SymbolTableEntryKind::CLASS) {
        for (const auto& super : parent_->supers()) {
            if (super->link()) {
                if (auto a = (*super->link())[name]) {
                    return a;
                }
            }
        }
    }

    // check if the parent table has the entry we want
    if (parent_ && parent_->parentTable()) {
        return (*parent_->parentTable())[name];
    }

    // entry does not exist
    return nullptr;
}

void SymbolTable::addEntry(const SymbolTable::entry_type& entry)
{
    entries_[entry->name()] = entry;
    entry->setParent(this);
}

void SymbolTable::print(std::ostream& s, std::string pad) const
{
    std::string tableName = parent_
                            ? (parent_->kind() == SymbolTableEntryKind::BLOCK ? "block" : parent_->name())
                            : "GLOBAL";

    std::string::size_type nameColLen = std::max(4ul, tableName.size()) + 1;
    std::string::size_type kindColLen = 9 + 1;
    std::string::size_type typeColLen = 4 + 1;
    std::string::size_type superColLen = 5 + 1;

    for (const auto& it : entries_) {
        nameColLen = std::max(nameColLen, it.second->name().size() + 1);

        if (it.second->type()) {
            typeColLen = std::max(typeColLen, it.second->type()->str().size() + 1);
        }
    }

    for (const auto& it : entries_) {
        std::string::size_type len = 0;
        for (auto sit = it.second->supers().begin(); sit != it.second->supers().end(); ++sit) {
            len += (*sit)->name().size();
            if (sit + 1 != it.second->supers().end()) {
                len += 2;
            }
        }
        superColLen = std::max(superColLen, len + 1);
    }

    s << std::left;

    // name row
    s << pad << "┍" << tableName;
    for (std::string::size_type i = 0; i < nameColLen - tableName.size(); ++i) s << "━";
    s << "┯";
    for (std::string::size_type i = 0; i < kindColLen; ++i) s << "━";
    s << "┯";
    for (std::string::size_type i = 0; i < typeColLen; ++i) s << "━";
    s << "┯";
    for (std::string::size_type i = 0; i < superColLen; ++i) s << "━";
    s << "┑" << std::endl;

    // header
    s << pad << "│" << std::setw(nameColLen) << "name" << "│"<< std::setw(kindColLen) << "kind" << "│" << std::setw(typeColLen) << "type" << "│" << std::setw(superColLen) << "super" << "│" << std::endl;
    s << pad << "├";
    for (std::string::size_type i = 0; i < nameColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < kindColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < typeColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < superColLen; ++i) s << "─";
    s << "┤" << std::endl;

    // entries
    for (const auto& it : entries_) {
        s << pad << "│";
        s << std::setw(nameColLen) << it.second->name();
        s << "│";

        s << std::setw(kindColLen);
        switch (it.second->kind()) {
            case SymbolTableEntryKind::FUNCTION:
                s << "function";
                break;
            case SymbolTableEntryKind::CLASS:
                s << "class";
                break;
            case SymbolTableEntryKind::PARAMETER:
                s << "parameter";
                break;
            case SymbolTableEntryKind::VARIABLE:
                s << "variable";
                break;
            case SymbolTableEntryKind::BLOCK:
                s << "block";
                break;
        }
        s << "│";

        s << std::setw(typeColLen);
        if (it.second->type()) {
            s << it.second->type()->str();
        } else {
            s << "";
        }
        s << "│";

        s << std::setw(superColLen);
        if (it.second->supers().empty()) {
            s << " ";
        } else {
            std::ostringstream oss;
            for (auto sit = it.second->supers().begin(); sit != it.second->supers().end(); ++sit) {
                oss << (*sit)->name();
                if (sit + 1 != it.second->supers().end()) {
                    oss << ", ";
                }
            }
            s << oss.str();
        }

        s << "│" << std::endl;
    }

    // footer
    s << pad << "┕";
    for (std::string::size_type i = 0; i < nameColLen; ++i) s << "━";
    s << "┷";
    for (std::string::size_type i = 0; i < kindColLen; ++i) s << "━";
    s << "┷";
    for (std::string::size_type i = 0; i < typeColLen; ++i) s << "━";
    s << "┷";
    for (std::string::size_type i = 0; i < superColLen; ++i) s << "━";
    s << "┙" << std::endl;

    pad += "│ ";

    // recursively print sub-tables
    for (const auto& it : entries_) {
        if (it.second->link()) {
            it.second->link()->print(s, pad);
        }
    }
}

SymbolTable::map_type::iterator SymbolTable::begin()
{
    return entries_.begin();
}

SymbolTable::map_type::iterator SymbolTable::end()
{
    return entries_.end();
}

void SymbolTable::setParentEntry(SymbolTable::weak_entry_type parent)
{
    parent_ = parent;
}

std::shared_ptr<SymbolTableEntry> SymbolTable::get(const SymbolTableEntry::key_type& name)
{
    auto entry = entries_.find(name);

    // we found the entry in this table
    if (entry != entries_.end()) {
        return entry->second;
    }

    // else, return nullptr (don't look up in the hierarchy)
    return nullptr;
}

SymbolTable* SymbolTableEntry::parentTable() const
{
    return parent_;
}

void SymbolTableEntry::setParent(SymbolTableEntry::weak_table_type parent)
{
    parent_ = parent;
}

bool SymbolTableEntry::hasReturn() const
{
    return hasReturn_;
}

void SymbolTableEntry::setHasReturn(const bool& hasReturn)
{
    hasReturn_ = hasReturn;
}

void SymbolTableEntry::addSuper(const std::shared_ptr<SymbolTableEntry>& super)
{
    supers_.push_back(super);
}

const std::vector<std::shared_ptr<SymbolTableEntry>>& SymbolTableEntry::supers() const
{
    return supers_;
}

void SymbolTableEntry::removeSuper(const std::string& super)
{
    auto it = std::find_if(supers_.begin(), supers_.end(), [&super](const std::shared_ptr<SymbolTableEntry>& entry) {
        return entry->name() == super;
    });

    if (it != supers_.end()) {
        supers_.erase(it);
    }
}

}}
