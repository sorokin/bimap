#ifndef BIMAP_H
#define BIMAP_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>

#define container_of( ptr, type, member ) \
   ( \
      { \
         const decltype( ((type *)0)->member ) *__mptr = (ptr); \
         (type *)( (char *)__mptr - offsetof( type, member ) ); \
      } \
   )

template <typename Data>
struct half_node
{
    half_node()
        : parent()
        , left()
        , right()
    {}

    half_node(Data const& data)
        : data(data)
        , parent()
        , left()
        , right()
    {}

    half_node* find(Data const& key)
    {
        if (this == nullptr)
            return nullptr;

        if (key < data)
            return left->find(key);
        else if (key == data)
            return this;
        else
            return right->find(key);
    }

    void insert(half_node<Data>& target)
    {
        if (target.data < data)
            insert_to_left(target);
        else
        {
            assert(target.data > data);
            insert_to_right(target);
        }
    }

    void insert_to_left(half_node<Data>& target)
    {
        if (left)
            left->insert(target);
        else
        {
            left = &target;
            target.parent = this;
        }
    }

    void insert_to_right(half_node<Data>& target)
    {
        if (right)
            right->insert(target);
        else
        {
            right = &target;
            target.parent = this;
        }
    }

    void erase()
    {
        bool left_child = is_left_child();

        if (left == nullptr)
        {
            parent->child(left_child) = right;
            if (right)
                right->parent = parent;
        }
        else if (right == nullptr)
        {
            parent->child(left_child) = left;
            if (left)
                left->parent = parent;
        }
        else
        {
            half_node* next = right->min();
            bool next_is_left_child = next->is_left_child();

            if (next == right)
            {
                assert(!next_is_left_child);

                std::swap(left, next->left);
                if (left)
                    left->parent = this;
                if (next->left)
                    next->left->parent = next;

                right = next->right;
                if (right)
                    right->parent = this;

                half_node* this_parent = parent;

                next->parent = this_parent;
                this_parent->child(left_child) = next;

                parent = next;
                next->right = this;
            }
            else
            {
                std::swap(left, next->left);
                if (left)
                    left->parent = this;
                if (next->left)
                    next->left->parent = next;

                std::swap(right, next->right);
                if (right)
                    right->parent = this;
                if (next->right)
                    next->right->parent = next;

                std::swap(parent, next->parent);
                parent->child(next_is_left_child) = this;
                next->parent->child(left_child) = next;
            }

            erase();
        }
    }

    bool is_left_child() const
    {
        return parent->left == this;
    }

    half_node*& child(bool left_child)
    {
        return left_child ? left : right;
    }

    half_node* min()
    {
        half_node* t = this;
        while (t->left)
            t = t->left;
        return t;
    }

    half_node* max()
    {
        half_node*t = this;
        while (t->right)
            t = t->right;
        return t;
    }

    half_node* next()
    {
        if (right)
            return right->min();

        half_node* t = this;
        while (!t->is_left_child())
            t = t->parent;

        return t->parent;
    }

    half_node* prev()
    {
        if (left)
            return left->max();

        half_node* t = this;
        while (t->is_left_child())
            t = t->parent;

        return t->parent;
    }

    void delete_this()
    {
        if (this == nullptr)
            return;

        left->delete_this();
        right->delete_this();
        delete this;
    }

    void check_invariant()
    {
        assert(left != this);
        assert(right != this);

        if (left)
        {
            assert(left->parent == this);
            left->check_invariant();
        }
        if (right)
        {
            assert(right->parent == this);
            right->check_invariant();
        }
    }

    size_t size()
    {
        if (this == nullptr)
            return 0;

        return 1 + left->size() + right->size();
    }

    Data data;
    half_node* parent;
    half_node* left;
    half_node* right;
};

template <typename Left, typename Right>
struct bimap
{
    typedef Left left_t;
    typedef Right right_t;

    struct left_iterator
    {
        left_iterator(half_node<Left>* node)
            : node(node)
        {}

        left_iterator& operator++()
        {
            node = node->next();
            return *this;
        }

        left_iterator operator++(int)
        {
            left_iterator old = *this;
            ++*this;
            return old;
        }

        left_iterator& operator--()
        {
            node = node->prev();
            return *this;
        }

        left_iterator operator--(int)
        {
            left_iterator old = *this;
            --*this;
            return old;
        }

        friend bool operator!=(left_iterator a, left_iterator b)
        {
            return a.node != b.node;
        }

        half_node<Left>* node;
    };

    struct right_iterator
    {
        right_iterator(half_node<Right>* node)
            : node(node)
        {}

        right_iterator& operator++()
        {
            node = node->next();
            return *this;
        }

        right_iterator operator++(int)
        {
            right_iterator old = *this;
            ++*this;
            return old;
        }

        right_iterator& operator--()
        {
            node = node->prev();
            return *this;
        }

        right_iterator operator--(int)
        {
            right_iterator old = *this;
            --*this;
            return old;
        }

        friend bool operator!=(right_iterator a, right_iterator b)
        {
            return a.node != b.node;
        }

        half_node<Right>* node;
    };

    struct node
    {
        node()
        {}

        node(left_t left, right_t right)
            : left_half(left)
            , right_half(right)
        {}

        half_node<Left> left_half;
        half_node<Right> right_half;
    };

    bimap()
        : fake_root()
    {}

    bimap(bimap const&) = delete;
    bimap& operator=(bimap const&) = delete;

    ~bimap()
    {
        fake_root.left_half.left->delete_this();
    }

    left_iterator find_left(left_t left)
    {
        auto* hnode = fake_root.left_half.left->find(left);
        return hnode ? left_iterator(hnode) : end_left();
    }

    right_iterator find_right(right_t right)
    {
        auto* hnode = fake_root.right_half.left->find(right);
        return hnode ? right_iterator(hnode) : end_right();
    }

    std::pair<left_iterator, right_iterator> insert(left_t left, right_t right)
    {
        node* new_node = new node(left, right);
        fake_root.left_half.insert_to_left(new_node->left_half);
        fake_root.right_half.insert_to_left(new_node->right_half);
        auto t = std::make_pair(left_iterator(&new_node->left_half), right_iterator(&new_node->right_half));

        check_invariant();

        return t;
    }

    left_iterator erase_left(left_iterator it)
    {
        left_iterator t = it;
        ++t;

        erase_node(node_by_left_half(it.node));

        check_invariant();
        return t;
    }

    right_iterator erase_right(right_iterator it)
    {
        right_iterator t = it;
        ++t;

        erase_node(node_by_right_half(it.node));

        check_invariant();
        return t;
    }

    left_iterator end_left() const
    {
        return left_iterator(&fake_root.left_half);
    }

    right_iterator end_right() const
    {
        return right_iterator(&fake_root.right_half);
    }

    size_t size() const
    {
        size_t n1 = fake_root.left_half.left->size();
        size_t n2 = fake_root.right_half.left->size();
        assert(n1 == n2);
        return n1;
    }

private:
    void check_invariant()
    {
        fake_root.left_half.check_invariant();
        fake_root.right_half.check_invariant();
        size();
    }

    void erase_node(node* node)
    {
        assert(node != &fake_root);
        node->left_half.erase();
        fake_root.left_half.check_invariant();
        node->right_half.erase();
        fake_root.right_half.check_invariant();
        delete node;
    }

    node* node_by_left_half(half_node<Left>* hnode)
    {
        return container_of(hnode, node, left_half);
    }

    node* node_by_right_half(half_node<Right>* hnode)
    {
        return container_of(hnode, node, right_half);
    }

private:
    mutable node fake_root;
};

#endif // BIMAP_H
