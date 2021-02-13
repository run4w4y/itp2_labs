#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <locale>

// trim string from the left side
std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char c) -> bool {
        return !std::isspace(c);
    }));
    return s;
}

// trim string from the right side
std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](char c) -> bool {
        return !std::isspace(c);
    }).base(), s.end());
    return s;
}

// trim string from both sides
std::string trim_string(std::string s) {
    return ltrim(rtrim(s));
}

// using it to split and join strings
template<char... delimiters>
class PartOfText : public std::string {
    friend std::istream& operator>>(std::istream& input_stream, PartOfText<delimiters...>& part) {
        char const delimiter_chars[] = { delimiters... };
        std::getline(input_stream, part, delimiter_chars[0]);
        return input_stream;
    }

    friend std::ostream& operator<<(std::ostream& output_stream, const PartOfText<delimiters...>& part) {
        char const delimiter_chars[] = { delimiters... };
        for (const auto& c : part) 
            output_stream.put(c);
        for (const auto& c : delimiter_chars)
            output_stream.put(c);
        return output_stream;
    }

    public:
    PartOfText<delimiters...> () {
        std::string::operator=(std::string());
    }

    PartOfText<delimiters...> (const std::string& s) {
        std::string::operator=(s);
    }
};

// returns sentences found in the istream
std::vector<std::string> get_sentences(std::istream& input_stream) {
    using Sentence = PartOfText<'.'>;

    std::vector<std::string> res(
        (std::istream_iterator<Sentence>(input_stream)), 
        std::istream_iterator<Sentence>()
    );
    std::transform(res.begin(), res.end(), res.begin(), 
        [](std::string s) -> std::string {
            return trim_string(s); 
        }
    );
    return res;
}

// find words and mark what was found
template<typename Iterator>
std::vector<std::string> find_word(Iterator begin, Iterator end, const std::string& word) {
    using Word = PartOfText<' '>;

    std::vector<std::string> sentences(std::distance(begin, end));
    std::transform(begin, end, sentences.begin(), 
        [&](std::string s) -> std::string {
            std::istringstream iss(s);
            std::vector<std::string> words = std::vector<std::string>(
                (std::istream_iterator<Word>(iss)), 
                std::istream_iterator<Word>()
            );
            auto found = std::find(words.begin(), words.end(), word);
            if (found != words.end()) {
                std::transform(found->begin(), found->end(), found->begin(), ::toupper);
                std::ostringstream oss;
                std::copy(words.begin(), words.end(), std::ostream_iterator<Word>(oss));
                std::string res = oss.str();
                res.pop_back();
                return res;
            } else
                return std::string();
        }
    );

    sentences.erase(std::remove(sentences.begin(), sentences.end(), ""), sentences.end());
    return sentences;
}

int main() {
    std::ifstream in_stream("in.txt");
    std::ofstream out_stream("out.txt");
    
    std::string s;
    getline(in_stream, s);
    std::istringstream iss(s);
    std::vector<std::string> sentences = get_sentences(iss);
    
    size_t n;
    in_stream >> n;

    for (int i = 0; i < n; ++i) {
        std::string word;
        in_stream >> word;
        std::vector<std::string> res = find_word(sentences.begin(), sentences.end(), word);
        out_stream << res.size() << std::endl;
        std::copy(res.begin(), res.end(), std::ostream_iterator<PartOfText<'.', '\n'>>(out_stream));
    }
}