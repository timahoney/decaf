/*
 *  Copyright (C) 2012 Tim Mahoney <tim.mahoney@me.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "RBNode.h"

#include "HTMLElement.h"
#include "RBAttr.h"
#include "RBCDATASection.h"
#include "RBComment.h"
#include "RBDocument.h"
#include "RBDocumentFragment.h"
#include "RBDocumentType.h"
#include "RBElement.h"
#include "RBEntity.h"
#include "RBEntityReference.h"
#include "RBExceptionHandler.h"
#include "RBHTMLElementWrapperFactory.h"
#include "RBNotation.h"
#include "RBProcessingInstruction.h"
#if ENABLE(SVG)
#include "RBSVGElement.h"
#include "RBSVGElementWrapperFactory.h"
#endif
#include "RBText.h"
#include <wtf/PassRefPtr.h>

namespace WebCore {

VALUE RBNode::insert_before(VALUE self, VALUE newChild, VALUE refChild)
{
    Node* selfImpl = impl<Node>(self);
    Node* newChildImpl = impl<Node>(newChild);
    Node* refChildImpl = impl<Node>(refChild);
    ExceptionCode ec = 0;
    bool success = selfImpl->insertBefore(newChildImpl, refChildImpl, ec, AttachLazily);
    RB::setDOMException(ec);
    if (success)
        return newChild;
    
    return Qnil;
}

VALUE RBNode::replace_child(VALUE self, VALUE newChild, VALUE refChild)
{
    Node* selfImpl = impl<Node>(self);
    Node* newChildImpl = impl<Node>(newChild);
    Node* refChildImpl = impl<Node>(refChild);
    ExceptionCode ec = 0;
    bool success = selfImpl->replaceChild(newChildImpl, refChildImpl, ec, AttachLazily);
    RB::setDOMException(ec);
    if (success)
        return newChild;
    
    return Qnil;
}

VALUE RBNode::remove_child(VALUE self, VALUE oldChild)
{
    Node* selfImpl = impl<Node>(self);
    Node* oldChildImpl = impl<Node>(oldChild);
    ExceptionCode ec = 0;
    bool success = selfImpl->removeChild(oldChildImpl, ec);
    RB::setDOMException(ec);
    if (success)
        return oldChild;
    
    return Qnil;
}

VALUE RBNode::append_child(VALUE self, VALUE newChild)
{
    Node* selfImpl = impl<Node>(self);
    Node* newChildImpl = impl<Node>(newChild);
    ExceptionCode ec = 0;
    bool success = selfImpl->appendChild(newChildImpl, ec, AttachLazily);
    RB::setDOMException(ec);
    if (success)
        return newChild;
    
    return Qnil;
}

inline VALUE createRBNodeWrapper(Node* node)
{
    if (!node)
        return Qnil;
    
    switch (node->nodeType()) {
    case Node::ELEMENT_NODE:
        if (node->isHTMLElement())
            return createRBHTMLWrapper(toHTMLElement(node));
#if ENABLE(SVG)
        if (node->isSVGElement())
            return createRBSVGWrapper(static_cast<SVGElement*>(node));
#endif
        return toRB(static_cast<Element*>(node));
    case Node::ATTRIBUTE_NODE:
        return toRB(static_cast<Attr*>(node));
    case Node::TEXT_NODE:
        return toRB(static_cast<Text*>(node));
    case Node::CDATA_SECTION_NODE:
        return toRB(static_cast<CDATASection*>(node));
    case Node::ENTITY_NODE:
        return toRB(static_cast<Entity*>(node));
    case Node::PROCESSING_INSTRUCTION_NODE:
        return toRB(static_cast<ProcessingInstruction*>(node));
    case Node::COMMENT_NODE:
        return toRB(static_cast<Comment*>(node));
    case Node::DOCUMENT_NODE:
        return toRB(static_cast<Document*>(node));
    case Node::DOCUMENT_TYPE_NODE:
        return toRB(static_cast<DocumentType*>(node));
    case Node::NOTATION_NODE:
        return toRB(static_cast<Notation*>(node));
    case Node::DOCUMENT_FRAGMENT_NODE:
        return toRB(static_cast<DocumentFragment*>(node));
    case Node::ENTITY_REFERENCE_NODE:
        return toRB(static_cast<EntityReference*>(node));
    default:
        return toRB(RBNode::rubyClass(), node);
    }
}

VALUE toRB(Node* impl)
{
    return createRBNodeWrapper(impl);
}

} // namespace WebCore
