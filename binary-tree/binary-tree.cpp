// Author: Jakub Mazurkiewicz
// C++ standard: 17

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

template<typename T>
struct tree_node {
    template<typename... Args>
    tree_node(Args&&... args)
        : value(std::forward<Args>(args)...)
        , parent{nullptr}
        , left{nullptr}
        , right{nullptr} { }

    T value;

    tree_node* parent;
    tree_node* left;
    tree_node* right;

    void attach_on_left(tree_node* new_left) {
        left = new_left;
        if(new_left != nullptr) {
            new_left->parent = this;
        }
    }

    void attach_on_right(tree_node* new_right) {
        right = new_right;
        if(new_right != nullptr) {
            new_right->parent = this;
        }
    }

    void replace_child(tree_node* old_child, tree_node* new_child) {
        if(left == old_child) {
            attach_on_left(new_child);
            old_child->parent = nullptr;
        } else if(right == old_child) {
            attach_on_right(new_child);
            old_child->parent = nullptr;
        }
    }
};

template<typename T>
class tree {
public:
    using value_type = T;
    using size_type = std::size_t;

private:
    using node = tree_node<value_type>;
    using node_ptr = node*;

    node_ptr root;
    size_type tree_size;

public:
    tree()
        : root{nullptr}
        , tree_size{0} { }

    tree(const tree&) = delete;
    tree& operator=(const tree&) = delete;

    ~tree() {
        if(!empty()) {
            remove_subtree(root);
        }
    }

private:
    void remove_subtree(node_ptr node) {
        if(node->left != nullptr) {
            remove_subtree(node->left);
        }
        if(node->right != nullptr) {
            remove_subtree(node->right);
        }

        delete node;
    }

public:
    size_type size() const {
        return tree_size;
    }

    bool empty() const {
        return size() == 0;
    }

    bool insert(const value_type& value) {
        return emplace(value);
    }

    template<typename... Args>
    bool emplace(Args&&... args) {
        const node_ptr new_node = new node(std::forward<Args>(args)...);

        if(empty()) {
            root = new_node;
        } else if(!try_insert(root, new_node)) {
            delete new_node;
            return false;
        }

        ++tree_size;
        return true;
    }

private:
    bool try_insert(node_ptr subtree, node_ptr new_node) {
        bool success = false;

        if(new_node->value < subtree->value) {
            if(subtree->left == nullptr) {
                subtree->attach_on_left(new_node);
                success = true;
            } else {
                success = try_insert(subtree->left, new_node);
            }

        } else if(subtree->value < new_node->value) {
            if(subtree->right == nullptr) {
                subtree->attach_on_right(new_node);
                success = true;
            } else {
                success = try_insert(subtree->right, new_node);
            }
        }

        return success;
    }

public:
    bool contains(const value_type& value) const {
        return find_at(root, value) != nullptr;
    }

private:
    node_ptr find_at(node_ptr subtree, const value_type& value) const {
        if(subtree == nullptr) {
            return nullptr;
        }

        if(value < subtree->value) {
            return find_at(subtree->left, value);
        } else if(subtree->value < value) {
            return find_at(subtree->right, value);
        }

        return subtree;
    }

public:
    void erase(const value_type& value) {
        if(const node_ptr node_to_erase = find_at(root, value);
           node_to_erase != nullptr) {
            extract_node(node_to_erase);
            delete node_to_erase;
            --tree_size;
        }
    }

private:
    void extract_node(node_ptr node) {
        if(node->left == nullptr && node->right == nullptr) {
            extract_leaf(node);
        } else if(node->left != nullptr && node->right != nullptr) {
            extract_double_node(node);
        } else {
            extract_single_node(node);
        }
    }

    void extract_leaf(node_ptr leaf_to_erase) {
        if(node_ptr parent = leaf_to_erase->parent; parent != nullptr) {
            parent->replace_child(leaf_to_erase, nullptr);
        } else {
            root = nullptr;
        }
    }

    void extract_single_node(node_ptr node_to_erase) {
        const node_ptr child =
            (node_to_erase->left != nullptr ? node_to_erase->left
                                            : node_to_erase->right);

        if(node_ptr parent = node_to_erase->parent; parent != nullptr) {
            parent->replace_child(node_to_erase, child);
        } else {
            root = child;
            child->parent = nullptr;
        }
    }

    void extract_double_node(node_ptr node_to_erase) {
        const node_ptr min_node = extract_min_node(node_to_erase->right);

        if(node_ptr parent = node_to_erase->parent; parent != nullptr) {
            parent->replace_child(node_to_erase, min_node);
        } else {
            root = min_node;
            min_node->parent = nullptr;
        }

        min_node->attach_on_left(node_to_erase->left);
        min_node->attach_on_right(node_to_erase->right);
    }

    node_ptr extract_min_node(node_ptr subtree) {
        while(subtree->left != nullptr) {
            subtree = subtree->left;
        }

        if(subtree->right != nullptr) {
            extract_single_node(subtree);
        } else {
            extract_leaf(subtree);
        }

        return subtree;
    }
};

void shuffle_test_vector(std::vector<int>& test_vector) {
    static std::mt19937 generator{std::random_device{}()};
    std::shuffle(test_vector.begin(), test_vector.end(), generator);
}

std::vector<int> generate_test_vector(std::size_t size) {
    std::vector<int> test_vector(size);

    std::iota(test_vector.begin(), test_vector.end(), 0);
    shuffle_test_vector(test_vector);

    return test_vector;
}

// First program argument is the size of random generated data (2048 by
// default).
int main(int argc, char* argv[]) {
    std::size_t test_vector_size = 2'048;
    if(argc >= 2) {
        try {
            test_vector_size = std::stoull(argv[1]);
        } catch(std::exception& e) {
            std::cerr << "Invalid program argument: " << e.what() << ".\n";
            std::cerr << "The size of random generated data will be "
                      << test_vector_size << ".\n";
        }
    }

    std::vector<int> test_vector = generate_test_vector(test_vector_size);

    tree<int> test_tree;
    for(int e : test_vector) {
        test_tree.insert(e);
    }

    assert(test_vector.size() == test_tree.size());
    std::cout << "Tree was successfully fille with values from 0 to "
              << test_vector_size << '.' << std::endl;

    shuffle_test_vector(test_vector);
    for(int e : test_vector) {
        test_tree.erase(e);
    }

    assert(test_tree.empty() == true);
    std::cout << "Tree was successfully emptied.\n"
}
