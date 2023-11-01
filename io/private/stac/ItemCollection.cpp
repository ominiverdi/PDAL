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

#include "ItemCollection.hpp"

namespace pdal
{

namespace stac
{

using namespace StacUtils;

ItemCollection::ItemCollection(const rapidjson::Value& json,
        const std::string& icPath,
        const connector::Connector& connector,
        bool validate):
    m_json(json), m_path(icPath), m_connector(connector),
    m_validate(validate)
{}


ItemCollection::~ItemCollection()
{}

const ItemList& ItemCollection::items()
{
    return m_itemList;
}

bool ItemCollection::init(const Filters& filters, rapidjson::Value& readerArgs,
    SchemaUrls schemaUrls)
{
    const ConstArrayType itemList = jsonValue<ConstArrayType>(m_json, "features");
    for (const Value& itemJson: itemList)
    {
        std::shared_ptr<Item> item(new Item(itemJson, m_path, m_connector, m_validate));
        // Item item(itemJson, m_path, m_connector, m_validate);
        bool valid = item->init(*filters.itemFilters, readerArgs, schemaUrls);
        if (valid)
        {
            m_itemList.push_back(item);
        }
    }
    const Value::ConstMemberIterator lit = m_json.FindMember("links");
    if (lit != m_json.MemberEnd())
    {
        if (!lit->value.IsArray())
            throw stac_error("Links in FeatureCollection must be an array.");
        for (auto& link: lit->value.GetArray())
        {
            const char* target = valueAt(link, "rel").GetString();
            if (strcmp(target, "next") == 0)
            {
                const char* nextLinkPath = valueAt(link, "href").GetString();
                std::string nextAbsPath =
                    handleRelativePath(m_path, nextLinkPath);
                std::vector<char> bin = m_connector.getBinary(nextAbsPath);

                const char* buf = reinterpret_cast<char*> (bin.data());
                rapidjson::Document nextJson;
                nextJson.Parse(buf);

                ItemCollection ic(nextJson, nextAbsPath, m_connector,
                    m_validate);

                if (ic.init(filters, readerArgs, schemaUrls))
                    for (auto& item: ic.items())
                        m_itemList.push_back(std::move(item));
            }
        }
    }
    return true;
}

}//stac

}//pdal