/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class const_iterator;
   class iterator {
      private:
       friend class map;
       friend class const_iterator;
       typedef typename map::value_type* node_value_ptr;
       node_value_ptr ptr;
       map* owner;
      public:
       iterator() : ptr(nullptr), owner(nullptr) {}

       iterator(node_value_ptr p, map* o) : ptr(p), owner(o) {}

       iterator(const iterator &other) : ptr(other.ptr), owner(other.owner) {}

       /**
    * TODO iter++
        */
       iterator operator++(int) {
         if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
         iterator tmp = *this;
         ptr = owner->successor(ptr);
         return tmp;
       }

       /**
    * TODO ++iter
        */
       iterator &operator++() {
         if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
         ptr = owner->successor(ptr);
         return *this;
       }

       /**
    * TODO iter--
        */
       iterator operator--(int) {
         iterator tmp = *this;
         if (owner == nullptr) throw invalid_iterator();
         if (ptr == nullptr) {
           ptr = owner->rightmost();
           if (ptr == nullptr) throw invalid_iterator();
         } else {
           ptr = owner->predecessor(ptr);
           if (ptr == nullptr) throw invalid_iterator();
         }
         return tmp;
       }

       /**
    * TODO --iter
        */
       iterator &operator--() {
         if (owner == nullptr) throw invalid_iterator();
         if (ptr == nullptr) {
           ptr = owner->rightmost();
           if (ptr == nullptr) throw invalid_iterator();
         } else {
           ptr = owner->predecessor(ptr);
           if (ptr == nullptr) throw invalid_iterator();
         }
         return *this;
       }

       /**
    * a operator to check whether two iterators are same (pointing to the same memory).
        */
       value_type &operator*() const {
         if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
         return *ptr;
       }

       bool operator==(const iterator &rhs) const {
         return owner == rhs.owner && ptr == rhs.ptr;
       }

       bool operator==(const const_iterator &rhs) const {
         return owner == rhs.owner && ptr == rhs.ptr;
       }

       /**
    * some other operator for iterator.
        */
       bool operator!=(const iterator &rhs) const {
         return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
         return !(*this == rhs);
       }

       /**
    * for the support of it->first.
    * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
        */
       value_type *operator->() const noexcept {
         return ptr;
       }
   };
   class const_iterator {
      private:
       friend class map;
       friend class iterator;
       typedef const typename map::value_type* node_value_ptr;
       node_value_ptr ptr;
       const map* owner;
      public:
       const_iterator() : ptr(nullptr), owner(nullptr) {}

       const_iterator(node_value_ptr p, const map* o) : ptr(p), owner(o) {}

       const_iterator(const const_iterator &other) : ptr(other.ptr), owner(other.owner) {}

       const_iterator(const iterator &other) : ptr(other.ptr), owner(other.owner) {}

       const_iterator operator++(int) {
         if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
         const_iterator tmp = *this;
         ptr = owner->successor(ptr);
         return tmp;
       }

       const_iterator &operator++() {
         if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
         ptr = owner->successor(ptr);
         return *this;
       }

       const_iterator operator--(int) {
         const_iterator tmp = *this;
         if (owner == nullptr) throw invalid_iterator();
         if (ptr == nullptr) {
           ptr = owner->rightmost();
           if (ptr == nullptr) throw invalid_iterator();
         } else {
           ptr = owner->predecessor(ptr);
           if (ptr == nullptr) throw invalid_iterator();
         }
         return tmp;
       }

       const_iterator &operator--() {
         if (owner == nullptr) throw invalid_iterator();
         if (ptr == nullptr) {
           ptr = owner->rightmost();
           if (ptr == nullptr) throw invalid_iterator();
         } else {
           ptr = owner->predecessor(ptr);
           if (ptr == nullptr) throw invalid_iterator();
         }
         return *this;
       }

       const value_type &operator*() const {
         if (owner == nullptr || ptr == nullptr) throw invalid_iterator();
         return *ptr;
       }

       bool operator==(const iterator &rhs) const {
         return owner == rhs.owner && ptr == rhs.ptr;
       }

       bool operator==(const const_iterator &rhs) const {
         return owner == rhs.owner && ptr == rhs.ptr;
       }

       bool operator!=(const iterator &rhs) const {
         return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
         return !(*this == rhs);
       }

       const value_type *operator->() const noexcept {
         return ptr;
       }
   };

  private:
   Compare comp;
   size_t size_;

   struct Node {
     char storage[sizeof(value_type)];
     Node *left, *right, *parent;
     int color;  // 0: black, 1: red
     Node() : left(nullptr), right(nullptr), parent(nullptr), color(0) {}
     value_type* data() { return reinterpret_cast<value_type*>(storage); }
     const value_type* data() const { return reinterpret_cast<const value_type*>(storage); }
   };

   Node* root;

   void destroy_node(Node* node) {
     if (node == nullptr) return;
     destroy_node(node->left);
     destroy_node(node->right);
     node->data()->~value_type();
     delete node;
   }

   Node* copy_tree(Node* node, Node* parent) {
     if (node == nullptr) return nullptr;
     Node* new_node = new Node();
     new (new_node->storage) value_type(*node->data());
     new_node->color = node->color;
     new_node->parent = parent;
     new_node->left = copy_tree(node->left, new_node);
     new_node->right = copy_tree(node->right, new_node);
     return new_node;
   }

   Node* find_node(Node* node, const Key& key) const {
     while (node != nullptr) {
       if (!comp(key, node->data()->first) && !comp(node->data()->first, key))
         return node;
       if (comp(key, node->data()->first))
         node = node->left;
       else
         node = node->right;
     }
     return nullptr;
   }

   Node* lower_bound_node(Node* node, const Key& key) const {
     Node* result = nullptr;
     while (node != nullptr) {
       if (!comp(node->data()->first, key)) {
         result = node;
         node = node->left;
       } else {
         node = node->right;
       }
     }
     return result;
   }

   void left_rotate(Node* x) {
     Node* y = x->right;
     x->right = y->left;
     if (y->left != nullptr) y->left->parent = x;
     y->parent = x->parent;
     if (x->parent == nullptr) root = y;
     else if (x == x->parent->left) x->parent->left = y;
     else x->parent->right = y;
     y->left = x;
     x->parent = y;
   }

   void right_rotate(Node* x) {
     Node* y = x->left;
     x->left = y->right;
     if (y->right != nullptr) y->right->parent = x;
     y->parent = x->parent;
     if (x->parent == nullptr) root = y;
     else if (x == x->parent->right) x->parent->right = y;
     else x->parent->left = y;
     y->right = x;
     x->parent = y;
   }

   void insert_fixup(Node* z) {
     while (z->parent != nullptr && z->parent->color == 1) {
       if (z->parent->parent != nullptr && z->parent == z->parent->parent->left) {
         Node* y = z->parent->parent->right;
         if (y != nullptr && y->color == 1) {
           z->parent->color = 0;
           y->color = 0;
           z->parent->parent->color = 1;
           z = z->parent->parent;
         } else {
           if (z == z->parent->right) {
             z = z->parent;
             left_rotate(z);
           }
           z->parent->color = 0;
           if (z->parent->parent != nullptr) {
             z->parent->parent->color = 1;
             right_rotate(z->parent->parent);
           }
         }
       } else if (z->parent->parent != nullptr) {
         Node* y = z->parent->parent->left;
         if (y != nullptr && y->color == 1) {
           z->parent->color = 0;
           y->color = 0;
           z->parent->parent->color = 1;
           z = z->parent->parent;
         } else {
           if (z == z->parent->left) {
             z = z->parent;
             right_rotate(z);
           }
           z->parent->color = 0;
           if (z->parent->parent != nullptr) {
             z->parent->parent->color = 1;
             left_rotate(z->parent->parent);
           }
         }
       } else break;
     }
     root->color = 0;
   }

   void erase_fixup(Node* x, Node* x_parent) {
     while (x != root && (x == nullptr || x->color == 0)) {
       if (x_parent != nullptr && x == x_parent->left) {
         Node* w = x_parent->right;
         if (w != nullptr && w->color == 1) {
           w->color = 0;
           x_parent->color = 1;
           left_rotate(x_parent);
           w = x_parent->right;
         }
         if (w != nullptr && (w->left == nullptr || w->left->color == 0) &&
             (w->right == nullptr || w->right->color == 0)) {
           w->color = 1;
           x = x_parent;
           x_parent = x->parent;
         } else if (w != nullptr) {
           if (w->right == nullptr || w->right->color == 0) {
             if (w->left != nullptr) w->left->color = 0;
             w->color = 1;
             right_rotate(w);
             w = x_parent->right;
           }
           w->color = x_parent->color;
           x_parent->color = 0;
           if (w->right != nullptr) w->right->color = 0;
           left_rotate(x_parent);
           x = root;
           x_parent = nullptr;
         } else break;
       } else if (x_parent != nullptr) {
         Node* w = x_parent->left;
         if (w != nullptr && w->color == 1) {
           w->color = 0;
           x_parent->color = 1;
           right_rotate(x_parent);
           w = x_parent->left;
         }
         if (w != nullptr && (w->right == nullptr || w->right->color == 0) &&
             (w->left == nullptr || w->left->color == 0)) {
           w->color = 1;
           x = x_parent;
           x_parent = x->parent;
         } else if (w != nullptr) {
           if (w->left == nullptr || w->left->color == 0) {
             if (w->right != nullptr) w->right->color = 0;
             w->color = 1;
             left_rotate(w);
             w = x_parent->left;
           }
           w->color = x_parent->color;
           x_parent->color = 0;
           if (w->left != nullptr) w->left->color = 0;
           right_rotate(x_parent);
           x = root;
           x_parent = nullptr;
         } else break;
       } else break;
     }
     if (x != nullptr) x->color = 0;
   }

   void transplant(Node* u, Node* v) {
     if (u->parent == nullptr) root = v;
     else if (u == u->parent->left) u->parent->left = v;
     else u->parent->right = v;
     if (v != nullptr) v->parent = u->parent;
   }

   Node* minimum(Node* node) const {
     if (node == nullptr) return nullptr;
     while (node->left != nullptr) node = node->left;
     return node;
   }

   Node* maximum(Node* node) const {
     if (node == nullptr) return nullptr;
     while (node->right != nullptr) node = node->right;
     return node;
   }

   value_type* successor(value_type* v) const {
     Node* node = node_from_value(v);
     if (node == nullptr) return nullptr;
     if (node->right != nullptr) return minimum(node->right)->data();
     Node* p = node->parent;
     while (p != nullptr && node == p->right) {
       node = p;
       p = p->parent;
     }
     return p != nullptr ? p->data() : nullptr;
   }

   const value_type* successor(const value_type* v) const {
     Node* node = node_from_value(v);
     if (node == nullptr) return nullptr;
     if (node->right != nullptr) return minimum(node->right)->data();
     Node* p = node->parent;
     while (p != nullptr && node == p->right) {
       node = p;
       p = p->parent;
     }
     return p != nullptr ? p->data() : nullptr;
   }

   value_type* predecessor(value_type* v) const {
     Node* node = node_from_value(v);
     if (node == nullptr) return nullptr;
     if (node->left != nullptr) return maximum(node->left)->data();
     Node* p = node->parent;
     while (p != nullptr && node == p->left) {
       node = p;
       p = p->parent;
     }
     return p != nullptr ? p->data() : nullptr;
   }

   const value_type* predecessor(const value_type* v) const {
     Node* node = node_from_value(v);
     if (node == nullptr) return nullptr;
     if (node->left != nullptr) return maximum(node->left)->data();
     Node* p = node->parent;
     while (p != nullptr && node == p->left) {
       node = p;
       p = p->parent;
     }
     return p != nullptr ? p->data() : nullptr;
   }

   value_type* rightmost() {
     if (root == nullptr) return nullptr;
     return maximum(root)->data();
   }

   const value_type* rightmost() const {
     if (root == nullptr) return nullptr;
     return maximum(root)->data();
   }

   Node* node_from_value(value_type* v) const {
     if (root == nullptr || v == nullptr) return nullptr;
     Node* node = root;
     while (node != nullptr) {
       if (node->data() == v) return node;
       if (comp(v->first, node->data()->first)) node = node->left;
       else node = node->right;
     }
     return nullptr;
   }

   Node* node_from_value(const value_type* v) const {
     if (root == nullptr || v == nullptr) return nullptr;
     Node* node = root;
     while (node != nullptr) {
       if (node->data() == v) return node;
       if (comp(v->first, node->data()->first)) node = node->left;
       else node = node->right;
     }
     return nullptr;
   }

  public:
   /**
  * TODO two constructors
    */
   map() : size_(0), root(nullptr) {}

   map(const map &other) : size_(0), root(nullptr) {
     if (other.root != nullptr) {
       root = copy_tree(other.root, nullptr);
       size_ = other.size_;
     }
   }

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
     if (this != &other) {
       clear();
       if (other.root != nullptr) {
         root = copy_tree(other.root, nullptr);
         size_ = other.size_;
       }
     }
     return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() { clear(); }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
     Node* node = find_node(root, key);
     if (node == nullptr) throw index_out_of_bound();
     return node->data()->second;
   }

   const T &at(const Key &key) const {
     Node* node = find_node(root, key);
     if (node == nullptr) throw index_out_of_bound();
     return node->data()->second;
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
     Node* node = find_node(root, key);
     if (node != nullptr) return node->data()->second;
     value_type val(key, T());
     auto pr = insert(val);
     return pr.first.ptr->second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
     return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
     if (root == nullptr) return iterator(nullptr, this);
     return iterator(minimum(root)->data(), this);
   }

   const_iterator cbegin() const {
     if (root == nullptr) return const_iterator(nullptr, this);
     return const_iterator(minimum(root)->data(), this);
   }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() { return iterator(nullptr, this); }

   const_iterator cend() const { return const_iterator(nullptr, this); }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const { return size_ == 0; }

   /**
  * returns the number of elements.
    */
   size_t size() const { return size_; }

   /**
  * clears the contents
    */
   void clear() {
     destroy_node(root);
     root = nullptr;
     size_ = 0;
   }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
     Node* exist = find_node(root, value.first);
     if (exist != nullptr) return pair<iterator, bool>(iterator(exist->data(), this), false);

     Node* z = new Node();
     new (z->storage) value_type(value);
     z->color = 1;
     Node* y = nullptr;
     Node* x = root;
     while (x != nullptr) {
       y = x;
       if (comp(z->data()->first, x->data()->first)) x = x->left;
       else x = x->right;
     }
     z->parent = y;
     if (y == nullptr) root = z;
     else if (comp(z->data()->first, y->data()->first)) y->left = z;
     else y->right = z;
     insert_fixup(z);
     size_++;
     return pair<iterator, bool>(iterator(z->data(), this), true);
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
     if (pos.owner != this || pos.ptr == nullptr) throw invalid_iterator();
     Node* z = node_from_value(pos.ptr);
     if (z == nullptr) throw invalid_iterator();

     Node* y = z;
     Node* x = nullptr;
     Node* x_parent = nullptr;
     int y_orig_color = y->color;
     if (z->left == nullptr) {
       x = z->right;
       x_parent = z->parent;
       transplant(z, z->right);
     } else if (z->right == nullptr) {
       x = z->left;
       x_parent = z->parent;
       transplant(z, z->left);
     } else {
       y = minimum(z->right);
       y_orig_color = y->color;
       x = y->right;
       if (y->parent == z) {
         x_parent = y;
         if (x != nullptr) x->parent = y;
       } else {
         transplant(y, y->right);
         y->right = z->right;
         y->right->parent = y;
         x_parent = y->parent;
       }
       transplant(z, y);
       y->left = z->left;
       y->left->parent = y;
       y->color = z->color;
     }
     z->data()->~value_type();
     delete z;
     size_--;
     if (y_orig_color == 0 && root != nullptr) {
       erase_fixup(x, x_parent);
     }
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const {
     return find_node(root, key) != nullptr ? 1 : 0;
   }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) {
     Node* node = find_node(root, key);
     if (node == nullptr) return end();
     return iterator(node->data(), this);
   }

   const_iterator find(const Key &key) const {
     Node* node = find_node(root, key);
     if (node == nullptr) return cend();
     return const_iterator(node->data(), this);
   }
};

}

#endif
