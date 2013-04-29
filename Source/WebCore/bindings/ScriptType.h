/*
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
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

#ifndef ScriptType_h
#define ScriptType_h

#include <wtf/Forward.h>
#include <wtf/Vector.h>

namespace WebCore {

class KURL;
class ScriptSourceCode;

enum ScriptType {
    JSScriptType = 1,
    RBScriptType = 2
};

ScriptType scriptTypeFromMIMEType(const String& mimeType);
String abbreviationForScriptType(ScriptType);
ScriptType scriptTypeFromSourceCode(const ScriptSourceCode&);
ScriptType scriptTypeFromUrl(const KURL&);
    
// FIXME: Either this function should be moved to MIMETypeRegistry,
// or MIMETypeRegistry::isSupportedJavaScriptMIMEType should be moved to here.
// Since we need to map MIME types to ScriptTypes, we might want to move everything
// over to here. That way, MIMETypeRegistery keeps somewhat clean.
bool isSupportedScriptMIMEType(const String& mimeType);

// A vector of all the script types.
typedef Vector<ScriptType> ScriptTypeVector;
ScriptTypeVector scriptTypeVector();

}

#endif // ScriptType_h
