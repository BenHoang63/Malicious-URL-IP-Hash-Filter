#include <cstddef>    // size_t
#include <functional> // std::hash
#include <ios>
#include <utility>    // std::pair
#include <iostream>

#include "primes.h"

using std::cout;

template <typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
class UnorderedMap {
    public:

    using key_type = Key;
    using mapped_type = T;
    using const_mapped_type = const T;
    using hasher = Hash;
    using key_equal = Pred;
    using value_type = std::pair<const key_type, mapped_type>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    private:

    struct HashNode {
        HashNode *next;
        value_type val;

        HashNode(HashNode *next = nullptr) : next{next} {}
        HashNode(const value_type & val, HashNode * next = nullptr) : next { next }, val { val } { }
        HashNode(value_type && val, HashNode * next = nullptr) : next { next }, val { std::move(val) } { }
    };

    size_type _bucket_count;
    HashNode **_buckets;

    HashNode * _head;
    size_type _size;

    Hash _hash;
    key_equal _equal;

    static size_type _range_hash(size_type hash_code, size_type bucket_count) {
        return hash_code % bucket_count;
    }

    public:

    template <typename pointer_type, typename reference_type, typename _value_type>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = _value_type;
        using difference_type = ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

    private:
        friend class UnorderedMap<Key, T, Hash, key_equal>;
        using HashNode = typename UnorderedMap<Key, T, Hash, key_equal>::HashNode;

        const UnorderedMap * _map;
        HashNode * _ptr;

        explicit basic_iterator(UnorderedMap const * map, HashNode *ptr) noexcept : _map(map), _ptr(ptr) {}

    public:
        basic_iterator() : _map(nullptr), _ptr(nullptr) {};

        basic_iterator(const basic_iterator &) = default;
        basic_iterator(basic_iterator &&) = default;
        ~basic_iterator() = default;
        basic_iterator &operator=(const basic_iterator &) = default;
        basic_iterator &operator=(basic_iterator &&) = default;
        reference operator*() const { return _ptr->val; }
        pointer operator->() const { return &(_ptr->val); }

        // HashNode* increment() { return this->_map->_buckets[this->_map->_bucket(this->_ptr->val.first) + 1]; }
        HashNode* increment() {
            if (this->_map == nullptr || this->_ptr == nullptr) return nullptr;

            // if there's a next node in the same bucket, return it first
            if (this->_ptr->next != nullptr) return this->_ptr->next;

            // find current bucket index and scan subsequent buckets for a non-empty head
            size_type b = this->_map->_bucket(this->_ptr->val.first);
            for (size_type i = b + 1; i < this->_map->_bucket_count; ++i) {
                if (this->_map->_buckets[i] != nullptr) return this->_map->_buckets[i];
            }
            return nullptr;
        }

        basic_iterator &operator++() { this->_ptr = this->increment(); return *this;}
        basic_iterator operator++(int) { 
            basic_iterator copy = *this;
            this->_ptr = this->increment();
            return copy;
        }
        bool operator==(const basic_iterator &other) const noexcept { return this->_ptr == other._ptr; }
        bool operator!=(const basic_iterator &other) const noexcept { return this->_ptr != other._ptr; }
    };

    using iterator = basic_iterator<pointer, reference, value_type>;
    using const_iterator = basic_iterator<const_pointer, const_reference, const value_type>;

    class local_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair<const key_type, mapped_type>;
            using difference_type = ptrdiff_t;
            using pointer = value_type *;
            using reference = value_type &;

        private:
            friend class UnorderedMap<Key, T, Hash, key_equal>;
            using HashNode = typename UnorderedMap<Key, T, Hash, key_equal>::HashNode;

            HashNode * _node;

            explicit local_iterator( HashNode * node ) noexcept : _node(node) {}

        public:
            local_iterator() : _node(nullptr) {}

            local_iterator(const local_iterator &) = default;
            local_iterator(local_iterator &&) = default;
            ~local_iterator() = default;
            local_iterator &operator=(const local_iterator &) = default;
            local_iterator &operator=(local_iterator &&) = default;
            reference operator*() const { return this->_node->val; }
            pointer operator->() const { return &(this->_node->val); }
            local_iterator & operator++() { 
                if (this->_node == nullptr) this->_node = nullptr;
                else this->_node = this->_node->next; 
                return *this; 
            }
            local_iterator operator++(int) { 
                local_iterator copy = *this;
                if (this->_node == nullptr) this->_node = nullptr;
                else this->_node = this->_node->next;
                return copy;
            }

            bool operator==(const local_iterator &other) const noexcept { return this->_node == other._node; }
            bool operator!=(const local_iterator &other) const noexcept { return this->_node != other._node; }
    };

