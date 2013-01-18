/*
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ScriptType.h"

#include "MIMETypeRegistry.h"
#include <wtf/text/WTFString.h>
#include <wtf/HashMap.h>

namespace WebCore {

static HashMap<String, ScriptType>* mimeTypeToScriptType;

static void initializeMIMETypes()
{
    mimeTypeToScriptType = new HashMap<String, ScriptType>;
    
    // JavaScript
    HashSet<String> javaScriptMimeTypes = MIMETypeRegistry::getSupportedJavaScriptMIMETypes();
    HashSet<String>::iterator end = javaScriptMimeTypes.end();
    for (HashSet<String>::iterator i = javaScriptMimeTypes.begin(); i != end; ++i)
        mimeTypeToScriptType->add(*i, JSScriptType);
    
    // Ruby
    static const char* rubyMIMETypes[] = {
        "text/ruby",
        "application/ruby"
    };
    for (size_t i = 0; i < WTF_ARRAY_LENGTH(rubyMIMETypes); ++i)
        mimeTypeToScriptType->add(rubyMIMETypes[i], RBScriptType);
}

ScriptType scriptTypeFromMIMEType(const String& mimeType)
{
    if (mimeType.isEmpty() || !isSupportedScriptMIMEType(mimeType))
        return JSScriptType;
    if (!mimeTypeToScriptType)
        initializeMIMETypes();
    
    return mimeTypeToScriptType->get(mimeType);
}

bool isSupportedScriptMIMEType(const String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!mimeTypeToScriptType)
        initializeMIMETypes();
    
    return mimeTypeToScriptType->contains(mimeType);
}

ScriptTypeVector scriptTypeVector()
{
    ScriptTypeVector vector;
    vector.append(JSScriptType);
    vector.append(RBScriptType);
    return vector;
}

String abbreviationForScriptType(ScriptType type)
{
    switch (type) {
    case JSScriptType:
        return "JS";
    case RBScriptType:
        return "RB";
    }
}

} // namespace WebCore
