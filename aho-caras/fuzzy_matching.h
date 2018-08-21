#include <algorithm>
#include <cstring>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <unordered_map>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>


template <class Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    Iterator begin() const { return begin_; }
    Iterator end() const { return end_; }

private:
    Iterator begin_, end_;
};

namespace traverses {

    template <class Vertex, class Graph, class Visitor>
    void BreadthFirstSearch(Vertex origin_vertex, const Graph& graph,
                            Visitor visitor) {
        std::queue<Vertex> order;
        order.push(origin_vertex);
        std::unordered_set<Vertex> visited;
        visited.insert(origin_vertex);
        visitor.DiscoverVertex(origin_vertex);
        while (!order.empty()) {
            Vertex next = order.front();
            visitor.ExamineVertex(next);
            order.pop();
            for (auto edge : OutgoingEdges(graph, next)) {
                visitor.ExamineEdge(edge);
                auto target = GetTarget(graph, edge);
                if (visited.find(target) == visited.end()) {
                    visitor.DiscoverVertex(target);
                    visited.insert(target);
                    order.push(target);
                }
            }
        }
    };


    template <class Vertex, class Edge>
    class BfsVisitor {
    public:
        virtual void DiscoverVertex(Vertex /*vertex*/) {}
        virtual void ExamineEdge(const Edge& /*edge*/) {}
        virtual void ExamineVertex(Vertex /*vertex*/) {}
        virtual ~BfsVisitor() = default;
    };

}  // namespace traverses

namespace aho_corasick {

    struct AutomatonNode {
        AutomatonNode() : suffix_link(nullptr), terminal_link(nullptr) {}

        // Stores ids of strings which are ended at this node.
        std::vector<size_t> terminated_string_ids;
        // Stores tree structure of nodes.
        std::map<char, AutomatonNode> trie_transitions;
        // Stores cached transitions of the automaton, contains
        // only pointers to the elements of trie_transitions.
        std::map<char, AutomatonNode*> automaton_transitions_cache;
        AutomatonNode* suffix_link;
        AutomatonNode* terminal_link;
    };

// Returns a corresponding trie transition 'nullptr' otherwise.
    AutomatonNode* GetTrieTransition(AutomatonNode* node, char character) {
        auto transition = node->trie_transitions.find(character);
        if (transition == node->trie_transitions.end()) {
            return nullptr;
        } else {
            return &(*transition).second;
        }
    }

// Returns an automaton transition, updates 'node->automaton_transitions_cache'
// if necessary.
// Provides constant amortized runtime.
    AutomatonNode* GetAutomatonTransition(AutomatonNode* node,
                                          const AutomatonNode* root,
                                          char character) {
        if (node->trie_transitions.find(character)
            != node->trie_transitions.end()) {
            return &node->trie_transitions[character];
        }
        if (node->automaton_transitions_cache.find(character)
            != node->automaton_transitions_cache.end()) {
            return node->automaton_transitions_cache[character];
        }
        if (node == root) {
            return node;
        }
        node->automaton_transitions_cache.insert({character,
                             GetAutomatonTransition(node->suffix_link, root, character)});
        return node->automaton_transitions_cache[character];
    }

    namespace internal {

        class AutomatonGraph {
        public:
            struct Edge {
                Edge(AutomatonNode* source, AutomatonNode* target, char character)
                        : source(source), target(target), character(character) {}

                AutomatonNode* source;
                AutomatonNode* target;
                char character;
            };
        };

        std::vector<typename AutomatonGraph::Edge> OutgoingEdges(
                const AutomatonGraph& /*graph*/, AutomatonNode* vertex) {
            std::vector<typename AutomatonGraph::Edge> result;
            for (auto& edge : vertex->trie_transitions) {
                result.push_back(AutomatonGraph::Edge(
                        vertex, GetTrieTransition(vertex, edge.first), edge.first));
            }
            return result;
        }

        AutomatonNode* GetTarget(const AutomatonGraph& /*graph*/,
                                 const AutomatonGraph::Edge& edge) {
            return edge.target;
        }

        class SuffixLinkCalculator
                : public traverses::BfsVisitor<AutomatonNode*, AutomatonGraph::Edge> {
        public:
            explicit SuffixLinkCalculator(AutomatonNode* root) : root_(root) {}

            void ExamineVertex(AutomatonNode* node) override {
                if ((node->suffix_link == nullptr) || (node->suffix_link == node)) {
                    node->suffix_link = root_;
                }
            }

            void ExamineEdge(const AutomatonGraph::Edge& edge) override {
                edge.target->suffix_link =
                        GetAutomatonTransition(edge.source->suffix_link, root_, edge.character);
            }

        private:
            AutomatonNode* root_;
        };

