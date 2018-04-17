#include "SymbolTable.h"

#include <exception>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

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
    if (entry != entries_.end()
        && entry->second->kind() != SymbolTableEntryKind::BLOCK
        && entry->second->kind() != SymbolTableEntryKind::TEMPVAR) {
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
    if (entry->kind() == SymbolTableEntryKind::BLOCK) {
        entry->setName(std::string("block") + std::to_string(forCount_++));
    }

    entries_[entry->name()] = entry;
    entry->setParent(this);
}

void SymbolTable::removeEntry(const SymbolTableEntry::key_type& name)
{
    auto entry = entries_.find(name);

    if (entry != entries_.end()) {
        entries_.erase(entry);
    }
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
    std::string::size_type sizeColLen = 4 + 1;
    std::string::size_type offsetColLen = 6 + 1;

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
    s << "┯";
    std::string sizeHeader = std::to_string(size_);
    s << sizeHeader;
    for (std::string::size_type i = 0; i < sizeColLen - sizeHeader.size(); ++i) s << "━";
    s << "┯";
    for (std::string::size_type i = 0; i < offsetColLen; ++i) s << "━";
    s << "┑" << std::endl;

    // header
    s << pad
      << "│" << std::setw(nameColLen) << "name"
      << "│" << std::setw(kindColLen) << "kind"
      << "│" << std::setw(typeColLen) << "type"
      << "│" << std::setw(superColLen) << "super"
      << "│" << std::setw(sizeColLen) << "size"
      << "│" << std::setw(offsetColLen) << "offset"
      << "│" << std::endl;
    s << pad << "├";
    for (std::string::size_type i = 0; i < nameColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < kindColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < typeColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < superColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < sizeColLen; ++i) s << "─";
    s << "┼";
    for (std::string::size_type i = 0; i < offsetColLen; ++i) s << "─";
    s << "┤" << std::endl;

    // entries
    for (const auto& it : entries_) {
        std::shared_ptr<SymbolTableEntry> entry = it.second;
        s << pad << "│";

        // name
        s << std::setw(nameColLen) << entry->name();
        s << "│";

        // kind
        s << std::setw(kindColLen);
        switch (entry->kind()) {
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
            case SymbolTableEntryKind::TEMPVAR:
                s << "tempvar";
                break;
            case SymbolTableEntryKind::LITERAL:
                s << "literal";
                break;
            case SymbolTableEntryKind::THIS:
                s << "this";
                break;
        }
        s << "│";

        // type
        s << std::setw(typeColLen);
        if (entry->type()) {
            s << entry->type()->str();
        } else {
            s << "";
        }
        s << "│";

        // super
        s << std::setw(superColLen);
        if (entry->supers().empty()) {
            s << " ";
        } else {
            std::ostringstream oss;
            for (auto sit = entry->supers().begin(); sit != entry->supers().end(); ++sit) {
                oss << (*sit)->name();
                if (sit + 1 != entry->supers().end()) {
                    oss << ", ";
                }
            }
            s << oss.str();
        }
        s << "│";

        // size
        s << std::setw(sizeColLen) << entry->size() << "│";

        // offset
        s << std::setw(offsetColLen) << entry->offset() << "│";

        s << std::endl;
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
    s << "┷";
    for (std::string::size_type i = 0; i < sizeColLen; ++i) s << "━";
    s << "┷";
    for (std::string::size_type i = 0; i < offsetColLen; ++i) s << "━";
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

int SymbolTable::size()
{
    return size_;
}

void SymbolTable::setSize(const int& size)
{
    size_ = size;
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

int SymbolTableEntry::size()
{
    return size_;
}

int SymbolTableEntry::offset()
{
    return offset_;
}

void SymbolTableEntry::setSize(const int& size)
{
    size_ = size;
}

void SymbolTableEntry::setOffset(const int& offset)
{
    offset_ = offset;
}

const std::vector<std::shared_ptr<SymbolTableEntry>>& SymbolTableEntry::parameters() const
{
    return parameters_;
}

void SymbolTableEntry::addParameter(const std::shared_ptr<SymbolTableEntry>& parameter)
{
    parameters_.emplace_back(parameter);
}

}}
