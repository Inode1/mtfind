#ifndef MTFIND_SUFFIX_ARRAY_HPP
#define MTFIND_SUFFIX_ARRAY_HPP

#include <cmath>
#include <vector>
#include <string>
#include <future>
#include <algorithm>

namespace mtfind
{
    using SPText = std::shared_ptr<std::vector<std::string>>;

    struct index_t
    {
        size_t Line;
        size_t Position;
        bool operator==(const index_t& rhs) const
        {
            return Line == rhs.Line && Position == rhs.Position;
        }
    };

    class MTFind
    {
    public:
        MTFind(SPText text, std::string pattern): 
                                                  m_text(text),
                                                  m_pattern(pattern)
        {}

        void Search(bool parallel = true)
        {
            if (!m_text || m_pattern.empty() || m_alreadyMatch)
            {
                return;
            }

            if (parallel)
            {
                ParallelMatch();
            }
            else
            {
                Match();
            }
            m_alreadyMatch = true;
        }

        std::ostream& Print(std::ostream& os) const
        {
            if (!m_text)
            {
                return os;
            }
            const auto& text = *m_text;
            os << m_subStrings.size() << std::endl;
            for (const auto& substring: m_subStrings)
            {
                os << substring.Line + 1 << " " << substring.Position + 1 << " " 
                   << text[substring.Line].substr(substring.Position, m_pattern.size()) << std::endl;
            }
            return os;
        }

        bool operator==(const MTFind& rhs) const
        {
            if (rhs.m_subStrings.size() != m_subStrings.size())
            {
                return false;
            }

            auto diff = std::mismatch(rhs.m_subStrings.cbegin(), rhs.m_subStrings.cend(), m_subStrings.cbegin());
            return diff.first == rhs.m_subStrings.cend() && diff.second == m_subStrings.cend();
        }

        bool operator!=(const MTFind& rhs) const
        {
            return !(*this == rhs);
        }

    private:
        SPText               m_text;
        std::string          m_pattern;
        std::vector<index_t> m_subStrings;
        bool                 m_alreadyMatch = false;

        constexpr static size_t cThresholdSize = 1'000'000;
        constexpr static size_t cThreshold     = 1;

        void ParallelMatch()
        {
            size_t total = 0;
            for (const auto& str: *m_text)
            {
                total += str.size();
            }
            size_t round = std::round(total / cThresholdSize);
            return round > cThreshold ? Match(round) : Match();
        }

        void Match()
        {
            size_t i = 0;
            for (const auto& str: *m_text)
            {
                MatchLine(str, i++);
            }
        }
        // parallel version
        void Match(size_t round)
        {
            std::vector<size_t> sizes;
            sizes.reserve(m_text->size());
            size_t total = 0;

            for (const auto& str: *m_text)
            {
                total += str.size();
                sizes.push_back(total);
            }

            size_t process = std::thread::hardware_concurrency();
            if (process == 0)
            {
                process = 2;
            }

            if (process > round)
            {
                process = round;
            }

            size_t block = total / process;
            std::vector<std::vector<index_t>> indexes(process);
            std::vector<std::future<void>> tasks(process);
            for (size_t i = 0; i < process; ++i)
            {
                size_t lower = i == 0 ? 0 : block * i - m_pattern.size();
                size_t upper = i == process - 1 ? total : block * (i + 1) + m_pattern.size();
                
                tasks[i] = std::async(std::launch::async, &MTFind::MatchParallelLine, this, std::cref(*m_text), std::cref(sizes), lower, upper, std::ref(indexes[i]));
            }

            index_t tmp;
            for (size_t i = 0; i < process; ++i)
            {
                tasks[i].get();
                std::copy_if(indexes[i].begin(), indexes[i].end(), std::back_inserter(m_subStrings),
                                    [&](const index_t& value)
                                    {
                                        if (m_subStrings.empty())
                                        {
                                            tmp = value;
                                            return true;
                                        }

                                        if (value.Line == tmp.Line &&
                                            tmp.Position + m_pattern.size() > value.Position)
                                        {
                                            return false;
                                        }

                                        tmp = value;
                                        return true;
                                    });
            }
        }

        void MatchLine(const std::string& s, size_t line)
        {
            if (s.size() < m_pattern.size())
            {
                return;
            }

            for (size_t i = 0, j = 0, backup = 0; i < s.size(); ++i)
            {
                if(m_pattern[j] == s[i] || m_pattern[j] == '?')
                {
                    ++j;
                    if (j == m_pattern.size())
                    {
                        m_subStrings.push_back({line, backup});
                        backup = i + 1;
                        j = 0;
                    }
                }
                else
                {
                    j = 0;
                    i = backup;
                    ++backup;
                }
            }
        }

        void MatchParallelLine(const std::vector<std::string>& text,
                               const std::vector<size_t>& sizes,
                               size_t start, size_t end,
                               std::vector<index_t>& result) 
        {
            
            size_t line = 0;
            size_t lower = 0;
            bool newLine = false;
            for (size_t i = 0; i < sizes.size(); ++i)
            {
                if (start < sizes[i])
                {
                    break;
                }
                lower = sizes[i];
                ++line;
            }
            size_t slice = sizes[line];

            for (size_t i = start, j = 0, backup = start; i < end; ++i)
            {
                if (i == slice)
                {
                    newLine = true;
                    j = 0;
                    ++line;
                    backup = i;
                    lower = slice;
                    slice = sizes[line];
                }
                char c = text[line][i - lower];
                if(m_pattern[j] == c || m_pattern[j] == '?')
                {
                    ++j;
                    if (j == m_pattern.size())
                    {
                        result.push_back({line, backup - lower});
                        j = 0;
                        if (newLine || start == 0)
                        {
                            backup = i + 1;
                        }
                        else
                        {
                            i = backup;
                            ++backup;
                        }
                    }
                }
                else
                {
                    j = 0;
                    i = backup;
                    ++backup;
                }
            }
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const MTFind& find)
    {
        return find.Print(os);
    }
}

#endif