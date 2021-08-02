// Author: Jakub Mazurkiewicz
// C++ standard: 17

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <utility>
#include <vector>

enum class CardRank : unsigned char {
    TWO = '2',
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE,
};

std::ostream& operator<<(std::ostream& stream, const CardRank& rank) {
    switch(rank) {
    case CardRank::TEN:
        stream << "10";
        break;

    case CardRank::JACK:
        stream << 'J';
        break;

    case CardRank::QUEEN:
        stream << 'Q';
        break;

    case CardRank::KING:
        stream << 'K';
        break;

    case CardRank::ACE:
        stream << 'A';
        break;

    default:
        stream << static_cast<unsigned char>(rank);
    }

    return stream;
}

enum CardSuite : unsigned char {
    SPADE = 'S',
    HEART = 'H',
    DIAMOND = 'D',
    CLUB = 'C',
};

std::ostream& operator<<(std::ostream& stream, const CardSuite& suite) {
    return stream << static_cast<unsigned char>(suite);
}

struct Card {
    CardSuite suite;
    CardRank rank;

    friend std::ostream& operator<<(std::ostream& stream, const Card& card) {
        return stream << '(' << card.rank << '|' << card.suite << ')';
    }

    bool operator==(const Card& other) const {
        return rank == other.rank;
    }

    bool operator<(const Card& other) const {
        return rank < other.rank;
    }
};

class Deck {
private:
    struct Node {
        Card card;
        Node* next;

        Node()
            : Node(Card{}) { }

        explicit Node(const Card& card, Node* next = nullptr)
            : card{card}
            , next{next} { }
    };

    Node beforeHeadNode;
    std::size_t deckSize;

public:
    Deck()
        : beforeHeadNode{Card{}, nullptr}
        , deckSize{0} { }

    ~Deck() {
        while(!empty()) {
            popFront();
        }
    }

    Deck(Deck&& other)
        : beforeHeadNode{std::exchange(other.beforeHeadNode, Node{})}
        , deckSize{std::exchange(other.deckSize, 0)} { }

    Deck& operator=(Deck&& other) {
        if(&other != this) {
            beforeHeadNode = std::exchange(other.beforeHeadNode, Node{});
            deckSize = std::exchange(other.deckSize, 0);
        }

        return *this;
    }

    std::size_t size() const {
        return deckSize;
    }

    bool empty() const {
        return size() == 0;
    }

    class ConstIterator {
    protected:
        const Node* node;

    public:
        using value_type = Card;
        using reference = const value_type&;
        using pointer = const value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        ConstIterator()
            : node{nullptr} { }
        explicit ConstIterator(const Node* node)
            : node{node} { }

        const Node* getNode() const {
            return node;
        }

        ConstIterator& operator++() {
            node = node->next;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator copy = *this;
            ++*this;
            return copy;
        }

        reference operator*() const {
            return node->card;
        }

        pointer operator->() const {
            return &(node->card);
        }

        bool operator==(const ConstIterator& other) const {
            return node == other.node;
        }

        bool operator!=(const ConstIterator& other) const {
            return !(*this == other);
        }
    };

    class Iterator : public ConstIterator {
    public:
        using value_type = Card;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        Iterator()
            : ConstIterator() { }
        explicit Iterator(Node* node)
            : ConstIterator(node) { }

        Node* getNode() const {
            return const_cast<Node*>(node);
        }

        reference operator*() const {
            return const_cast<reference>(node->card);
        }

        pointer operator->() const {
            return &const_cast<reference>(node->card);
        }
    };

    Iterator beforeBegin() {
        return Iterator{&beforeHeadNode};
    }

    Iterator begin() {
        return std::next(beforeBegin());
    }

    Iterator end() {
        return Iterator{nullptr};
    }

    ConstIterator beforeBegin() const {
        return ConstIterator{&beforeHeadNode};
    }

    ConstIterator begin() const {
        return std::next(beforeBegin());
    }

    ConstIterator end() const {
        return ConstIterator{nullptr};
    }

    void pushFront(const Card& card) {
        insertAfter(beforeBegin(), card);
    }

    void popFront() {
        eraseAfter(beforeBegin());
    }

    void insertAfter(Iterator it, const Card& card) {
        attachAfter(it.getNode(), new Node{card});
        ++deckSize;
    }

    void eraseAfter(Iterator it) {
        delete extractAfter(it.getNode());
        --deckSize;
    }

private:
    void attachAfter(Node* node, Node* nodeToAttach) {
        assert(node != nullptr);

        nodeToAttach->next = node->next;
        node->next = nodeToAttach;
    }

