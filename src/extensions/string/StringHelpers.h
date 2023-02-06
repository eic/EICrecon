// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <locale>

namespace eicrecon::str
{
    inline bool EndsWith(const std::string &str, const char *suffix, unsigned suffixLen)
    {
        return str.size() >= suffixLen && 0 == str.compare(str.size()-suffixLen, suffixLen, suffix, suffixLen);
    }

    inline bool EndsWith(const std::string& str, const char* suffix)
    {
        return EndsWith(str, suffix, static_cast<unsigned int>(std::string::traits_type::length(suffix)));
    }

    inline bool StartsWith(const std::string &str, const char *prefix, unsigned prefixLen)
    {
        return str.size() >= prefixLen && 0 == str.compare(0, prefixLen, prefix, prefixLen);
    }

    inline bool StartsWith(const std::string &str, const char *prefix)
    {
        return StartsWith(str, prefix, static_cast<unsigned int>(std::string::traits_type::length(prefix)));
    }

    inline bool StartsWith(const std::string &str, const std::string &prefix)
    {
        return StartsWith(str, prefix.c_str());
    }

    // TrimThis from start (in place)
    inline void LeftTrimThis(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // TrimThis from end (in place)
    inline void RightTrimThis(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // TrimThis from both ends (in place)
    inline void TrimThis(std::string &s)
    {
        LeftTrimThis(s);
        RightTrimThis(s);
    }

    // TrimThis from start (copying)
    inline std::string LeftTrimCopy(std::string s)
    {
        LeftTrimThis(s);
        return s;
    }

    // TrimThis from end (copying)
    inline std::string RightTrimCopy(std::string s)
    {
        RightTrimThis(s);
        return s;
    }

    // TrimThis from both ends (copying)
    inline std::string TrimCopy(std::string s)
    {
        TrimThis(s);
        return s;
    }

    inline std::vector<std::string> Split(const std::string& content, const std::string& delimeter)
    {
        std::vector<std::string> result;
        auto prev_pos = content.begin();
        auto next_pos = std::search(prev_pos, content.end(),
                                    delimeter.begin(), delimeter.end());
        while (next_pos != content.end())
        {
            result.emplace_back(prev_pos, next_pos);
            prev_pos = next_pos + delimeter.size();
            next_pos = std::search(prev_pos, content.end(),
                                   delimeter.begin(), delimeter.end());
        }

        if (prev_pos != content.end())
        {
            result.emplace_back(prev_pos, content.end());
        }
        return result;
    }

    /**
     * Check the character is one of " \n\t\v\r\f"
     *
     * @param ch character to check
     * @return true if character is one of " \n\t\v\r\f"
     */
    constexpr bool IsBlank(char ch){
        return ((ch)==' ' || (ch)=='\n' || (ch)=='\t' || (ch)=='\v' || (ch)=='\r' || (ch)=='\f');
    }


    /** Splits string to lexical values.
    *
    * LexicalSplit treats:
    * 1) "quoted values" as one value,
    * 2) '#' not in the beginning of the file are treated as comments to the end of the line
    * 3) skips all white space characters. All specification is in doc/ccdb_file_format.pdf
    *
    * @remark
    * Handling inconsistencies and errors while readout parse time:
    *  -  No ending quote. If no ending " is found, string value will be taken
    *     until the end of line.
    *  -  Comment inside a string. Comment symbol inside the line is ignored.
    *     So if you have a record in the file "info #4" it will be read just
    *     as "info #4" string
    *  -  Sticking string. In case of there is no spaces between symbols and
    *     a quote, all will be merged as one string. I.e.:
    *     John" Smith" will be parsed as one value: "John Smith"
    *     John" "Smith will be parsed as one value: "John Smith"
    *     but be careful(!) not to forget to do a spaces between columns
    *     5.14"Smith" will be parsed as one value "5.14Smith" that probably would
    *     lead to errors if these were two different columns
    *  -  If data contains string fields they are taken into "..." characters. All "
    *     inside string should be saved by \" symbol. All words and symbols
    *     inside "..." will be interpreted as string entity.
    *
    */
    inline std::vector<std::string> LexicalSplit(const std::string& source )
    {
        std::vector<std::string> tokens;

        tokens.reserve(160);
        bool stringIsStarted = false; //Indicates that we meet '"' and looking for second one
        std::string readValue;

        //iterate through string
        for(size_t i=0; i<source.length(); i++)
        {
            if(IsBlank(source[i]) && !stringIsStarted)
            {
                //we have a space! Is it a space that happens after value?
                if(readValue.length()>0)
                {
                    tokens.push_back(readValue);
                    readValue="";
                }
            }
            else
            {
                //it is not a blank character!
                if(source[i]=='\\' && stringIsStarted && i<(source.length()-1) && source[i+1]=='"')
                {
                    //ok! we found a \" inside a string! Not a problem! At all!

                    i++; //skip this \ symbol
                    readValue+=source[i]; //it is just one more symbol in value
                }
                else if(source[i]=='#' && !stringIsStarted) //lets check if it is a comment symbol that is not inside a string...
                {
                    //it is a comment started...
                    //lets save what we collected for now if we collected
                    if(readValue.length()>0)
                    {
                        tokens.push_back(readValue);
                        readValue="";
                    }

                    //and put there the rest of the lint(all comment) if there is something to put
                    if(i<(source.length()-1))
                    {
                        tokens.push_back(source.substr(i));

                        //after that a gentlemen should exit
                        break;
                    }
                }
                else if(source[i]=='"')
                {

                    //it is a beginnig or ending  of a string
                    //just set appropriate flag and continue
                    stringIsStarted = !stringIsStarted;
                }
                else
                {
                    //it is just one more symbol in file
                    readValue+=source[i];
                }
            }

            //last we have is to check that
            //it is not the end of the lint
            if(i==(source.length()-1) && readValue.length()>0)
            {
                tokens.push_back(readValue);
                readValue="";
            }
        }

        return tokens;
    }

    /**
     * Replaces all occurrences of 'from' in 'str' to 'to'
     * @param str - input string
     * @param from - substring to search
     * @param to - value to replace
     */
    inline void ReplaceAllInPlace(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    /**
     * Replaces all occurrences of 'from' in copy of 'str' to 'to', returns changed copy of srt
     * @param str - input string
     * @param from - substring to search
     * @param to - value to replace
     * @return resulting string
     */
    inline std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
        ReplaceAllInPlace(str, from, to);
        return str;
    }
}