        class TerminalLinkCalculator
                : public traverses::BfsVisitor<AutomatonNode*, AutomatonGraph::Edge> {
        public:
            explicit TerminalLinkCalculator(AutomatonNode* root) : root_(root) {}
            void DiscoverVertex(AutomatonNode* node) override {
                if (node == root_) {
                    node->terminal_link = root_;
                    return;
                }
                AutomatonNode* next = node->suffix_link;
                if (next->terminated_string_ids.size() == 0) {
                    node->terminal_link = next->terminal_link;
                } else {
                    node->terminal_link = next;
                }
            }

        private:
            AutomatonNode* root_;
        };
}  // namespace internal


    class NodeReference {
    public:
        NodeReference() : node_(nullptr), root_(nullptr) {}

        NodeReference(AutomatonNode* node, AutomatonNode* root)
                : node_(node), root_(root) {}

        NodeReference Next(char character) const {
            return NodeReference(GetAutomatonTransition(node_, root_, character),
                                 root_);
        }


        template <class Callback>
        void GenerateMatches(Callback on_match) const {
            if (node_ && IsTerminal()) {
                for (auto string_id : TerminatedStringIds()) {
                    on_match(string_id);
                }
            }
            if (!(node_ == root_)) {
                TerminalLink().GenerateMatches(on_match);
            }
        }

        bool IsTerminal() const { return !node_->terminated_string_ids.empty(); }

        explicit operator bool() const { return node_ != nullptr; }

        bool operator==(NodeReference other) const { return node_ == other.node_; }

    private:
        using TerminatedStringIterator = std::vector<size_t>::const_iterator;
        using TerminatedStringIteratorRange = IteratorRange<TerminatedStringIterator>;

        NodeReference TerminalLink() const {
            return NodeReference(node_->terminal_link, root_);
        }

        TerminatedStringIteratorRange TerminatedStringIds() const {
            return TerminatedStringIteratorRange(node_->terminated_string_ids.begin(),
                                                 node_->terminated_string_ids.end());
        }

        AutomatonNode* node_;
        AutomatonNode* root_;
    };

    class AutomatonBuilder;

    class Automaton {
    public:
        Automaton() = default;

        Automaton(const Automaton&) = delete;

        Automaton& operator=(const Automaton&) = delete;

        NodeReference Root() { return NodeReference(&root_, &root_); }

    private:
        AutomatonNode root_;

        friend class AutomatonBuilder;
    };

    class AutomatonBuilder {
    public:
        void Add(const std::string& string, size_t id) {
            words_.push_back(string);
            ids_.push_back(id);
        }

        std::unique_ptr<Automaton> Build() {
            auto automaton = std::make_unique<Automaton>();
            BuildTrie(words_, ids_, automaton.get());
            BuildSuffixLinks(automaton.get());
            BuildTerminalLinks(automaton.get());
            return automaton;
        }

    private:
        static void BuildTrie(const std::vector<std::string>& words,
                              const std::vector<size_t>& ids, Automaton* automaton) {
            for (size_t i = 0; i < words.size(); ++i) {
                AddString(&automaton->root_, ids[i], words[i]);
            }
        }

        static void AddString(AutomatonNode* root, size_t string_id,
                              const std::string& string) {
            AutomatonNode* current = root;
            for (char ch : string) {
                if (current->trie_transitions.find(ch) ==
                    current->trie_transitions.end()) {
                    current->trie_transitions.insert(
                            std::pair<char, AutomatonNode>(ch, AutomatonNode()));
                }
                current = &(*current->trie_transitions.find(ch)).second;
            }
            current->terminated_string_ids.push_back(string_id);
        }

        static void BuildSuffixLinks(Automaton* automaton) {
            internal::SuffixLinkCalculator visitor(&(automaton->root_));
            traverses::BreadthFirstSearch(&(automaton->root_),
                                          internal::AutomatonGraph(), visitor);
        }

        static void BuildTerminalLinks(Automaton* automaton) {
            internal::TerminalLinkCalculator visitor(&(automaton->root_));
            traverses::BreadthFirstSearch(&(automaton->root_),
                                          internal::AutomatonGraph(), visitor);
        }