    [[nodiscard]] Node* extractAfter(Node* node) {
        assert(node != nullptr);
        assert(node->next != nullptr);

        Node* const nodeToExtract = node->next;
        node->next = nodeToExtract->next;
        return nodeToExtract;
    }

public:
    template<typename Generator>
    void shuffle(Generator& generator) {
        using DifferenceType = Iterator::difference_type;
        using Distribution = std::uniform_int_distribution<DifferenceType>;
        using ParamType = Distribution::param_type;

        Distribution distr;
        DifferenceType minOffset = 0;

        for(Iterator it = begin(); it != end(); ++it) {
            const ParamType param{minOffset++,
                                  static_cast<DifferenceType>(size() - 1)};
            const DifferenceType offset = distr(generator, param);
            std::iter_swap(it, std::next(begin(), offset));
        }
    }

    void stableSelectionSort() {
        Iterator inserter = beforeBegin();

        while(std::next(inserter) != end()) {
            Iterator beforeMin = findBeforeMin(inserter);

            Node* const extracted = extractAfter(beforeMin.getNode());
            attachAfter(inserter.getNode(), extracted);
            ++inserter;
        }
    }

private:
    Iterator findBeforeMin(Iterator beforeStart) {
        Iterator beforeMin = beforeStart;
        while(++beforeStart != end() && std::next(beforeStart) != end()) {
            if(*std::next(beforeStart) < *std::next(beforeMin)) {
                beforeMin = beforeStart;
            }
        }

        assert(beforeMin != end());
        assert(std::next(beforeMin) != end());

        return beforeMin;
    }

public:
    friend std::ostream& operator<<(std::ostream& stream, const Deck& deck) {
        stream << '[';

        if(!deck.empty()) {
            ConstIterator it = deck.begin();
            stream << *it;

            while(++it != deck.end()) {
                stream << ", " << *it;
            }
        }

        return stream << ']';
    }

    static Deck generateStandardDeck() {
        Deck deck;

        for(CardRank rank = CardRank::ACE; rank >= CardRank::TWO;) {
            for(CardSuite suite : {CardSuite::SPADE, CardSuite::HEART,
                                   CardSuite::CLUB, CardSuite::DIAMOND}) {
                deck.pushFront(Card{suite, rank});
            }
            rank = static_cast<CardRank>(static_cast<unsigned char>(rank) - 1);
        }

        return deck;
    }
};

bool isStandardDeckStablySorted(const Deck& deck,
                                 const std::vector<Card>& shuffledCollection) {
    for(auto it = deck.begin(); it != deck.end();) {
        auto jt = shuffledCollection.begin();
        for(int i = 0; i < 4; ++i) {
            jt = std::find(jt, shuffledCollection.end(), *it);

            if(jt == shuffledCollection.end()) {
                return false;
            }

            ++it;
        }
    }

    return true;
}

void genericTest(std::size_t testCount) {
    Deck deck = Deck::generateStandardDeck();
    assert(deck.size() == 52);

    std::mt19937 generator{std::random_device{}()};

    for(std::size_t testId = 1; testId <= testCount; ++testId) {
        deck.shuffle(generator);
        std::vector shuffledCollection(deck.begin(), deck.end());

        deck.stableSelectionSort();

        if(!std::is_sorted(deck.begin(), deck.end())) {
            std::cout << "Test " << testId << ":\t";
            std::cout << "Deck was not sorted.\n";
        }

        if(!isStandardDeckStablySorted(deck, shuffledCollection)) {
            std::cout << "Test " << testId << ":\t";
            std::cout << "Deck was not stably sorted.\n";
        }
    }
}

void interactiveTest() {
    std::cout << "Scheme: (rank|suite)\n";

    Deck deck = Deck::generateStandardDeck();
    assert(deck.size() == 52);
    std::cout << "Input deck:\n";
    std::cout << deck << '\n';

    std::mt19937 generator{std::random_device{}()};
    deck.shuffle(generator);
    std::cout << "Shuffled deck:\n";
    std::cout << deck << '\n';
    std::vector shuffledCollection(deck.begin(), deck.end());

    deck.stableSelectionSort();
    std::cout << "Stably sorted deck:\n";
    std::cout << deck << '\n';

    std::cout << std::boolalpha;
    std::cout << "Is deck sorted: ";
    std::cout << std::is_sorted(deck.begin(), deck.end());
    std::cout << '\n';

    std::cout << "Is deck stably sorted: ";
    std::cout << isStandardDeckStablySorted(deck, shuffledCollection) << '\n';
}

int main(int argc, char* argv[]) {
    if(argc >= 2) {
        try {
            genericTest(std::stoull(argv[1]));
        } catch(std::exception& e) {
            std::cerr << "Fatal error: " << e.what() << '\n';
        }
    } else {
        interactiveTest();
    }
}