private:

    size_type _bucket(size_t code) const { return _range_hash(code,this->_bucket_count); }
    size_type _bucket(const Key & key) const { return _bucket(_hash(key)); }
    size_type _bucket(const value_type & val) const { return _bucket(_hash(val.first)); }

    HashNode* _find(size_type bucket, const Key & key) { /* TODO */ 

        // get the first node of bucket 
        HashNode* current = this->_buckets[bucket];

        // iterate through until key is reached
        while (current != nullptr) {
            if (this->_equal(current->val.first,key)) return current;
            current = current->next;
        }

        return nullptr;

    }

    HashNode* _find(const Key & key) {

        // get the bucket index
        size_type bucket = this->_bucket(key);

        // iterate through until key is reached
        return this->_find(bucket,key);

    }

    std::pair<HashNode*,HashNode*> _find_previous_and_target(const Key & key) {

        // get the bucket index
        size_type bucket = this->_bucket(key);

        // get the first node of bucket
        HashNode* previous = nullptr;
        HashNode* current = this->_buckets[bucket];

        // iterate through until key is reached
        while (current != nullptr) {
            if (this->_equal(current->val.first,key)) return std::pair<HashNode*,HashNode*>(previous,current);
            previous = current;
            current = current->next;
        }

        return std::pair<HashNode*,HashNode*>(nullptr,nullptr);

    }

    HashNode * _insert_into_bucket(size_type bucket, value_type && value) {

        // cover bucket head
        HashNode* old_head = this->_buckets[bucket];                    
        HashNode* new_head = new HashNode(std::move(value),old_head);   
        this->_buckets[bucket] = new_head;                              

        // cover global head
        if (this->_head == nullptr || this->_bucket(this->_head->val.first) >= this->_bucket(new_head->val.first)) 
            this->_head = new_head;

        ++this->_size;

        return new_head;

    }

    HashNode * _insert_into_bucket(size_type bucket, const value_type & value) {

        // cover bucket head
        HashNode* old_head = this->_buckets[bucket];                    
        HashNode* new_head = new HashNode(value,old_head);   
        this->_buckets[bucket] = new_head;                              

        // cover global head
        if (this->_head == nullptr || this->_bucket(this->_head->val.first) >= this->_bucket(new_head->val.first)) 
            this->_head = new_head;

        ++this->_size;

        return new_head;

    }
    
    void _move_content(UnorderedMap & src, UnorderedMap & dst) {
        
        // move everything to dst
        dst._bucket_count = src._bucket_count;
        dst._head = src._head;
        dst._buckets = src._buckets;
        dst._size = src._size;
        dst._hash = src._hash;
        dst._equal = src._equal;

        // set src to empty state
        src._head = nullptr;
        src._buckets = new HashNode*[src._bucket_count]();
        src._size = 0;

    }

    void _copy_content(const UnorderedMap & other) {

        // copy buckets
        this->_bucket_count = other._bucket_count;
        this->_buckets = new HashNode*[this->_bucket_count];
        for (size_type i = 0; i < _bucket_count; ++i) {

            // create the head
            HashNode* c = other._buckets[i];
            if (c == nullptr) { this->_buckets[i] = nullptr; continue; }
            HashNode* p = new HashNode(c->val,nullptr);
            this->_buckets[i] = p;

            // create the linked list
            while (c->next != nullptr) {

                // increment c
                c = c->next;

                // create new node and update pointers
                HashNode* new_node = new HashNode(c->val,nullptr);
                p->next = new_node;

                // increment p
                p = new_node;

            }
        }

        // set the rest of the variables
        this->_head = nullptr;
        for (size_type i = 0; i < _bucket_count; ++i) 
            if (this->_buckets[i] != nullptr) { this->_head = this->_buckets[i]; break; }
        this->_size = other._size;
        this->_hash = other._hash;
        this->_equal = other._equal;

    }

