// Idempotent package naming library

#ifndef POAC_CORE_NAMING_HPP
#define POAC_CORE_NAMING_HPP

#include <string>
#include <regex>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "exception.hpp"
#include "../io/file.hpp"


namespace poac::core::naming {
    // -- poac --
    // Cache package name
    //   * opencv-3.4.2
    //   * boost-config-1.66.0
    // Current package name
    //   * opencv
    //   * boost-config

    namespace err {
        const std::string source(const std::string& src)
        {
            return "poac.yml error\nWhat source is " + src + "?";
        }
    }


    // username/repository -> repository
    std::string basename(const std::string& name)
    {
        if (name.find('/') != std::string::npos)
            { return std::string(name, name.find('/')+1); }
        else
            { return name; }
    }
    // boost/bind -> boost-bind
    std::string slash_to_hyphen(std::string name)
    {
        std::replace(name.begin(), name.end(), '/', '-');
        return name;
    }


    // 1. opencv/opencv, 3.4.2 -> opencv-3.4.2
    // 2. curl/curl, 7_61_0 -> curl-7.61.0
    // 3. matken11235/logger_slack_backend, v0.1.0 -> logger_slack_backend-0.1.0
    // 4. curl/curl, curl-7_61_0 -> curl-7_61_0

    // GitHub
    std::string to_cache_github(const std::string& name, const std::string& tag)
    {
        std::regex pattern("^(v?)?(?:(\\d+)(\\.|_))?(?:(\\d+)(\\.|_))?(\\*|\\d+)$");
        std::smatch matches;

        // 1. boost -> boost
        // 2. matken11235/logger_slack_backend -> logger_slack_backend
        // ...
        std::string prefix = basename(name);

        // 1. Group1: ``,  Group2: `3`, Group3: `.`, Group4: `4`,  Group5: `.`, Group6: `2`
        // 2. Group1: ``,  Group2: `7`, Group3: `_`, Group4: `61`, Group5: `_`, Group6: `0`
        // 3. Group1: `v`, Group2: `0`, Group3: `.`, Group4: `1`,  Group5: `.`, Group6: `0`
        if (std::regex_match(tag, matches, pattern)) {
            return prefix + "-" + matches[2].str() + "." + matches[4].str() + "." + matches[6].str();
        }
        // 4
        else {
            return tag;
        }
    }
    // poac
    std::string to_cache_poac(const std::string& name, const std::string& ver)
    {
        const auto name2 = slash_to_hyphen(name);
        return name2 + "-" + ver;
    }


    // opencv-3.4.2 -> opencv
    std::string cache_to_current(const std::string& name)
    {
        if (const auto pos = name.find_last_of('-'); pos != std::string::npos)
            { return std::string(name, 0, pos); }
        else
            { return name; }
    }


    // like `poac-0.3.0-beta`
    std::string to_cache(
            const std::string& src,
            const std::string& name,
            const std::string& version)
    {
        if (src == "poac") {
            return to_cache_poac(name, version);
        }
        else if (src == "github") {
            return to_cache_github(name, version);
        }
        else {
            throw exception::error(err::source(src));
        }
    }
    // boost/bind -> boost-bind
    std::string to_current(
            const std::string& src,
            const std::string& name,
            const std::string& version )
    {
        if (src == "poac") {
            return slash_to_hyphen(name);
        }
        else if (src == "github") {
            const auto cache_name = to_cache_github(name, version);
            return cache_to_current(cache_name);
        }
        else {
            throw exception::error(err::source(src));
        }
    }


    std::string get_version(const YAML::Node& node, const std::string& src)
    {
        if (src == "github") {
            if (const auto ver = io::file::yaml::get<std::string>(node, "tag")) {
                return *ver;
            }
        }
        else if (src == "poac") {
            if (const auto ver = io::file::yaml::get<std::string>(node)) {
                return *ver;
            }
            else if (const auto ver2 = io::file::yaml::get<std::string>(node, "version")) {
                return *ver2;
            }
        }
        throw exception::error(err::source(src));
    }

    // github/curl/curl -> src = github, name = curl/curl
    // boost/config -> src = poac, name = boost/config
    std::pair<std::string, std::string>
    get_source(const std::string& name)
    {
        std::regex exp("^github\\/(.*)");
        std::smatch match_group;
        if (std::regex_match(name, match_group, exp)) {
            return std::make_pair("github", match_group[1].str());
        }
        else {
            return std::make_pair("poac", name);
        }
    }
} // end namespace
#endif // !POAC_CORE_NAMING_HPP