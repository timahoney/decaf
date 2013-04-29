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
#include "RBSVGPathSeg.h"

#include "RBConverters.h"
#include "RBSVGPathSegArcAbs.h"
#include "RBSVGPathSegArcRel.h"
#include "RBSVGPathSegClosePath.h"
#include "RBSVGPathSegCurvetoCubicAbs.h"
#include "RBSVGPathSegCurvetoCubicRel.h"
#include "RBSVGPathSegCurvetoCubicSmoothAbs.h"
#include "RBSVGPathSegCurvetoCubicSmoothRel.h"
#include "RBSVGPathSegCurvetoQuadraticAbs.h"
#include "RBSVGPathSegCurvetoQuadraticRel.h"
#include "RBSVGPathSegCurvetoQuadraticSmoothAbs.h"
#include "RBSVGPathSegCurvetoQuadraticSmoothRel.h"
#include "RBSVGPathSegLinetoAbs.h"
#include "RBSVGPathSegLinetoRel.h"
#include "RBSVGPathSegLinetoHorizontalAbs.h"
#include "RBSVGPathSegLinetoHorizontalRel.h"
#include "RBSVGPathSegLinetoVerticalAbs.h"
#include "RBSVGPathSegLinetoVerticalRel.h"
#include "RBSVGPathSegMovetoAbs.h"
#include "RBSVGPathSegMovetoRel.h"

namespace WebCore {

VALUE toRB(SVGPathSeg* impl)
{
    if (!impl)
        return Qnil;

    switch (impl->pathSegType()) {
    case SVGPathSeg::PATHSEG_CLOSEPATH:
        return toRB(static_cast<SVGPathSegClosePath*>(impl));
    case SVGPathSeg::PATHSEG_MOVETO_ABS:
        return toRB(static_cast<SVGPathSegMovetoAbs*>(impl));
    case SVGPathSeg::PATHSEG_MOVETO_REL:
        return toRB(static_cast<SVGPathSegMovetoRel*>(impl));
    case SVGPathSeg::PATHSEG_LINETO_ABS:
        return toRB(static_cast<SVGPathSegLinetoAbs*>(impl));
    case SVGPathSeg::PATHSEG_LINETO_REL:
        return toRB(static_cast<SVGPathSegLinetoRel*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_CUBIC_ABS:
        return toRB(static_cast<SVGPathSegCurvetoCubicAbs*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_CUBIC_REL:
        return toRB(static_cast<SVGPathSegCurvetoCubicRel*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_QUADRATIC_ABS:
        return toRB(static_cast<SVGPathSegCurvetoQuadraticAbs*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_QUADRATIC_REL:
        return toRB(static_cast<SVGPathSegCurvetoQuadraticRel*>(impl));
    case SVGPathSeg::PATHSEG_ARC_ABS:
        return toRB(static_cast<SVGPathSegArcAbs*>(impl));
    case SVGPathSeg::PATHSEG_ARC_REL:
        return toRB(static_cast<SVGPathSegArcRel*>(impl));
    case SVGPathSeg::PATHSEG_LINETO_HORIZONTAL_ABS:
        return toRB(static_cast<SVGPathSegLinetoHorizontalAbs*>(impl));
    case SVGPathSeg::PATHSEG_LINETO_HORIZONTAL_REL:
        return toRB(static_cast<SVGPathSegLinetoHorizontalRel*>(impl));
    case SVGPathSeg::PATHSEG_LINETO_VERTICAL_ABS:
        return toRB(static_cast<SVGPathSegLinetoVerticalAbs*>(impl));
    case SVGPathSeg::PATHSEG_LINETO_VERTICAL_REL:
        return toRB(static_cast<SVGPathSegLinetoVerticalRel*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_CUBIC_SMOOTH_ABS:
        return toRB(static_cast<SVGPathSegCurvetoCubicSmoothAbs*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_CUBIC_SMOOTH_REL:
        return toRB(static_cast<SVGPathSegCurvetoCubicSmoothRel*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS:
        return toRB(static_cast<SVGPathSegCurvetoQuadraticSmoothAbs*>(impl));
    case SVGPathSeg::PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL:
        return toRB(static_cast<SVGPathSegCurvetoQuadraticSmoothRel*>(impl));
    case SVGPathSeg::PATHSEG_UNKNOWN:
    default:
        return toRB(RBSVGPathSeg::rubyClass(), impl);
    }
}

} // namespace WebCore
