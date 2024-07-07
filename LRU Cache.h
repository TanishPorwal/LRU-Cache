#pragma once

#include <list>
#include <unordered_map>
#include <optional>
#include <functional>

template<typename Key, typename Value>
class LRU_Cache
{
public:
    using list_type =std::list<Key>;
    using map_type = std::unordered_map<Key, std::pair<Value, typename list_type::iterator>>;
    using map_iterator = typename map_type::iterator;
    using const_map_iterator = typename map_type::const_iterator;
    using create_callback_type = std::function<Value(Key)>;
    using delete_callback_type = std::function<void(const Value &)>;

    LRU_Cache(size_t size, create_callback_type creator = nullptr, delete_callback_type deletor = nullptr)
            : m_list()
            , m_map()
            , m_capacity(size)
            , m_create_callback(creator)
            , m_delete_callback(deletor)
    {}

    ~LRU_Cache()
    {
        for(auto& iter : m_map)
        {
            if(m_delete_callback)
                m_delete_callback(iter.second.first);
        }
    }

    LRU_Cache(std::initializer_list<std::pair<Key, Value>> il)
            : LRU_Cache(static_cast<size_t>(il.size()))
    {
        for(auto& i : il)
            insert_or_assign(i.first, i.second);
    }

    LRU_Cache(const LRU_Cache<Key, Value>&) = delete;
    LRU_Cache<Key, Value> &operator= (const LRU_Cache<Key, Value>&) = delete;

    bool empty() const {return m_map.empty();   }
    size_t size() const {return m_map.size();   }
    size_t capacity() const {return m_capacity; }

    bool insert(const Key& key, const Value& val)
    {
        if(m_map.find(key) == m_map.end())
        {
            make_space();

            m_list.push_front(key);
            m_map[key] = std::pair<Key, typename std::list<Key>::iterator>(val, m_list.begin());

            return true;
        }
        else
            return false;
    }

    void insert_or_assign(const Key& key, const Value& val)
    {
        auto iter = m_map.find(key);
        if(m_map.find(key) != m_map.end())
            assign(iter, val);
        else
            insert(key, val);
    }

    bool contains(const Key& key) const { return m_map.find(key) != m_map.end(); }

    // std::optional is used because the value may or may not be contained in the container
    std::optional<Value> at(const Key& key)
    {
        map_iterator iter = m_map.find(key);
        if(iter != m_map.end())
            return iter->second.first;
        else
            return std::nullopt;
    }

    Value& get(const Key& key)
    {
        auto iter = m_map.find(key);

        if(iter != m_map.end())
        {
            touch(key);
            return iter->second.first;
        }
        else
        {
            insert(key, m_create_callback ? m_create_callback(key) : Value());
            return m_map[key].first;
        }
    }

    // touches key at iter and making it most recently used
    void touch(map_iterator& iter)
    {
        auto listRef = iter->second.second;
        m_list.erase(listRef);
        m_list.push_front(iter->first);
        iter->second.second = m_list.begin();
    }

    bool touch(const Key& key)
    {
        auto iter = m_map.find(key);
        if(iter != m_map.end())
        {
            touch(iter);
            return true;
        }
        return false;
    }

    bool assign(const Key& key, const Value& value)
    {
        auto iter = m_map.find(key);
        if(iter != m_map.end())
        {
            assign(iter, value);
            return true;
        }
        return false;
    }

    void assign(map_iterator& iter, const Value& val)
    {
        if(m_delete_callback)
            m_delete_callback(iter->second.first);
        touch(iter);
        iter->second.first = val;
    }

    bool erase(const Key& key)
    {
        auto iter = m_map.find(key);
        if(iter != m_map.end())
        {
            m_list.erase(iter->second.second);
            map_erase(iter);
            return true;
        }
        return false;
    }

    void erase_oldest()
    {
        auto key = m_list.back();
        m_list.pop_back();

        auto iter = m_map.find(key);
        map_erase(iter);
    }

    void clear()
    {
        auto orig_capacity = m_capacity;

        m_capacity = 0;
        trim();
        m_capacity = orig_capacity;
    }

    void resize(size_t newSize)
    {
        m_capacity = newSize;
        trim();
    }

    void setCreateCallback(create_callback_type callback) { m_create_callback = callback; }
    void setDeleteCallback(delete_callback_type callback) { m_delete_callback = callback; }

    // Iterators
    map_iterator begin() { return m_map.begin(); }
    map_iterator end() { return m_map.end(); }
    map_iterator rbegin() { return m_map.rbegin(); }
    map_iterator rend() { return m_map.rbend(); }
    const_map_iterator begin() const { return m_map.begin(); }
    const_map_iterator end() const { return m_map.end(); }
    const_map_iterator cbegin() const { return m_map.cbegin(); }
    const_map_iterator cend() const { return m_map.cend(); }
    const_map_iterator crbegin() const { return m_map.crbegin(); }
    const_map_iterator crend() const { return m_map.crend(); }

private:
    void map_erase(map_iterator pos)
    {
        if(m_delete_callback)
            m_delete_callback(pos->second.first);
        m_map.erase(pos);
    }

    bool trim()
    {
        if (size() <= m_capacity)
            return false;
        do{
            erase_oldest();
        } while(m_list.size() > m_capacity);
        return true;
    }

    void make_space()
    {
        if(size() == m_capacity)
            erase_oldest();
    }

private:
    list_type m_list;
    map_type m_map;
    size_t m_capacity;
    create_callback_type m_create_callback;
    delete_callback_type  m_delete_callback;
};