public:
    explicit UnorderedMap(size_type bucket_count, const Hash & hash = Hash { },
                const key_equal & equal = key_equal { }) : _head(nullptr), _size(0), _hash(hash), _equal(equal) {

                    // make sure _bucket_count is prime
                    _bucket_count = next_greater_prime(bucket_count);

                    // create the array of bucket nodes
                    _buckets = new HashNode*[_bucket_count];
                    for(size_type i = 0; i < _bucket_count; ++i) _buckets[i] = nullptr;

                }

    ~UnorderedMap() {
        this->clear();
        delete _buckets;
    }

    UnorderedMap(const UnorderedMap & other) { 
        this->_copy_content(other); 
    }

    UnorderedMap(UnorderedMap && other) { 
        this->_move_content(other,*this); 
    }

    UnorderedMap & operator=(const UnorderedMap & other) { 
        if (&other == this) return *this;
        this->clear(); 
        delete _buckets;
        this->_copy_content(other); 
        return *this; 
    }

    UnorderedMap & operator=(UnorderedMap && other) { 
        if (&other == this) return *this;
        this->clear();
        delete _buckets;
        this->_move_content(other, *this);
        return *this;
    }

    void clear() noexcept {

        if (!this->empty()) {
            
            // loop thru the buckets
            for (size_type i = 0; i < this->_bucket_count; ++i) {

                // loop thru each bucket
                HashNode* current = this->_buckets[i];
                while (current != nullptr) {
                    HashNode* next = current->next;     // get next
                    delete current;                     // deallocate
                    current = next;                     // set current to next
                }

            }

        }

        // update internals
        for(size_type i = 0; i < _bucket_count; i++) _buckets[i] = nullptr;
        this->_head = nullptr;
        this->_size = 0;

    }

    size_type size() const noexcept { return this->_size; }

    bool empty() const noexcept { return (this->_size == 0 && this->_head == nullptr); }

    size_type bucket_count() const noexcept { return this->_bucket_count; }

    iterator begin() { 
        if (this->_size == 0) return iterator(this,nullptr);
        return iterator(this,this->_head); 
    }
    iterator end() { return iterator(this,nullptr); }

    const_iterator cbegin() const { 
        if (this->_size == 0) return const_iterator(this,nullptr);
        return const_iterator(this,this->_head); 
    };
    const_iterator cend() const { return const_iterator(this,nullptr); }

    local_iterator begin(size_type n) { return local_iterator(this->_buckets[n]); }
    local_iterator end(size_type n) { return local_iterator(nullptr); }

    size_type bucket_size(size_type n) {
        size_type count = 0;
        for (local_iterator it = this->begin(n); it != this->end(n); ++it) ++count;
        return count;
    }

    float load_factor() const { return static_cast<float>(this->_size) / static_cast<float>(this->bucket_count()); }

    size_type bucket(const Key & key) const { return this->_bucket(key); }

    std::pair<iterator, bool> insert(value_type && value) { /* TODO */ 
        size_type bucket = _bucket(value.first);
        
        // check if the value exists
        HashNode * node = _find(bucket, value.first);
        if (node != nullptr) return std::pair<iterator,bool>(iterator(this,node),false);

        // if not, then create the new hash node
        node = this->_insert_into_bucket(bucket,std::move(value));

        return std::pair<iterator,bool>(iterator(this,node),true);
    }

    std::pair<iterator, bool> insert(const value_type & value) { /* TODO */ 
        size_type bucket = _bucket(value.first);
        
        // check if the value exists
        HashNode * node = _find(bucket, value.first);
        // cout << value.first << " | " << value.second << "\n";
        if (node != nullptr) { return std::pair<iterator,bool>(iterator(this,node),false); }

        // if not, then create the new hash node
        node = this->_insert_into_bucket(bucket,value);

        return std::pair<iterator,bool>(iterator(this,node),true);
    }

    iterator find(const Key & key) { 
        HashNode* node = this->_find(key);
        if (node != nullptr) return iterator(this,node);
        return iterator(this,nullptr);
    }

    T& operator[](const Key & key) { 

        // check if the key exists
        HashNode* node = this->_find(key);
        if (node != nullptr) return node->val.second;

        // if not, then insert the value
        std::pair<iterator,bool> insert_node = insert(value_type(key,mapped_type()));
        return insert_node.first->second;

    }

    iterator erase(iterator pos) {

        // check if pos is invalid 
        if (pos == this->end()) return this->end();
        
        // find target & check for nullptr
        HashNode* t = pos._ptr;
        if (t == nullptr) return this->end();

        // get replacement
        iterator replacement = pos;
        ++replacement;

        // get previous & next
        size_type bucket = this->_bucket(t->val.first);
        std::pair<HashNode*,HashNode*> node_pair = this->_find_previous_and_target(pos->first);
        HashNode* p = node_pair.first;
        HashNode* n = t->next;

        // if p is nullptr, then we are removing the bucket head
        if (p == nullptr) this->_buckets[bucket] = n;
        else p->next = n;

        // if target is the global head
        if (this->_head == t) {
            if (n != nullptr) this->_head = n;
            else this->_head = replacement._ptr;
        }

        // delete & update pointers
        delete t;
        t = nullptr;
        --this->_size;

        return replacement;

    }

    size_type erase(const Key & key) {

        // find bucket
        size_type bucket = this->_bucket(key);

        // find target, previous, and next nodes
        std::pair<HashNode*,HashNode*> node_pair = this->_find_previous_and_target(key);
        HashNode* p = node_pair.first;
        HashNode* t = node_pair.second;
        HashNode* n = t != nullptr ? t->next : nullptr;

        // check the nodes
        if (t == nullptr) return 0;

        // delete & update pointers
        delete t;
        t = nullptr;
        --this->_size;

        // check if target is the bucket head or map head
        if (p != nullptr)
            p->next = n;
        else if (this->_head == nullptr)
            this->_head = n;
        else
            this->_buckets[bucket] = n;

        return 1;

    }

    template<typename KK, typename VV>
    friend void print_map(const UnorderedMap<KK, VV> & map, std::ostream & os);
};

template<typename K, typename V>
void print_map(const UnorderedMap<K, V> & map, std::ostream & os = std::cout) {
    using size_type = typename UnorderedMap<K, V>::size_type;
    using HashNode = typename UnorderedMap<K, V>::HashNode;

    for(size_type bucket = 0; bucket < map.bucket_count(); bucket++) {
        os << bucket << ": ";

        HashNode const * node = map._buckets[bucket];

        while(node) {
            os << "(" << node->val.first << ", " << node->val.second << ") ";
            node = node->next;
        }

        os << std::endl;
    }
}
