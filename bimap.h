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

    friend half_node* find(half_node* hnode, Data const& key)
    {
        if (hnode == nullptr)
            return nullptr;

        if (key < hnode->data)
            return find(hnode->left, key);
        else if (key == hnode->data)
            return hnode;
        else
            return find(hnode->right, key);
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

            std::swap(left, next->left);
            if (left)
                left->parent = this;
            if (next->left)
                next->left->parent = next;

            if (next == right)
            {
                assert(!next_is_left_child);

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

    friend void delete_tree(half_node* hnode)
    {
        if (hnode == nullptr)
            return;

        delete_tree(hnode->left);
        delete_tree(hnode->right);
        delete hnode;
    }

    void check_invariant()
    {
#ifndef NDEBUG
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
#endif
    }

    friend size_t tree_size(half_node const* hnode)
    {
        if (hnode == nullptr)
            return 0;

        return 1 + tree_size(hnode->left) + tree_size(hnode->right);
    }

    Data data;
    half_node* parent;
    half_node* left;
    half_node* right;
};

template <typename Left, typename Right>
struct node
{
    node()
    {}

    node(Left left, Right right)
        : left_half(left)
        , right_half(right)
    {}

    half_node<Left> left_half;
    half_node<Right> right_half;
};

template <typename Left, typename Right, bool IsLeft>
struct node_traits;

template <typename Left, typename Right>
struct node_traits<Left, Right, true>
{
    typedef node<Left, Right> node_type;
    typedef Left value_type;
    typedef half_node<Left> half_node_type;

    static half_node_type* get_half_node(node_type* n)
    {
        return &n->left_half;
    }

    static node_type* get_node(half_node_type* hnode)
    {
        return container_of(hnode, node_type, left_half);
    }
};

template <typename Left, typename Right>
struct node_traits<Left, Right, false>
{
    typedef node<Left, Right> node_type;
    typedef Right value_type;
    typedef half_node<Right> half_node_type;

    static half_node_type* get_half_node(node_type* n)
    {
        return &n->right_half;
    }

    static node_type* get_node(half_node_type* hnode)
    {
        return container_of(hnode, node_type, right_half);
    }
};

template <typename Left, typename Right>
struct bimap
{
    typedef Left left_t;
    typedef Right right_t;
    typedef node<left_t, right_t> node_type;

    template <bool IsLeft>
    struct iterator
    {
        static constexpr bool is_left = IsLeft;
        typedef typename node_traits<left_t, right_t, is_left>::value_type value_type;
        typedef typename node_traits<left_t, right_t, is_left>::half_node_type half_node_type;

        iterator(node_type* n)
            : hnode(node_traits<left_t, right_t, is_left>::get_half_node(n))
        {}

        iterator(half_node_type* hnode)
            : hnode(hnode)
        {}

        value_type const& operator*() const
        {
            return hnode->data;
        }

        value_type const* operator->() const
        {
            return &hnode->data;
        }

        iterator& operator++()
        {
            hnode = hnode->next();
            return *this;
        }

        iterator operator++(int)
        {
            iterator old = *this;
            ++*this;
            return old;
        }

        iterator& operator--()
        {
            hnode = hnode->prev();
            return *this;
        }

        iterator operator--(int)
        {
            iterator old = *this;
            --*this;
            return old;
        }

        iterator<!is_left> flip() const
        {
            return iterator<!is_left>(get_node());
        }

        friend bool operator==(iterator a, iterator b)
        {
            return a.hnode != b.hnode;
        }

        friend bool operator!=(iterator a, iterator b)
        {
            return a.hnode != b.hnode;
        }

        node_type* get_node() const
        {
            return node_traits<left_t, right_t, is_left>::get_node(hnode);
        }

        half_node_type* hnode;
    };

    typedef iterator<true> left_iterator;
    typedef iterator<false> right_iterator;

    bimap()
        : fake_root()
    {}

    bimap(bimap const&) = delete;
    bimap& operator=(bimap const&) = delete;

    ~bimap()
    {
        delete_tree(fake_root.left_half.left);
    }

    left_iterator find_left(left_t left)
    {
        auto* hnode = find(fake_root.left_half.left, left);
        return hnode ? left_iterator(hnode) : end_left();
    }

    right_iterator find_right(right_t right)
    {
        auto* hnode = find(fake_root.right_half.left, right);
        return hnode ? right_iterator(hnode) : end_right();
    }

    std::pair<left_iterator, right_iterator> insert(left_t left, right_t right)
    {
        node_type* new_node = new node_type(left, right);
        fake_root.left_half.insert_to_left(new_node->left_half);
        fake_root.right_half.insert_to_left(new_node->right_half);
        auto t = std::make_pair(left_iterator(new_node), right_iterator(new_node));

        check_invariant();

        return t;
    }

    template <bool IsLeft>
    iterator<IsLeft> erase(iterator<IsLeft> it)
    {
        iterator<IsLeft> t = it;
        ++t;

        erase_node(it.get_node());

        check_invariant();
        return t;
    }

    left_iterator begin_left() const
    {
        return left_iterator(fake_root.left_half.min());
    }

    right_iterator begin_right() const
    {
        return right_iterator(fake_root.right_half.min());
    }

    left_iterator end_left() const
    {
        return left_iterator(&fake_root);
    }

    right_iterator end_right() const
    {
        return right_iterator(&fake_root);
    }

    bool empty() const
    {
        return fake_root.left_half.left != nullptr;
    }

    size_t size() const
    {
        size_t n = tree_size(fake_root.left_half.left);
        assert(n == tree_size(fake_root.right_half.left));
        return n;
    }

private:
    void check_invariant()
    {
#ifndef NDEBUG
        fake_root.left_half.check_invariant();
        fake_root.right_half.check_invariant();
        size();
#endif
    }

    void erase_node(node_type* node)
    {
        assert(node != &fake_root);
        node->left_half.erase();
        fake_root.left_half.check_invariant();
        node->right_half.erase();
        fake_root.right_half.check_invariant();
        delete node;
    }

private:
    mutable node_type fake_root;
};

#endif // BIMAP_H
