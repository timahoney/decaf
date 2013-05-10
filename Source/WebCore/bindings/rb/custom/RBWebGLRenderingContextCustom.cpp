/*
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEBGL)

#include "RBWebGLRenderingContext.h"

#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBEXTDrawBuffers.h"
#include "RBEXTTextureFilterAnisotropic.h"
#include "RBFloat32Array.h"
#include "RBHTMLCanvasElement.h"
#include "RBHTMLImageElement.h"
#include "RBImageData.h"
#include "RBInt32Array.h"
#include "RBOESElementIndexUint.h"
#include "RBOESStandardDerivatives.h"
#include "RBOESTextureFloat.h"
#include "RBOESTextureHalfFloat.h"
#include "RBOESVertexArrayObject.h"
#include "RBUint32Array.h"
#include "RBUint8Array.h"
#include "RBWebGLBuffer.h"
#include "RBWebGLCompressedTextureATC.h"
#include "RBWebGLCompressedTexturePVRTC.h"
#include "RBWebGLCompressedTextureS3TC.h"
#include "RBWebGLDepthTexture.h"
#include "RBWebGLFramebuffer.h"
#include "RBWebGLLoseContext.h"
#include "RBWebGLProgram.h"
#include "RBWebGLRenderbuffer.h"
#include "RBWebGLShader.h"
#include "RBWebGLTexture.h"
#include "RBWebGLUniformLocation.h"
#include "RBWebGLVertexArrayObjectOES.h"
#include "RBWebKitCSSMatrix.h"
#include "WebGLDebugRendererInfo.h"
#include "WebGLDebugShaders.h"
#include <Ruby/ruby.h>

namespace WebCore {

static VALUE toRB(const WebGLGetInfo& info)
{
    switch (info.getType()) {
    case WebGLGetInfo::kTypeBool:
        return toRB(info.getBool());
    case WebGLGetInfo::kTypeBoolArray: {
        const Vector<bool>& value = info.getBoolArray();
        VALUE list = rb_ary_new2(value.size());
        for (size_t ii = 0; ii < value.size(); ++ii)
            rb_ary_push(list, toRB(value[ii]));
        return list;
    }
    case WebGLGetInfo::kTypeFloat:
        return toRB(info.getFloat());
    case WebGLGetInfo::kTypeInt:
        return toRB(info.getInt());
    case WebGLGetInfo::kTypeNull:
        return Qnil;
    case WebGLGetInfo::kTypeString:
        return rb_str_new2(info.getString().utf8().data());
    case WebGLGetInfo::kTypeUnsignedInt:
        return toRB(info.getUnsignedInt());
    case WebGLGetInfo::kTypeWebGLBuffer:
        return toRB(info.getWebGLBuffer());
    case WebGLGetInfo::kTypeWebGLFloatArray:
        return toRB(info.getWebGLFloatArray());
    case WebGLGetInfo::kTypeWebGLFramebuffer:
        return toRB(info.getWebGLFramebuffer());
    case WebGLGetInfo::kTypeWebGLIntArray:
        return toRB(info.getWebGLIntArray());
    // FIXME: implement WebGLObjectArray
    // case WebGLGetInfo::kTypeWebGLObjectArray:
    case WebGLGetInfo::kTypeWebGLProgram:
        return toRB(info.getWebGLProgram());
    case WebGLGetInfo::kTypeWebGLRenderbuffer:
        return toRB(info.getWebGLRenderbuffer());
    case WebGLGetInfo::kTypeWebGLTexture:
        return toRB(info.getWebGLTexture());
    case WebGLGetInfo::kTypeWebGLUnsignedByteArray:
        return toRB(info.getWebGLUnsignedByteArray());
    case WebGLGetInfo::kTypeWebGLUnsignedIntArray:
        return toRB(info.getWebGLUnsignedIntArray());
    case WebGLGetInfo::kTypeWebGLVertexArrayObjectOES:
        return toRB(info.getWebGLVertexArrayObjectOES());
    default:
        rb_notimplement();
        return Qnil;
    }
}

enum ObjectType {
    kBuffer, kRenderbuffer, kTexture, kVertexAttrib
};

static VALUE getObjectParameter(int argc, VALUE* argv, VALUE self, ObjectType objectType)
{
    VALUE targetRB, pnameRB;
    rb_scan_args(argc, argv, "20", &targetRB, &pnameRB);

    ExceptionCode ec = 0;
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    unsigned target = NUM2INT(targetRB);
    unsigned pname = NUM2INT(pnameRB);
    WebGLGetInfo info;
    switch (objectType) {
    case kBuffer:
        info = context->getBufferParameter(target, pname, ec);
        break;
    case kRenderbuffer:
        info = context->getRenderbufferParameter(target, pname, ec);
        break;
    case kTexture:
        info = context->getTexParameter(target, pname, ec);
        break;
    case kVertexAttrib:
        // target => index
        info = context->getVertexAttrib(target, pname, ec);
        break;
    default:
        rb_notimplement();
        break;
    }
    if (ec) {
        RB::setDOMException(ec);
        return Qnil;
    }
    return toRB(info);
}

static VALUE toRB(WebGLExtension* extension)
{
    if (!extension)
        return Qnil;
    switch (extension->getName()) {
    case WebGLExtension::WebGLLoseContextName:
        return toRB(static_cast<WebGLLoseContext*>(extension));
    case WebGLExtension::EXTDrawBuffersName:
        return toRB(static_cast<EXTDrawBuffers*>(extension));
    case WebGLExtension::EXTTextureFilterAnisotropicName:
        return toRB(static_cast<EXTTextureFilterAnisotropic*>(extension));
    case WebGLExtension::OESStandardDerivativesName:
        return toRB(static_cast<OESStandardDerivatives*>(extension));
    case WebGLExtension::OESTextureFloatName:
        return toRB(static_cast<OESTextureFloat*>(extension));
    case WebGLExtension::OESTextureHalfFloatName:
        return toRB(static_cast<OESTextureHalfFloat*>(extension));
    case WebGLExtension::OESVertexArrayObjectName:
        return toRB(static_cast<OESVertexArrayObject*>(extension));
    case WebGLExtension::OESElementIndexUintName:
        return toRB(static_cast<OESElementIndexUint*>(extension));
    case WebGLExtension::WebGLDebugRendererInfoName:
        return toRB(static_cast<WebGLDebugRendererInfo*>(extension));
    case WebGLExtension::WebGLDebugShadersName:
        return toRB(static_cast<WebGLDebugShaders*>(extension));
    case WebGLExtension::WebGLCompressedTextureATCName:
        return toRB(static_cast<WebGLCompressedTextureATC*>(extension));
    case WebGLExtension::WebGLCompressedTexturePVRTCName:
        return toRB(static_cast<WebGLCompressedTexturePVRTC*>(extension));
    case WebGLExtension::WebGLCompressedTextureS3TCName:
        return toRB(static_cast<WebGLCompressedTextureS3TC*>(extension));
    case WebGLExtension::WebGLDepthTextureName:
        return toRB(static_cast<WebGLDepthTexture*>(extension));
    }
    ASSERT_NOT_REACHED();
    return Qnil;
}

VALUE RBWebGLRenderingContext::get_attached_shaders(VALUE self, VALUE programRB)
{
    ExceptionCode ec = 0;
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    if (!NIL_P(programRB) && !IS_RB_KIND(programRB, WebGLProgram)) {
        rb_raise(rb_eTypeError, "The argument to get_attached_shaders must be a WebGLProgram");
        return Qnil;
    }
    WebGLProgram* program = impl<WebGLProgram>(programRB);
    Vector<RefPtr<WebGLShader> > shaders;
    bool succeed = context->getAttachedShaders(program, shaders, ec);
    if (ec) {
        RB::setDOMException(ec);
        return Qnil;
    }
    if (!succeed)
        return Qnil;
    VALUE list = rb_ary_new();
    for (size_t ii = 0; ii < shaders.size(); ++ii) {
        VALUE shader = toRB(shaders[ii].get());
        rb_ary_push(list, shader);
    }
    return list;
}

VALUE RBWebGLRenderingContext::get_buffer_parameter(int argc, VALUE* argv, VALUE self)
{
    return getObjectParameter(argc, argv, self, kBuffer);
}

VALUE RBWebGLRenderingContext::get_extension(VALUE self, VALUE nameRB)
{
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    const String name = StringValueCStr(nameRB);
    WebGLExtension* extension = context->getExtension(name);
    return toRB(extension);
}

VALUE RBWebGLRenderingContext::get_framebuffer_attachment_parameter(int argc, VALUE* argv, VALUE self)
{
    VALUE targetRB, attachmentRB, pnameRB;
    rb_scan_args(argc, argv, "30", &targetRB, &attachmentRB, &pnameRB);

    ExceptionCode ec = 0;
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    unsigned target = NUM2UINT(targetRB);
    unsigned attachment = NUM2UINT(attachmentRB);
    unsigned pname = NUM2UINT(pnameRB);
    WebGLGetInfo info = context->getFramebufferAttachmentParameter(target, attachment, pname, ec);
    if (ec) {
        RB::setDOMException(ec);
        return Qnil;
    }
    return toRB(info);
}

VALUE RBWebGLRenderingContext::get_parameter(int argc, VALUE* argv, VALUE self)
{
    VALUE pnameRB;
    rb_scan_args(argc, argv, "10", &pnameRB);

    ExceptionCode ec = 0;
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    unsigned pname = NUM2UINT(pnameRB);
    WebGLGetInfo info = context->getParameter(pname, ec);
    if (ec) {
        RB::setDOMException(ec);
        return Qnil;
    }
    return toRB(info);
}

VALUE RBWebGLRenderingContext::get_program_parameter(int argc, VALUE* argv, VALUE self)
{
    VALUE programRB, pnameRB;
    rb_scan_args(argc, argv, "20", &programRB, &pnameRB);

    ExceptionCode ec = 0;
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    if (!NIL_P(programRB) && !IS_RB_KIND(programRB, WebGLProgram)) {
        rb_raise(rb_eTypeError, "First argument to get_program_parameter must be a WebGLProgram");
        return Qnil;
    }
    WebGLProgram* program = impl<WebGLProgram>(programRB);
    unsigned pname = NUM2UINT(pnameRB);
    WebGLGetInfo info = context->getProgramParameter(program, pname, ec);
    if (ec) {
        RB::setDOMException(ec);
        return Qnil;
    }
    return toRB(info);
}

VALUE RBWebGLRenderingContext::get_renderbuffer_parameter(int argc, VALUE* argv, VALUE self)
{
    return getObjectParameter(argc, argv, self, kRenderbuffer);
}

VALUE RBWebGLRenderingContext::get_shader_parameter(int argc, VALUE* argv, VALUE self)
{
    VALUE shaderRB, pnameRB;
    rb_scan_args(argc, argv, "20", &shaderRB, &pnameRB);

    ExceptionCode ec = 0;
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    if (!NIL_P(shaderRB) && !IS_RB_KIND(shaderRB, WebGLShader)) {
        rb_raise(rb_eTypeError, "First argument to get_shader_parameter must be a WebGLShader");
        return Qnil;
    }
    WebGLShader* shader = impl<WebGLShader>(shaderRB);
    unsigned pname = NUM2UINT(pnameRB);
    WebGLGetInfo info = context->getShaderParameter(shader, pname, ec);
    if (ec) {
        RB::setDOMException(ec);
        return Qnil;
    }
    return toRB(info);
}

VALUE RBWebGLRenderingContext::get_supported_extensions(VALUE self)
{
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    if (context->isContextLost())
        return Qnil;
    Vector<String> value = context->getSupportedExtensions();
    VALUE list = rb_ary_new2(value.size());
    for (size_t ii = 0; ii < value.size(); ++ii)
        rb_ary_push(list, rb_str_new2(value[ii].utf8().data()));
    return list;
}

VALUE RBWebGLRenderingContext::get_tex_parameter(int argc, VALUE* argv, VALUE self)
{
    return getObjectParameter(argc, argv, self, kTexture);
}

VALUE RBWebGLRenderingContext::get_uniform(int argc, VALUE* argv, VALUE self)
{
    VALUE programRB, locationRB;
    rb_scan_args(argc, argv, "20", &programRB, &locationRB);

    ExceptionCode ec = 0;
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    if (!NIL_P(programRB) && !IS_RB_KIND(programRB, WebGLProgram)) {
        rb_raise(rb_eTypeError, "First argument to get_uniform must be a WebGLProgram");
        return Qnil;
    }
    WebGLProgram* program = impl<WebGLProgram>(programRB);
    if (!NIL_P(locationRB) && !IS_RB_KIND(locationRB, WebGLUniformLocation)) {
        rb_raise(rb_eTypeError, "Second argument to get_uniform must be a WebGLUniformLocation");
        return Qnil;
    }
    WebGLUniformLocation* location = impl<WebGLUniformLocation>(locationRB);
    WebGLGetInfo info = context->getUniform(program, location, ec);
    if (ec) {
        RB::setDOMException(ec);
        return Qnil;
    }
    return toRB(info);
}

VALUE RBWebGLRenderingContext::get_vertex_attrib(int argc, VALUE* argv, VALUE self)
{
    return getObjectParameter(argc, argv, self, kVertexAttrib);
}

enum DataFunctionToCall {
    f_uniform1v, f_uniform2v, f_uniform3v, f_uniform4v,
    f_vertexAttrib1v, f_vertexAttrib2v, f_vertexAttrib3v, f_vertexAttrib4v
};

enum DataFunctionMatrixToCall {
    f_uniformMatrix2fv, f_uniformMatrix3fv, f_uniformMatrix4fv
};

static bool functionForUniform(DataFunctionToCall f)
{
    switch (f) {
    case f_uniform1v:
    case f_uniform2v:
    case f_uniform3v:
    case f_uniform4v:
        return true;
        break;
    default: break;
    }
    return false;
}

static VALUE dataFunctionf(DataFunctionToCall f, VALUE locationRB, VALUE arrayRB, VALUE self)
{
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    WebGLUniformLocation* location = 0;
    long index = -1;
    
    if (functionForUniform(f)) {
        if (!NIL_P(locationRB) && !IS_RB_KIND(locationRB, WebGLUniformLocation)) {
            rb_raise(rb_eTypeError, "First argument must be a WebGLUniformLocation");
            return Qnil;
        }
        location = impl<WebGLUniformLocation>(locationRB);
    } else {
        index = NUM2LONG(locationRB);
    }

        
    ExceptionCode ec = 0;
    if (IS_RB_KIND(arrayRB, Float32Array)) {
        RefPtr<Float32Array> webGLArray = impl<Float32Array>(arrayRB);
        switch (f) {
        case f_uniform1v:
            context->uniform1fv(location, webGLArray.get(), ec);
            break;
        case f_uniform2v:
            context->uniform2fv(location, webGLArray.get(), ec);
            break;
        case f_uniform3v:
            context->uniform3fv(location, webGLArray.get(), ec);
            break;
        case f_uniform4v:
            context->uniform4fv(location, webGLArray.get(), ec);
            break;
        case f_vertexAttrib1v:
            context->vertexAttrib1fv(index, webGLArray.get());
            break;
        case f_vertexAttrib2v:
            context->vertexAttrib2fv(index, webGLArray.get());
            break;
        case f_vertexAttrib3v:
            context->vertexAttrib3fv(index, webGLArray.get());
            break;
        case f_vertexAttrib4v:
            context->vertexAttrib4fv(index, webGLArray.get());
            break;
        }
        
        RB::setDOMException(ec);
        return Qnil;
    }

    if (!IS_RB_ARRAY(arrayRB)) {
        rb_raise(rb_eTypeError, "Second argument must be a Float32Array or an array of numbers");
        return Qnil;
    }
    Vector<float> array = rbToNativeArray<float, 64>(arrayRB);

    switch (f) {
    case f_uniform1v:
        context->uniform1fv(location, array.data(), array.size(), ec);
        break;
    case f_uniform2v:
        context->uniform2fv(location, array.data(), array.size(), ec);
        break;
    case f_uniform3v:
        context->uniform3fv(location, array.data(), array.size(), ec);
        break;
    case f_uniform4v:
        context->uniform4fv(location, array.data(), array.size(), ec);
        break;
    case f_vertexAttrib1v:
        context->vertexAttrib1fv(index, array.data(), array.size());
        break;
    case f_vertexAttrib2v:
        context->vertexAttrib2fv(index, array.data(), array.size());
        break;
    case f_vertexAttrib3v:
        context->vertexAttrib3fv(index, array.data(), array.size());
        break;
    case f_vertexAttrib4v:
        context->vertexAttrib4fv(index, array.data(), array.size());
        break;
    }
    
    RB::setDOMException(ec);
    return Qnil;
}

static VALUE dataFunctioni(DataFunctionToCall f, VALUE locationRB, VALUE arrayRB, VALUE self)
{
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);
    if (!NIL_P(locationRB) && !IS_RB_KIND(locationRB, WebGLUniformLocation)) {
        rb_raise(rb_eTypeError, "First argument must be a WebGLUniformLocation");
        return Qnil;
    }
    WebGLUniformLocation* location = impl<WebGLUniformLocation>(locationRB);
    
    ExceptionCode ec = 0;
    if (IS_RB_KIND(arrayRB, Int32Array)) {
        RefPtr<Int32Array> webGLArray = impl<Int32Array>(arrayRB);
        switch (f) {
        case f_uniform1v:
            context->uniform1iv(location, webGLArray.get(), ec);
            break;
        case f_uniform2v:
            context->uniform2iv(location, webGLArray.get(), ec);
            break;
        case f_uniform3v:
            context->uniform3iv(location, webGLArray.get(), ec);
            break;
        case f_uniform4v:
            context->uniform4iv(location, webGLArray.get(), ec);
            break;
        default:
            break;
        }
        
        RB::setDOMException(ec);
        return Qnil;
    }

    if (!IS_RB_ARRAY(arrayRB)) {
        rb_raise(rb_eTypeError, "Second argument must be a Int32Array or an array of numbers");
        return Qnil;
    }
    Vector<int> array = rbToNativeArray<int, 64>(arrayRB);

    switch (f) {
    case f_uniform1v:
        context->uniform1iv(location, array.data(), array.size(), ec);
        break;
    case f_uniform2v:
        context->uniform2iv(location, array.data(), array.size(), ec);
        break;
    case f_uniform3v:
        context->uniform3iv(location, array.data(), array.size(), ec);
        break;
    case f_uniform4v:
        context->uniform4iv(location, array.data(), array.size(), ec);
        break;
    default:
        break;
    }
    
    RB::setDOMException(ec);
    return Qnil;
}

static VALUE dataFunctionMatrix(DataFunctionMatrixToCall f, VALUE locationRB, VALUE transposeRB, VALUE arrayRB, VALUE self)
{
    WebGLRenderingContext* context = impl<WebGLRenderingContext>(self);

    if (!NIL_P(locationRB) && !IS_RB_KIND(locationRB, WebGLUniformLocation)) {
        rb_raise(rb_eTypeError, "First argument must be a WebGLUniformLocation");
        return Qnil;
    }
    WebGLUniformLocation* location = impl<WebGLUniformLocation>(locationRB);

    ExceptionCode ec = 0;
    bool transpose = RTEST(transposeRB);
    if (IS_RB_KIND(arrayRB, Float32Array)) {
        RefPtr<Float32Array> webGLArray = impl<Float32Array>(arrayRB);
        switch (f) {
        case f_uniformMatrix2fv:
            context->uniformMatrix2fv(location, transpose, webGLArray.get(), ec);
            break;
        case f_uniformMatrix3fv:
            context->uniformMatrix3fv(location, transpose, webGLArray.get(), ec);
            break;
        case f_uniformMatrix4fv:
            context->uniformMatrix4fv(location, transpose, webGLArray.get(), ec);
            break;
        }
        
        RB::setDOMException(ec);
        return Qnil;
    }

    if (!IS_RB_ARRAY(arrayRB)) {
        rb_raise(rb_eTypeError, "Third argument must be a Float32Array or an array of numbers");
        return Qnil;
    }
    Vector<float> array = rbToNativeArray<float, 64>(arrayRB);

    switch (f) {
    case f_uniformMatrix2fv:
        context->uniformMatrix2fv(location, transpose, array.data(), array.size(), ec);
        break;
    case f_uniformMatrix3fv:
        context->uniformMatrix3fv(location, transpose, array.data(), array.size(), ec);
        break;
    case f_uniformMatrix4fv:
        context->uniformMatrix4fv(location, transpose, array.data(), array.size(), ec);
        break;
    }

    RB::setDOMException(ec);
    return Qnil;
}

VALUE RBWebGLRenderingContext::uniform1fv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctionf(f_uniform1v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform1iv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctioni(f_uniform1v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform2fv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctionf(f_uniform2v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform2iv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctioni(f_uniform2v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform3fv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctionf(f_uniform3v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform3iv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctioni(f_uniform3v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform4fv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctionf(f_uniform4v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform4iv(VALUE self, VALUE location, VALUE v)
{
    return dataFunctioni(f_uniform4v, location, v, self);
}

VALUE RBWebGLRenderingContext::uniform_matrix2fv(VALUE self, VALUE location, VALUE transpose, VALUE array)
{
    return dataFunctionMatrix(f_uniformMatrix2fv, location, transpose, array, self);
}

VALUE RBWebGLRenderingContext::uniform_matrix3fv(VALUE self, VALUE location, VALUE transpose, VALUE array)
{
    return dataFunctionMatrix(f_uniformMatrix3fv, location, transpose, array, self);
}

VALUE RBWebGLRenderingContext::uniform_matrix4fv(VALUE self, VALUE location, VALUE transpose, VALUE array)
{
    return dataFunctionMatrix(f_uniformMatrix4fv, location, transpose, array, self);
}

VALUE RBWebGLRenderingContext::vertex_attrib1fv(VALUE self, VALUE indx, VALUE values)
{
    return dataFunctionf(f_vertexAttrib1v, indx, values, self);
}

VALUE RBWebGLRenderingContext::vertex_attrib2fv(VALUE self, VALUE indx, VALUE values)
{
    return dataFunctionf(f_vertexAttrib2v, indx, values, self);
}

VALUE RBWebGLRenderingContext::vertex_attrib3fv(VALUE self, VALUE indx, VALUE values)
{
    return dataFunctionf(f_vertexAttrib3v, indx, values, self);
}

VALUE RBWebGLRenderingContext::vertex_attrib4fv(VALUE self, VALUE indx, VALUE values)
{
    return dataFunctionf(f_vertexAttrib4v, indx, values, self);
}

#endif // ENABLE(WEBGL)

} // namespace WebCore