        std::vector<std::string> words_;
        std::vector<size_t> ids_;
    };

}  // namespace aho_corasick

// Consecutive delimiters are not grouped together and are deemed
// to delimit empty strings
template <class Predicate>
std::vector<std::string> Split(const std::string& string,
                               Predicate is_delimiter) {
    size_t prev_pos = 0;
    std::string pattern;
    std::vector<std::string> patterns;
    for (size_t pos = 0; pos < string.size(); ++pos) {
        if (is_delimiter(string[pos])) {
            pattern = string.substr(prev_pos, pos - prev_pos);
            patterns.emplace_back(pattern);
            prev_pos = pos + 1;
        }
    }
    pattern = string.substr(prev_pos, string.size() - prev_pos);
    patterns.emplace_back(pattern);
    pattern.clear();
    return patterns;
}

// Wildcard is a character that may be substituted
// for any of all possible characters.
class WildcardMatcher {
public:
    WildcardMatcher() : number_of_words_(0), pattern_length_(0) {}

    WildcardMatcher static BuildFor(const std::string& pattern, char wildcard) {
        WildcardMatcher wild;
        aho_corasick::AutomatonBuilder builder;
        std::vector<std::string> patterns =
                Split(pattern, [](char ch) { return ch == '?'; });
        size_t position = 0;
        for (size_t index = 0; index < patterns.size(); ++index) {
            if (patterns[index].size() != 0) {
                wild.number_of_words_ += 1;
                builder.Add(patterns[index], patterns[index].length() + position);
                position += patterns[index].length();
                ++position;
            } else {
                ++position;
            }
        }
        wild.words_occurrences_by_position_ = std::deque<size_t>();
        wild.aho_corasick_automaton_ = builder.Build();
        wild.state_ = wild.aho_corasick_automaton_->Root();
        wild.pattern_length_ = pattern.size();
        return wild;
    }

    // Resets the matcher. Call allows to abandon all data which was already
    // scanned,
    // a new stream can be scanned afterwards.
    void Reset() {
        state_ = aho_corasick_automaton_->Root();
        words_occurrences_by_position_ = std::deque<size_t>();
    }


    template <class Callback>
    void Scan(char character, Callback on_match) {
        ShiftWordOccurrencesCounters();
        state_ = state_.Next(character);
        position_ = words_occurrences_by_position_.size();
        state_.GenerateMatches(
                [&](size_t string_id) {
                    UpdateWordOccurrencesCounters(string_id);
                });
        if ( words_occurrences_by_position_.size() == pattern_length_ &&
            words_occurrences_by_position_[0] == number_of_words_) {
            on_match();
        }
    }

private:
    void UpdateWordOccurrencesCounters(size_t string_id) {
        if (string_id <= position_) {
            ++words_occurrences_by_position_[position_ - string_id];
        }
    }

    void ShiftWordOccurrencesCounters() {
        if (words_occurrences_by_position_.size() == pattern_length_) {
            words_occurrences_by_position_.pop_front();
        }
        words_occurrences_by_position_.push_back(0);
    }
    // Storing only O(|pattern|) elements allows us
    // to consume only O(|pattern|) memory for matcher.
    std::deque<size_t> words_occurrences_by_position_;
    aho_corasick::NodeReference state_;
    size_t number_of_words_;
    size_t pattern_length_;
    size_t position_;
    std::unique_ptr<aho_corasick::Automaton> aho_corasick_automaton_;
};

std::string ReadString(std::istream& input_stream) {
    std::string result;
    input_stream >> result;
    return result;
}

// Returns positions of the first character of an every match.
std::vector<size_t> FindFuzzyMatches(const std::string& pattern_with_wildcards,
                                     const std::string& text, char wildcard) {
    auto matcher = WildcardMatcher::BuildFor(pattern_with_wildcards, wildcard);
    std::vector<size_t> occurrences;

    for (size_t offset = 0; offset < text.size(); ++offset) {
        matcher.Scan(text[offset], [&occurrences, offset, &pattern_with_wildcards] {
            occurrences.push_back(offset - pattern_with_wildcards.size() + 1);
        });
    }
    return occurrences;
}

void Print(const std::vector<size_t>& sequence) {
    std::cout << sequence.size() << "\n";
    for (auto element : sequence) {
        std::cout << element << " ";
    }
    std::cout << "\n";
}
