/******************************************************************************
 * Copyright (c) 2022, Kyle Mann (kyle@hobu.co)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following
 * conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the Martin Isenburg or Iowa Department
 *       of Natural Resources nor the names of its contributors may be
 *       used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 ****************************************************************************/
#pragma once

#include <deque>

#include <arbiter/arbiter.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/pdal_types.hpp>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace pdal
{

namespace stac
{
    class Item;
    class Catalog;
    class Collection;
    class ItemCollection;

    typedef std::pair<std::string, std::string> StacError;
    typedef std::deque<StacError> ErrorList;
    typedef std::vector<std::shared_ptr<Item>> ItemList;
    typedef std::vector<std::shared_ptr<Catalog>> SubList;
    typedef std::deque<std::pair<std::time_t, std::time_t>> DatePairs;

    enum GroupType
    {
        catalog,
        collection
    };

    pdal_error stac_error(std::string id, std::string stacType, std::string const& msg);
    pdal_error stac_error(std::string const& msg);


namespace StacUtils
{
    using namespace rapidjson;

    std::string handleRelativePath(std::string srcPath, std::string linkPath);
    std::time_t getStacTime(std::string in);
    const std::string &stacId(const rapidjson::Document& stac);
    const std::string &stacType(const rapidjson::Document& stac);
    const std::string &icSelfPath(const rapidjson::Document& json);

    template <class S = rapidjson::Value::Object>
    const S jsonValue(const Value& stac, std::string key = "")
    {
        if (key.empty())
            return stac.Get<S>();

        const GenericMemberIterator vit = stac.FindMember(key);
        if (vit == stac.MemberEnd())
            throw pdal_error("Missing key '"+key+"' in object: " + stac.GetString());
        try
        {
            return vit->value.Get<S>();
        }
        catch (const std::exception &e)
        {
            throw stac_error(e.what());
        }

    }

    template <class T = NL::json::object_t>
    inline const T &jsonValue(const NL::json& json, std::string key = "")
    {
        if (key.empty())
            return stac.Get<T>();

        const auto vit = stac.FindMember(key)
        if (vit == stac.MemberEnd())
            throw pdal_error("Missing key '"+key+"' in object: " + stac.GetString());
        try
        {
            return stac[key].Get<T>();
        }
        catch (const std::exception &e)
        {
            throw stac_error(e.what());
        }
    }

    template <typename AddType>
    void addMember(Value& obj, std::string name, AddType& v, Document::AllocatorType& al)
    {
        if (typeid(v) == typeid(std::string))
        {
            Value key(name.c_str(), name.size(), al);
            Value val(v.c_str(), v.size(), al);
            obj.AddMember(key, val, al);
        }
        else
        {
            Value key(name.c_str(), name.size(), al);
            Value val(v, v.size(), al);
            obj.AddMember(key, val, al);
        }

    }

    void addMember(Value& obj, std::string name, std::string& v, Document::AllocatorType& al)
    {
        Value key(name.c_str(), name.size(), al);
        Value val(v.c_str(), v.size(), al);
        obj.AddMember(key, val, al);
    }

    // template <class U = NL::json::object_t>
    // inline const U &stacValue(const NL::json& stac, std::string key = "",
    //     const NL::json& rootJson = {})
    // {

    //     try
    //     {
    //         if (key.empty())
    //             return stac.get_ref<const U &>();
    //         return stac.at(key).get_ref<const U &>();
    //     }
    //     catch (NL::detail::exception& e)
    //     {
    //         try {
    //             NL::json stacCheck = stac;
    //             if (!rootJson.empty())
    //                 stacCheck = rootJson;
    //             const std::string type = stacType(stacCheck);
    //             if (type == "FeatureCollection")
    //                 throw stac_error(icSelfPath(stacCheck), type, e.what());
    //             else
    //                 throw stac_error(stacId(stacCheck), type, e.what());
    //         }
    //         catch (std::exception& )
    //         {
    //             throw stac_error(e.what());
    //         }
    //     }
    // }

    // template <class U = rapidjson::Value::Object>
    // const U stacValue(const Value& stac, std::string key = "",
    //     const Document& rootJson = {})
    // {
    //     try
    //     {
    //         if (key.empty())
    //             return stac[key].Get<U>();
    //         return stac[key].Get<U>();
    //     }
    //     catch (const std::exception &e)
    //     {
    //         throw stac_error(e.what());
    //     }

    // }


}// StacUtils
}// stac
}// pdal
